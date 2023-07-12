/*
MIT License

Copyright (c) 2022 Clay Paky S.R.L.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "CPGDTFFixtureActor.h"
#include "Factories/CPGDTFFactory.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Utils/CPGDTFColorWizard.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Utils/CPFActorComponentsLoader.h"
#include "Components/DMXComponents/CPGDTFFixtureComponentBase.h"
#include "Components/DMXComponents/CPGDTFColorSourceFixtureComponent.h"
#include "Components/DMXComponents/CPGDTFAdditiveColorFixtureComponent.h"
#include "Components/DMXComponents/CPGDTFSubstractiveColorFixtureComponent.h"
#include "Components/DMXComponents/CPGDTFColorCorrectionFixtureComponent.h"
#include "Components/DMXComponents/CPGDTFSimpleAttributeFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFColorWheelFixtureComponent.h"

#include "Library/DMXEntityFixturePatch.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/KismetReinstanceUtilities.h"
#include "AssetRegistryModule.h"

#include "Engine/Blueprint.h"
#include "Engine/SimpleConstructionScript.h"

ACPGDTFFixtureActor::ACPGDTFFixtureActor() {

	PrimaryActorTick.bCanEverTick = true;

	#if WITH_EDITORONLY_DATA
		bRunConstructionScriptOnDrag = 0;
	#endif

	this->SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));

	this->DMX = CreateDefaultSubobject<UDMXComponent>(TEXT("DMX Input"));
	
	this->DMX->OnFixturePatchChanged.AddUObject(this, &ACPGDTFFixtureActor::ChangeFixtureMode); // Catch mode changes from DMXLibrary
}

// Called during Actor spawn to a specific world
void ACPGDTFFixtureActor::OnConstruction(const FTransform& Transform) {

	// OnConstruction is called twice when the actor is dragged into a level. Once for the preview actor and once for the real actor.
	// We don't need to full setup the Actor for the preview
	if (!IsTemplate(RF_Transient)) {

		this->GeometryTree.ReParseGeometryTree(this);
		// Initialize Beamcomponents
		for (UCPGDTFBeamSceneComponent* BeamComponent : TInlineComponentArray<UCPGDTFBeamSceneComponent*>(this)) {
			BeamComponent->OnConstruction();
		}
		// Initialize DMX fixture components
		for (UCPGDTFFixtureComponentBase* DMXComponent : TInlineComponentArray<UCPGDTFFixtureComponentBase*>(this)) {
			auto aa = DMXComponent->GetParentFixtureActor();
			DMXComponent->OnConstruction();
		}
	}
}

void ACPGDTFFixtureActor::BeginDestroy() {

	// If destroy occurs during gameplay 'EndPlay()' is called by Unreal before this method

	this->CheckOcclusionTimer.Invalidate();

	Super::BeginDestroy();
}

void ACPGDTFFixtureActor::BeginPlay() {

	Super::BeginPlay();

	this->GeometryTree.ReParseGeometryTree(this);
	this->UpdateProperties();

	if (this->UseDynamicOcclusion)
		GetWorld()->GetTimerManager().SetTimer(CheckOcclusionTimer, this, &ACPGDTFFixtureActor::CheckOcclusion, FMath::FRandRange(0.1f, 0.3f), true, 0.0f);

	this->DMX->OnDMXReceivedRaw.AddDynamic(this, &ACPGDTFFixtureActor::PushNormalizedRawValues);
}

void ACPGDTFFixtureActor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	
	Super::EndPlay(EndPlayReason);

	this->DMX->OnDMXReceivedRaw.RemoveAll(this);					// We stop listening DMX packets
	GetWorld()->GetTimerManager().ClearTimer(CheckOcclusionTimer); // We remove the CheckOcclusion timer
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);  // And just in case
}

void ACPGDTFFixtureActor::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);
	this->ToggleLightVisibility();

	// Interpolation of the DMXComponents
	// WARNING: The order is important to create the behaviour of real fixtures (cf. Order of different effects in the head of a moving head)
	
	// Array with all the DMXComponents of the Actor
	TInlineComponentArray<UCPGDTFFixtureComponentBase*> DMXComponents = TInlineComponentArray<UCPGDTFFixtureComponentBase*>(this);

	// Step 1 Color Sources
	for (TPair<FName, UCPGDTFBeamSceneComponent*> BeamPair : this->GeometryTree.BeamComponents) {

		FCPColorWizard AdditiveWizard;
		bool bIsAdditivePresent = false;
		for (UCPGDTFAdditiveColorFixtureComponent* ColorSource : TInlineComponentArray<UCPGDTFAdditiveColorFixtureComponent*>(this)) {
			if (ColorSource->AttachedBeams.Contains(BeamPair.Value)) {
				if (ColorSource->bUseInterpolation)
					ColorSource->InterpolateComponent(DeltaTime);
				AdditiveWizard.BlendColor(ColorSource->GetCurrentColor(), 1);
				DMXComponents.Remove(ColorSource);
				bIsAdditivePresent = true;
			}
		}

		bool bIsMacroPresent = false;
		FLinearColor FinalColor;
		if (bIsAdditivePresent)
			FinalColor = AdditiveWizard.GetColor();
		else
			FinalColor = FLinearColor(1, 1, 1, 1); // If we don't have any additive color source the base light is white
		for (UCPGDTFSubstractiveColorFixtureComponent* ColorSource : TInlineComponentArray<UCPGDTFSubstractiveColorFixtureComponent*>(this)) {
			if (ColorSource->AttachedBeams.Contains(BeamPair.Value)) {
				if (ColorSource->bUseInterpolation) ColorSource->InterpolateComponent(DeltaTime);
				UCPGDTFColorWheelFixtureComponent* ColorWheelPtr = Cast<UCPGDTFColorWheelFixtureComponent>(ColorSource);

				if (ColorWheelPtr && ColorWheelPtr->IsMacroColor()) {
					FinalColor = ColorWheelPtr->ApplyFilter(FLinearColor(1, 1, 1)); // If this is a macro color we overwrite everything and exit the loop
					bIsMacroPresent = true;
					break;
				}
				if (!bIsMacroPresent) FinalColor = ColorSource->ApplyFilter(FinalColor);
				DMXComponents.Remove(ColorSource);
			}
		}

		BeamPair.Value->SetLightColor(FinalColor);
	}
	
	// Step 3 Color corrections
	for (UCPGDTFColorCorrectionFixtureComponent* ColorCorrection : TInlineComponentArray<UCPGDTFColorCorrectionFixtureComponent*>(this)) {
		if (ColorCorrection->bUseInterpolation) ColorCorrection->InterpolateComponent(DeltaTime);
		DMXComponents.Remove(ColorCorrection);
	}

	// Step 4 Other effects
	// WARNING: Here the order between the DMXComponents is unknow
	for (UCPGDTFFixtureComponentBase* Component : DMXComponents) {
		if (Component->bUseInterpolation) Component->InterpolateComponent(DeltaTime);
	}
}

#if WITH_EDITOR
	void ACPGDTFFixtureActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {

		Super::PostEditChangeProperty(PropertyChangedEvent);
		this->UpdateProperties();
	}
#endif

/**
 * Setup the actor for a specific GDTFDescription.
 * Used during the generation of the Asset by the GDTFFactory.
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 25 may 2022
 *
 * @param FixtureGDTFDescription GDTF Description of the Fixture
 * @param FixturePackagePath Path of the package (folder) containing the fixture assets. For example: "/Game/ClayPaky_MiniB"
 */
