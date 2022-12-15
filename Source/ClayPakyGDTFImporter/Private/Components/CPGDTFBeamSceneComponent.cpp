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


#include "Components/CPGDTFBeamSceneComponent.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Utils/CPGDTFImporterUtils.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"


UCPGDTFBeamSceneComponent::UCPGDTFBeamSceneComponent() {

	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	
	this->LightDistanceMax = 1000.0f;
	this->LightColorTemp = 6500;
	this->SpotlightIntensityScale = 1.0f;
	this->PointlightIntensityScale = 1.0f;
	this->LightCastShadow = false;
}

void UCPGDTFBeamSceneComponent::BeginPlay() {
	
	Super::BeginPlay();

	// Create dynamic materials if needed
	if (!this->DynamicMaterialLens || !this->DynamicMaterialBeam || !this->DynamicMaterialSpotLight || !this->DynamicMaterialPointLight) {

		this->DynamicMaterialLens = UMaterialInstanceDynamic::Create(this->LensMaterialInstance, nullptr);
		this->DynamicMaterialBeam = UMaterialInstanceDynamic::Create(this->BeamMaterialInstance, nullptr);
		this->DynamicMaterialSpotLight = UMaterialInstanceDynamic::Create(this->SpotLightMaterialInstance, nullptr);
		this->DynamicMaterialPointLight = UMaterialInstanceDynamic::Create(this->PointLightMaterialInstance, nullptr);
	}

	// Get lens width (support scaling)
	if (this->LensStaticMeshComponent) this->LensRadius = this->LensStaticMeshComponent->Bounds.SphereRadius * 0.9f;

	// Feed fixture data into materials and lights
	this->UpdateProperties();

	// Assign dynamic materials to static meshes
	if (this->LensStaticMeshComponent) this->LensStaticMeshComponent->SetMaterial(0, this->DynamicMaterialLens);

	// Make sure beam doesnt have any scale applied or it wont render correctly
	if (this->BeamStaticMeshComponent) {
		this->BeamStaticMeshComponent->SetMaterial(0, this->DynamicMaterialBeam);
		this->BeamStaticMeshComponent->SetWorldScale3D(FVector(1, 1, 1));
	}

	// Assign dynamic materials to lights
	this->SpotLight->SetMaterial(0, this->DynamicMaterialSpotLight);
	this->PointLight->SetMaterial(0, this->DynamicMaterialPointLight);
}

// Called during Actor spawn to a specific world
void UCPGDTFBeamSceneComponent::OnConstruction() {
	FString BeamComponentName = this->GetName();
	BeamComponentName.RemoveFromStart("BEAM_", ESearchCase::CaseSensitive);

	TArray<USceneComponent*> Childrens;
	this->GetChildrenComponents(false, Childrens);
	for (USceneComponent* Child : Childrens) {

		if (Child->GetName().Equals(FString("Occlusion_").Append(BeamComponentName))) this->OcclusionDirection = Cast<UArrowComponent>(Child);

		else if (Child->GetName().Equals(FString("CPSM_BEAM_").Append(BeamComponentName))) this->BeamStaticMeshComponent = Cast<UStaticMeshComponent>(Child);
		else if (Child->GetName().Equals(FString("CPSM_Lens_").Append(BeamComponentName))) this->LensStaticMeshComponent = Cast<UStaticMeshComponent>(Child);

		else if (Child->GetName().Equals(FString("SpotLight_").Append(BeamComponentName))) this->SpotLight = Cast<USpotLightComponent>(Child);
		else if (Child->GetName().Equals(FString("PointLight_").Append(BeamComponentName))) this->PointLight = Cast<UPointLightComponent>(Child);
	}
}

