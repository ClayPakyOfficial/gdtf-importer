/*
MIT License

Copyright (c) 2022 Clay Paky S.P.A.

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

/*#include "DMXProtocolTypes.h"
#include "MVR/DMXMVRFixtureActorInterface.h"*/


ACPGDTFFixtureActor::ACPGDTFFixtureActor() {

	PrimaryActorTick.bCanEverTick = true;

	#if WITH_EDITORONLY_DATA
		bRunConstructionScriptOnDrag = 0;
	#endif

	this->SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));

	this->DMX = CreateDefaultSubobject<UDMXComponent>(TEXT("DMX Input"));
	
	/// TODO \todo ChangeFixtureMode disabled keep the actor stable. Finish the Blueprint update and enable it again.
	//this->DMX->OnFixturePatchChanged.AddUObject(this, &ACPGDTFFixtureActor::ChangeFixtureMode); // Catch mode changes from DMXLibrary
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
	//TODO LS: Why is it calling DMXComponents.Remove(ColorSource); ?

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

				if (ColorWheelPtr && ColorWheelPtr->IsMacroColor()) { //LS: CHECK: is this an optimization?
					FinalColor = ColorWheelPtr->ApplyFilter(FLinearColor(1, 1, 1)); // If this is a macro color we overwrite everything and exit the loop
					//TODO LS: Are we sure this is correct? Maybe it will give problems with multiple wheels/flags
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
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 25 may 2022
 *
 * @param FixtureGDTFDescription GDTF Description of the Fixture
 * @param FixturePackagePath Path of the package (folder) containing the fixture assets. For example: "/Game/ClayPaky_MiniB"
 */
void ACPGDTFFixtureActor::PreConstruct(UCPGDTFDescription* FixtureGDTFDescription, FString FixturePackagePath) {

	this->GDTFDescription = FixtureGDTFDescription;
	this->FixturePathInContentBrowser = FixturePackagePath;

	// By default we setup the first DMX mode
	this->CurrentModeIndex = 0;
	
	// Construction of the Geometries tree
	this->GeometryTree.CreateGeometryTree(this, FixturePackagePath);

	// Add of DMX components
	FCPFActorComponentsLoader::LoadDMXComponents(this, FixtureGDTFDescription->GetDMXModes()->DMXModes[this->CurrentModeIndex]);
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
		MinQuality = FMath::Clamp(MinQuality, 0.2f, 4.0f);
		MaxQuality = FMath::Clamp(MaxQuality, 0.2f, 4.0f);
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

void ACPGDTFFixtureActor::SetSpotlightIntensityScale(float NewSpotlightIntensityScale) {

	for (const TPair<FName, UCPGDTFBeamSceneComponent*> pair : this->GeometryTree.BeamComponents) {
		pair.Value->SetSpotlightIntensityScale(NewSpotlightIntensityScale);
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
		for (UCPGDTFFixtureComponentBase* DMXComponent : TInlineComponentArray<UCPGDTFFixtureComponentBase*>(this)) {
			DMXComponent->PushNormalizedRawValues(FixturePatch, RawValuesMap);
		}
	}
}

#if WITH_EDITOR
void ACPGDTFFixtureActor::ChangeFixtureMode(const UDMXEntityFixturePatch* FixturePatch) {

	/// TODO \todo Fix this method. Works but don't save the changes on disk.

	if (FixturePatch->GetActiveModeIndex() == this->CurrentModeIndex) return; // If the mode didn't changed we return

	if (FixturePatch->GetActiveModeIndex() < 0 || FixturePatch->GetActiveModeIndex() > this->GDTFDescription->GetDMXModes()->DMXModes.Num() - 1) {
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Mode index out of bounds in '%s' on ChangeFixtureMode"), *this->GetName());
		return;
	}

	this->CurrentModeIndex = FixturePatch->GetActiveModeIndex();

	// Step 1: Purge DMX Components
	FCPFActorComponentsLoader::PurgeDMXComponents(this);

	// Step 2: Purge Geometries tree
	this->GeometryTree.DestroyGeometryTree();

	// Step 3: Reconstruct the new Geometries tree
	this->GeometryTree.CreateGeometryTree(this, this->FixturePathInContentBrowser, this->CurrentModeIndex);

	// Step 4: Add the new DMX components
	FCPFActorComponentsLoader::LoadDMXComponents(this, this->GDTFDescription->GetDMXModes()->DMXModes[this->CurrentModeIndex]);

	this->GetPackage()->SetDirtyFlag(true);
}
#endif