void ACPGDTFFixtureActor::PreConstruct(UCPGDTFDescription* FixtureGDTFDescription) {
	this->GDTFDescription = FixtureGDTFDescription;
	
	// Add of DMX components
	FCPFActorComponentsLoader::LoadDMXComponents(this, FixtureGDTFDescription->GetDMXModes()->DMXModes[this->CurrentModeIndex]);

	// Construction of the Geometries tree
	this->GeometryTree.CreateGeometryTree(this, FixturePathInContentBrowser);
}

void ACPGDTFFixtureActor::CreateRenderingPipelines(UCPGDTFDescription* FixtureGDTFDescription) {
	auto modes = FixtureGDTFDescription->GetDMXModes()->DMXModes;
	FCPFActorComponentsLoader::PurgeAllComponents(this);
	for (int modeId = 0; modeId < modes.Num(); modeId++) {
		this->CurrentModeIndex = modeId;
		FCPFActorComponentsLoader::LoadDMXComponents(this, FixtureGDTFDescription->GetDMXModes()->DMXModes[this->CurrentModeIndex]);
		CPGDTFRenderPipelineBuilder pipelineBuilder = CPGDTFRenderPipelineBuilder(FixtureGDTFDescription, this->CurrentModeIndex, GetInstanceComponents(), FixturePathInContentBrowser);
		pipelineBuilder.buildLightRenderPipeline();
		FCPFActorComponentsLoader::PurgeAllComponents(this);
	}
}