/**
 * Setup the component for a specific GDTF Beam Description
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 08 june 2022
 *
 * @param BeamDescription Description of the Beam to construct
 * @param Model Model linked to this beam if any on GDTF file (represent Lens mesh)
 * @param FixturePathOnContentBrowser Path of the fixture in Content Browser (ex: "/Game/MyMovingHead")
 * @return True if everything OK.
*/
bool UCPGDTFBeamSceneComponent::PreConstruct(UCPGDTFDescriptionGeometryBeam* BeamDescription, FCPGDTFDescriptionModel* Model, FString FixturePathOnContentBrowser) {

	// If the model provided is not the one referenced in the BeamDescription
	if (Model != nullptr && !BeamDescription->Model.IsEqual(Model->Name)) return false;

	/* Unused GDTF values

	 * ThrowRatio			(for Rectangle BeamType)
	 * RectangleRatio		(for Rectangle BeamType)
	 * BeamType				(For now only Spot)
	 *
	 * ColorRenderingIndex	(Seems unsupported by Unreal)
	 * EmitterSpectrum		(Unrelevant)
	 */

	this->LightIntensityMax = BeamDescription->LuminousFlux;

	FString AssetPath;

	FString ComponentName = this->GetName();
	ComponentName.RemoveFromStart("BEAM_", ESearchCase::CaseSensitive);

	// PointLight
	this->PointLight = NewObject<UPointLightComponent>(this->GetAttachmentRootActor(), UPointLightComponent::StaticClass(), FName(FString("PointLight_").Append(ComponentName)));
	this->PointLight->SetCastShadows(false); // Default is False but it can be edited in UI
	this->PointLight->SetIntensityUnits(ELightUnits::Lumens);
	this->PointLight->SetIntensity(BeamDescription->LuminousFlux);
	this->PointLight->SetSourceRadius(BeamDescription->BeamRadius);
	this->PointLight->bUseTemperature = true;
	this->PointLight->bAffectsWorld = false;
	this->PointLight->OnComponentCreated();
	//this->DynamicMaterialPointLight = UMaterialInstanceDynamic::Create(this->PointLightMaterialInstance, nullptr); // Needs a material instance to be usefull but not provided on example
	this->PointLight->SetupAttachment(this);
	this->GetAttachmentRootActor()->AddInstanceComponent(this->PointLight);
	//this->PointLight->RegisterComponent(); // Disabled because of log warning

	//SpotLight
	this->SpotLight = NewObject<USpotLightComponent>(this->GetAttachmentRootActor(), USpotLightComponent::StaticClass(), FName(FString("SpotLight_").Append(ComponentName)));
	this->SpotLight->SetCastShadows(false); // Default is False but it can be edited in UI
	this->SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	this->SpotLight->SetIntensity(BeamDescription->LuminousFlux);
	this->SpotLight->SetSourceRadius(BeamDescription->BeamRadius);
	this->SpotLight->SetInnerConeAngle(BeamDescription->BeamAngle / 2.0f); // / 2.0 because the angle in Unreal is between the center and the border of the beam
	this->SpotLight->SetOuterConeAngle(BeamDescription->FieldAngle / 2.0f);
	this->SpotLight->bUseTemperature = true;
	this->SpotLight->bAffectsWorld = true;
	AssetPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	AssetPath += "MaterialInstances/MI_Light.MI_Light";
	this->SpotLightMaterialInstance = Cast<UMaterialInstance>(FCPGDTFImporterUtils::LoadObjectByPath(AssetPath));
	this->DynamicMaterialSpotLight = UMaterialInstanceDynamic::Create(this->SpotLightMaterialInstance, nullptr);
	this->SpotLight->OnComponentCreated();
	this->SpotLight->SetupAttachment(this);
	this->GetAttachmentRootActor()->AddInstanceComponent(this->SpotLight);
	//this->SpotLight->RegisterComponent(); // Disabled because of log warning

	// Beam
	this->BeamStaticMeshComponent = NewObject<UStaticMeshComponent>(this->GetAttachmentRootActor(), UStaticMeshComponent::StaticClass(), FName(FString("CPSM_BEAM_").Append(ComponentName)));
	AssetPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	AssetPath += "GenericMeshes/SM_Beam.SM_Beam";
	this->BeamStaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(FCPGDTFImporterUtils::LoadObjectByPath(AssetPath)));
	this->BeamStaticMeshComponent->SetRelativeRotation(FRotator(90, 0, 0));
	AssetPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	AssetPath += "MaterialInstances/MI_Beam.MI_Beam";
	this->BeamMaterialInstance = Cast<UMaterialInstance>(FCPGDTFImporterUtils::LoadObjectByPath(AssetPath));
	this->DynamicMaterialBeam = UMaterialInstanceDynamic::Create(this->BeamMaterialInstance, nullptr);
	this->BeamStaticMeshComponent->OnComponentCreated();
	this->BeamStaticMeshComponent->SetupAttachment(this);
	this->GetAttachmentRootActor()->AddInstanceComponent(this->BeamStaticMeshComponent);
	//this->BeamStaticMeshComponent->RegisterComponent(); // Disabled because of log warning

	// Lens
	this->LensStaticMeshComponent = NewObject<UStaticMeshComponent>(this->GetAttachmentRootActor(), UStaticMeshComponent::StaticClass(), FName(FString("CPSM_Lens_").Append(ComponentName)));
	this->LensStaticMeshComponent->SetRelativeRotation(FRotator(90, 0, 0).Quaternion());
	if (Model) { // If a model is provided

		// Load of Mesh from Content Browser
		if (!FixturePathOnContentBrowser.EndsWith("/")) FixturePathOnContentBrowser.Append("/"); // Make sure that the path end with a slash
		UStaticMesh* LensMesh;
		if (Model->PrimitiveType == ECPGDTFDescriptionModelsPrimitiveType::Undefined) // If type is Undefined the mesh was embeded in GDTF file
			LensMesh = FCPGDTFImporterUtils::LoadMeshesInFolder(FixturePathOnContentBrowser + "models/" + Model->Name.ToString())[0]; // In case of multiple asset we only use the first one
		else LensMesh = FCPGDTFImporterUtils::LoadGDTFGenericMesh(Model->PrimitiveType); // else this is a generic one

		if (LensMesh == nullptr) return false;

		// Calc position and scale of mesh
		FVector ActualSize = LensMesh->GetBounds().GetBox().GetSize();
		FVector ScaleFactor = FVector(Model->Length, Model->Width, Model->Height) / ActualSize;

		// Apply to the Component
		this->LensStaticMeshComponent->SetStaticMesh(LensMesh);
		this->LensStaticMeshComponent->SetRelativeScale3D(ScaleFactor);
	}
	AssetPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	AssetPath += "MaterialInstances/MI_Lens.MI_Lens";
	this->LensMaterialInstance = Cast<UMaterialInstance>(FCPGDTFImporterUtils::LoadObjectByPath(AssetPath));
	this->DynamicMaterialLens = UMaterialInstanceDynamic::Create(this->LensMaterialInstance, nullptr);
	this->LensStaticMeshComponent->OnComponentCreated();
	this->LensStaticMeshComponent->SetupAttachment(this);
	this->GetAttachmentRootActor()->AddInstanceComponent(this->LensStaticMeshComponent);
	//this->LensStaticMeshComponent->RegisterComponent(); // Disabled because of log warning

	// Occlusion Direction
	this->OcclusionDirection = NewObject<UArrowComponent>(this->GetAttachmentRootActor(), UArrowComponent::StaticClass(), FName(FString("Occlusion_").Append(ComponentName)));
	this->OcclusionDirection->ArrowSize = 0.5f;
	this->OcclusionDirection->ArrowLength = 40.0f;
	this->OcclusionDirection->OnComponentCreated();
	this->OcclusionDirection->SetupAttachment(this);
	this->GetAttachmentRootActor()->AddInstanceComponent(this->OcclusionDirection);
	//this->OcclusionDirection->RegisterComponent(); // Disabled because of log warning

	this->DefaultLightColorTemp = BeamDescription->ColorTemperature;
	this->SetLightColorTemp(BeamDescription->ColorTemperature);

	return true;
}