void ACPGDTFFixtureActor::UpdateProperties() {

	// Note: MinQuality and MaxQuality are used in conjonction with the zoom angle when zoom component is used
	// Note:fallback when fixture doesnt use zoom component
	float QualityFallback = 1.0f;
	switch (QualityLevel) {
	case(ECPGDTFFixtureQualityLevel::LowQuality): MinQuality = 4.0f; MaxQuality = 4.0f; QualityFallback = 4.0f; break;
	case(ECPGDTFFixtureQualityLevel::MediumQuality): MinQuality = 2.0f; MaxQuality = 2.0f; QualityFallback = 2.0f; break;
	case(ECPGDTFFixtureQualityLevel::HighQuality): MinQuality = 1.0f; MaxQuality = 1.0f; QualityFallback = 1.0f; break;
	case(ECPGDTFFixtureQualityLevel::UltraQuality): MinQuality = 0.33f; MaxQuality = 0.33f; QualityFallback = 0.33f; break;
	}

	if (QualityLevel == ECPGDTFFixtureQualityLevel::Custom) {
		MinQuality = FMath::Clamp(MinQuality, 0.1f, 4.0f);
		MaxQuality = FMath::Clamp(MaxQuality, 0.1f, 4.0f);
		QualityFallback = MaxQuality;
	}

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetBeamQuality(QualityFallback);
	}
}

void ACPGDTFFixtureActor::ToggleLightVisibility() {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->ToggleLightVisibility();
	}
}

bool ACPGDTFFixtureActor::IsMoving() {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		if (pair.Value->IsMoving()) return true;
	}
	return false;
}

void ACPGDTFFixtureActor::CheckOcclusion() {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->CheckOcclusion();
	}
}

void ACPGDTFFixtureActor::SetLightCastShadow(bool bLightShouldCastShadow) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetLightCastShadow(bLightShouldCastShadow);
	}
}

void ACPGDTFFixtureActor::SetPointlightIntensityScale(float NewPointlightIntensityScale) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetPointlightIntensityScale(NewPointlightIntensityScale);
	}
}

void ACPGDTFFixtureActor::SetSpotlightLightIntensityScale(float NewSpotlightIntensityScale) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetSpotlightLightIntensityScale(NewSpotlightIntensityScale);
	}
}

void ACPGDTFFixtureActor::SetSpotlightBeamIntensityScale(float NewSpotlightIntensityScale) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetSpotlightBeamIntensityScale(NewSpotlightIntensityScale);
	}
}

void ACPGDTFFixtureActor::SetSpotlightLensIntensityScale(float NewSpotlightIntensityScale) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetSpotlightLensIntensityScale(NewSpotlightIntensityScale);
	}
}