void UCPGDTFBeamSceneComponent::SetLightCastShadow(bool bLightShouldCastShadow) {
	
	this->LightCastShadow = bLightShouldCastShadow;
	this->SpotLight->SetCastShadows(bLightShouldCastShadow);
	this->PointLight->SetCastShadows(bLightShouldCastShadow);
}

void UCPGDTFBeamSceneComponent::SetLightDistanceMax(float NewLightDistanceMax) {
	
	this->LightDistanceMax = NewLightDistanceMax;

	if (this->DynamicMaterialBeam) {
		this->DynamicMaterialBeam->SetScalarParameterValue("DMX Max Light Distance", NewLightDistanceMax);
	}

	this->SpotLight->SetAttenuationRadius(NewLightDistanceMax);
	this->PointLight->SetAttenuationRadius(NewLightDistanceMax);
}

void UCPGDTFBeamSceneComponent::SetLightColorTemp(float NewLightColorTemp) {

	this->LightColorTemp = NewLightColorTemp;
	this->SpotLight->SetTemperature(this->LightColorTemp);
	this->PointLight->SetTemperature(this->LightColorTemp);
	if (this->DynamicMaterialBeam) {
		this->DynamicMaterialBeam->SetVectorParameterValue(FName("DMX Color Temperature"), FLinearColor::MakeFromColorTemperature(this->LightColorTemp));
	}
}

void UCPGDTFBeamSceneComponent::SetLightColor(FLinearColor NewLightColor) {

	this->SpotLight->SetLightColor(NewLightColor);
	this->PointLight->SetLightColor(NewLightColor);
	if (this->DynamicMaterialBeam) this->DynamicMaterialBeam->SetVectorParameterValue("DMX Color", NewLightColor);
	if (this->DynamicMaterialLens) this->DynamicMaterialLens->SetVectorParameterValue("DMX Color", NewLightColor);
}

void UCPGDTFBeamSceneComponent::SetSpotlightIntensityScale(float NewSpotlightIntensityScale) {

	this->SpotlightIntensityScale = NewSpotlightIntensityScale;
	this->SpotLight->SetIntensity(this->LightIntensityMax * NewSpotlightIntensityScale);

	if (this->DynamicMaterialBeam) this->DynamicMaterialBeam->SetScalarParameterValue("DMX Max Light Intensity", this->LightIntensityMax * NewSpotlightIntensityScale);
	if (this->DynamicMaterialLens) this->DynamicMaterialLens->SetScalarParameterValue("DMX Max Light Intensity", this->LightIntensityMax * NewSpotlightIntensityScale);
}

void UCPGDTFBeamSceneComponent::SetPointlightIntensityScale(float NewPointlightIntensityScale) {
	
	this->PointlightIntensityScale = NewPointlightIntensityScale;
	this->PointLight->SetIntensity(this->LightIntensityMax * NewPointlightIntensityScale);
}

void UCPGDTFBeamSceneComponent::ToggleLightVisibility() {

	if (!this->DynamicMaterialBeam->IsValidLowLevel()) return;

	FHashedMaterialParameterInfo ParameterName = FHashedMaterialParameterInfo("DMX Dimmer");
	float DimmerValue;
	if (!this->DynamicMaterialBeam->GetScalarParameterValue(ParameterName, DimmerValue)) return;

	FLinearColor LightColor = this->SpotLight->GetLightColor();

	bool NewVisibility = LightColor != FLinearColor::Black && DimmerValue > 0.0f;

	this->SpotLight->SetVisibility(NewVisibility);
	this->BeamStaticMeshComponent->SetVisibility(NewVisibility);
}

bool UCPGDTFBeamSceneComponent::IsMoving() {

	bool isNearlyEqual = UKismetMathLibrary::NearlyEqual_TransformTransform(this->LensStaticMeshComponent->GetComponentToWorld(), this->PreviousHeadTransform, 0.0001f, 0.0001f, 0.0001f);
	if (isNearlyEqual) this->PreviousHeadTransform = this->LensStaticMeshComponent->GetComponentToWorld();
	return isNearlyEqual;
}

void UCPGDTFBeamSceneComponent::CheckOcclusion() {

	if (this->IsMoving()) {
		FVector Start, End;
		Start = this->OcclusionDirection->GetComponentLocation();
		End = this->OcclusionDirection->GetForwardVector() * this->LightDistanceMax + Start;
		FHitResult OutHit;

		if (UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, {}, EDrawDebugTrace::Type::None, OutHit, true, FLinearColor::Red, FLinearColor::Green, 10.0f))
			this->DynamicMaterialBeam->SetScalarParameterValue(FName(""), FMath::Min(OutHit.Distance + 25.0f / this->LightDistanceMax, 1.0f));
		else this->DynamicMaterialBeam->SetScalarParameterValue(FName(""), 1.0f);
	}
}

#if WITH_EDITOR
void UCPGDTFBeamSceneComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {

	Super::PostEditChangeProperty(PropertyChangedEvent);
	this->UpdateProperties();
}
#endif

void UCPGDTFBeamSceneComponent::UpdateProperties() {

	if (this->DynamicMaterialBeam) {
		this->DynamicMaterialBeam->SetScalarParameterValue("DMX Max Light Distance", LightDistanceMax);
		this->DynamicMaterialBeam->SetScalarParameterValue("DMX Max Light Intensity", LightIntensityMax * SpotlightIntensityScale);
	}

	if (this->DynamicMaterialLens) {
		this->DynamicMaterialLens->SetScalarParameterValue("DMX Max Light Intensity", LightIntensityMax * SpotlightIntensityScale);
		this->DynamicMaterialBeam->SetScalarParameterValue("DMX Lens Radius", LensRadius);
	}

	// Set lights
	this->SpotLight->SetIntensity(LightIntensityMax * SpotlightIntensityScale);
	this->SpotLight->SetTemperature(LightColorTemp);
	this->SpotLight->SetCastShadows(LightCastShadow);

	this->PointLight->SetIntensity(LightIntensityMax * PointlightIntensityScale);
	this->PointLight->SetTemperature(LightColorTemp);
	this->PointLight->SetCastShadows(LightCastShadow);

	this->SetLightDistanceMax(this->LightDistanceMax);
}

void UCPGDTFBeamSceneComponent::SetBeamQuality(float Quality) {
	if (this->DynamicMaterialBeam)
		this->DynamicMaterialBeam->SetScalarParameterValue("DMX Quality Level", Quality);
}