void ACPGDTFFixtureActor::SetLightColorTemp(float NewLightColorTemp) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetLightColorTemp(NewLightColorTemp);
	}
}

void ACPGDTFFixtureActor::SetLightDistanceMax(float NewLightDistanceMax) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetLightDistanceMax(NewLightDistanceMax);
	}
}

void ACPGDTFFixtureActor::PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) {
	
	if (this->HasActorBegunPlay()) {
		auto asd = TInlineComponentArray<UCPGDTFFixtureComponentBase*>(this);
		for (UCPGDTFFixtureComponentBase* DMXComponent : TInlineComponentArray<UCPGDTFFixtureComponentBase*>(this)) {
			DMXComponent->PushNormalizedRawValues(FixturePatch, RawValuesMap);
		}
	}
}

#if WITH_EDITOR
void ACPGDTFFixtureActor::ChangeFixtureMode(const UDMXEntityFixturePatch* FixturePatch) {
	if (FixturePatch->GetActiveModeIndex() == this->CurrentModeIndex) return; // If the mode didn't changed we return

	if (FixturePatch->GetActiveModeIndex() < 0 || FixturePatch->GetActiveModeIndex() > this->GDTFDescription->GetDMXModes()->DMXModes.Num() - 1) {
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Mode index out of bounds in '%s' on ChangeFixtureMode"), *this->GetName());
		return;
	}

	this->CurrentModeIndex = FixturePatch->GetActiveModeIndex();

	//Unloading all components and geometries and reloading them with the correct dmx mode won't work, since we cannot update the ConstructionScripts once the blueprint has been created.
	//The current method works by firstly creating a blueprint class for each mode when we firstly import a light, then replacing the current object's blueprint with the selected mode each time we change it.
	//This works with a lot of magic, please don't break it.
	//For any questions, ask Luca Sorace.

	//Load the class of the correct mode
	FString newClassName = getClassNameFromMode(UCPGDTFFactory::generateModeName(this->CurrentModeIndex));
	UBlueprint* targetBp = Cast<UBlueprint>(FCPGDTFImporterUtils::IsAssetExisting(newClassName, ActorsPathInContentBrowser));

	//Obtain the current blueprint
	UClass* cls = GetClass();
	UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>(cls);
	UObject* generatedBy = bpClass->ClassGeneratedBy;
	UBlueprint* currentBp = Cast<UBlueprint>(generatedBy);

	//Obtain a list of ALL object using the previous BluePrint, and remove the current object from the said list.
	// This will be the list of exclusion used in the next step.
	TArray<UObject*> classes;
	GetObjectsOfClass(currentBp->GeneratedClass, classes, true);
	TSet<UObject*> excludedInstances = TSet<UObject*>(classes);
	excludedInstances.Remove(this);

	//Replace the blueprint of the current object with the one of the selected mode
	FReplaceInstancesOfClassParameters params(currentBp->GeneratedClass, targetBp->GeneratedClass);
	params.InstancesThatShouldUseOldClass = &excludedInstances;
	params.ObjectsThatShouldUseOldStuff = &excludedInstances;
	FBlueprintCompileReinstancer::ReplaceInstancesOfClassEx(params);
	currentBp->GeneratedClass->ClassFlags &= ~EClassFlags::CLASS_NewerVersionExists; //Needed to prevent segfaults
}
#endif

FString ACPGDTFFixtureActor::getClassNameFromMode(FString modeName) {
	UClass* cls = GetClass();
	UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>(cls);
	UObject* generatedBy = bpClass->ClassGeneratedBy;
	FString currentName = generatedBy->GetName();
	if (currentName.EndsWith(this->CurrentModeName)) //Legacy/old actors support
		currentName = currentName.Left(currentName.Len() - this->CurrentModeName.Len());
	else
		currentName += "_";
	return currentName + modeName;
}