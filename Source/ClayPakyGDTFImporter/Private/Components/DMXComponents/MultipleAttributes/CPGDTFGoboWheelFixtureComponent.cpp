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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFGoboWheelFixtureComponent.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "PackageTools.h"

void UCPGDTFGoboWheelFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels) {

	Super::Setup(AttachedGeometryNamee, DMXChannels);
	if (DMXChannels[0].Offset.Num() > 0) this->AddressChannelOne = DMXChannels[0].Offset[0];
	else this->AddressChannelOne = -1;
	this->GDTFDMXChannelDescriptionOne = DMXChannels[0];
	this->DMXChannelTreeOne.Insert(this->GDTFDMXChannelDescriptionOne.LogicalChannels[0], this->GDTFDMXChannelDescriptionOne.Offset.Num());

	if (DMXChannels.Num() > 1) {
		if (DMXChannels[1].Offset.Num() > 0) this->AddressChannelTwo = DMXChannels[1].Offset[0];
		else this->AddressChannelTwo = -1;
		this->GDTFDMXChannelDescriptionTwo = DMXChannels[1];
		this->DMXChannelTreeTwo.Insert(this->GDTFDMXChannelDescriptionTwo.LogicalChannels[0], this->GDTFDMXChannelDescriptionTwo.Offset.Num());
	}

	FDMXImportGDTFWheel Wheel = this->GDTFDMXChannelDescriptionOne.LogicalChannels[0].ChannelFunctions[0].Wheel;
	FString TextureLoadPath = this->GetParentFixtureActor()->GDTFDescription->GetFixtureSavePath();
	FString TextureLoadPathFrosted = TextureLoadPath;
	FString SanitizedWheelName = UPackageTools::SanitizePackageName(Wheel.Name.ToString());
	TextureLoadPath.Append("textures/" + SanitizedWheelName + "/Wheel_" + SanitizedWheelName + ".Wheel_" + SanitizedWheelName);
	TextureLoadPathFrosted.Append("textures/" + SanitizedWheelName + "/Wheel_" + SanitizedWheelName + "_Frosted.Wheel_" + SanitizedWheelName + "_Frosted");
	this->WheelTexture = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(TextureLoadPath));
	this->WheelTextureFrosted = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(TextureLoadPathFrosted));
	this->NbrGobos = Wheel.Slots.Num();

	this->InterpolationScale = 0.25f;
	this->bIsRawDMXEnabled = true;
}

void UCPGDTFGoboWheelFixtureComponent::BeginPlay() {

	Super::BeginPlay();
	for (UCPGDTFBeamSceneComponent* Beam : this->AttachedBeams) {

		if (!Beam->HasBegunPlay()) Beam->BeginPlay(); // To avoid a skip of gobos disk

		if (Beam->DynamicMaterialBeam) {
			Beam->DynamicMaterialBeam->SetTextureParameterValue("DMX Gobo Disk", this->WheelTexture);
			Beam->DynamicMaterialBeam->SetTextureParameterValue("DMX Gobo Disk Frosted", this->WheelTextureFrosted);
			Beam->DynamicMaterialBeam->SetScalarParameterValue("DMX Gobo Num Mask", this->NbrGobos);
		}
		if (Beam->DynamicMaterialLens) {
			Beam->DynamicMaterialLens->SetTextureParameterValue("DMX Gobo Disk", this->WheelTexture);
			Beam->DynamicMaterialLens->SetTextureParameterValue("DMX Gobo Disk Frosted", this->WheelTextureFrosted);
			Beam->DynamicMaterialLens->SetScalarParameterValue("DMX Gobo Num Mask", this->NbrGobos);
		}
		if (Beam->DynamicMaterialSpotLight) {
			Beam->DynamicMaterialSpotLight->SetTextureParameterValue("DMX Gobo Disk", this->WheelTexture);
			Beam->DynamicMaterialSpotLight->SetTextureParameterValue("DMX Gobo Disk Frosted", this->WheelTextureFrosted);
			Beam->DynamicMaterialSpotLight->SetScalarParameterValue("DMX Gobo Num Mask", this->NbrGobos);
		}
	}

	this->CurrentWheelIndex = FChannelInterpolation();
	this->CurrentWheelIndex.InterpolationScale = this->InterpolationScale;
	this->CurrentWheelIndex.RangeSize = this->NbrGobos;
	this->CurrentWheelIndex.CurrentValue = 0;
	this->CurrentWheelIndex.TargetValue = 0;
	this->CurrentWheelIndex.StartTravel(0);

	if (this->DMXChannelTreeOne.IsEmpty()) this->DMXChannelTreeOne.Insert(this->GDTFDMXChannelDescriptionOne.LogicalChannels[0], this->GDTFDMXChannelDescriptionOne.Offset.Num());
	if (this->DMXChannelTreeTwo.IsEmpty() && this->GDTFDMXChannelDescriptionTwo.LogicalChannels.Num() > 0) this->DMXChannelTreeTwo.Insert(this->GDTFDMXChannelDescriptionTwo.LogicalChannels[0], this->GDTFDMXChannelDescriptionTwo.Offset.Num());
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->bUseInterpolation = true;
}

void UCPGDTFGoboWheelFixtureComponent::PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap) {

	const int32* DMXValuePtrOne = RawValuesMap.Find(this->AddressChannelOne);
	if (DMXValuePtrOne) this->ApplyEffectToBeam(*DMXValuePtrOne, true);
	const int32* DMXValuePtrTwo = RawValuesMap.Find(this->AddressChannelTwo);
	if (DMXValuePtrTwo) this->ApplyEffectToBeam(*DMXValuePtrTwo, false);
}

void UCPGDTFGoboWheelFixtureComponent::SetTargetValue(float WheelIndex) {

	if (this->HasBegunPlay()) {
		if (this->bUseInterpolation) {
			if (this->CurrentWheelIndex.bFirstValueWasSet)
				// Only 'Push' the next value into interpolation. BPs will read the resulting value on tick.
				this->CurrentWheelIndex.Push(WheelIndex);
			else {
				// Jump to the first value if it never was set
				this->CurrentWheelIndex.SetValueNoInterp(WheelIndex);
				this->CurrentWheelIndex.bFirstValueWasSet = true;

				SetValueNoInterp(WheelIndex);
			}
		} else {
			this->CurrentWheelIndex.SetValueNoInterp(WheelIndex);
			SetValueNoInterp(WheelIndex);
		}
	}
}

/**
 * Read DMXValue and apply the effect to the Beam
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 27 july 2022
 *
 * @param DMXValue
 * @param IsFirstChannel
*/
void UCPGDTFGoboWheelFixtureComponent::ApplyEffectToBeam(int32 DMXValue, bool IsFirstChannel) {

	FDMXChannelTree DMXChannelTree;
	ECPGDTFAttributeType* RunningEffectType;
	if (IsFirstChannel) {
		DMXChannelTree = this->DMXChannelTreeOne;
		RunningEffectType = &this->RunningEffectTypeChannelOne;
	} else {
		DMXChannelTree = this->DMXChannelTreeTwo;
		RunningEffectType = &this->RunningEffectTypeChannelTwo;
	}

	if (DMXChannelTree.IsEmpty()) return;

	TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> DMXBehaviour = DMXChannelTree.GetBehaviourByDMXValue(DMXValue);
	// If we are unable to find the behaviour in the tree we can't do anything
	if (DMXBehaviour.Key == nullptr || DMXBehaviour.Value == nullptr) return;

	ECPGDTFAttributeType AttributeType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(DMXBehaviour.Key->Attribute.Name.ToString());
	float PhysicalValue = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, DMXBehaviour.Value->PhysicalFrom, DMXBehaviour.Value->PhysicalTo);

	switch (AttributeType) {

	case ECPGDTFAttributeType::Gobo_n_:
	case ECPGDTFAttributeType::Gobo_n_WheelIndex: // Gobo selection
		this->GoboWheelPeriod = 0;
		this->WheelCurrentTime = 0;
		this->SetTargetValue(PhysicalValue + (float)DMXBehaviour.Value->WheelSlotIndex);
		break;

	case ECPGDTFAttributeType::Gobo_n_Pos: // Gobo indexing
		if (this->AddressChannelTwo <= 0) this->SetValueNoInterp((float)DMXBehaviour.Value->WheelSlotIndex); // If there is only one channel set the Gobo
		this->SetValueNoInterpGoboRotation(PhysicalValue); // Set the rotation
		break;

	case ECPGDTFAttributeType::Gobo_n_PosRotate: // Gobo rotation
		if (*RunningEffectType != AttributeType) this->RotationCurrentTime = 0;
		if (PhysicalValue == 0) this->GoboRotationPeriod = 0;
		else this->GoboRotationPeriod = 360 / PhysicalValue; // PhysicalValue is an Angular speed in deg/s
		break;

	case ECPGDTFAttributeType::Gobo_n_WheelSpin:
	case ECPGDTFAttributeType::Gobo_n_SelectSpin: // Gobo wheel rotation
		// We stop interpolation
		this->CurrentWheelIndex.IsUpdating = false;
		this->CurrentWheelIndex.TargetValue = this->CurrentWheelIndex.CurrentValue;

		if (*RunningEffectType != AttributeType) this->WheelCurrentTime = 0;
		if (PhysicalValue == 0) this->GoboWheelPeriod = 0;
		else this->GoboWheelPeriod = 360 / PhysicalValue; // PhysicalValue is an Angular speed in deg/s
		break;

	case ECPGDTFAttributeType::Gobo_n_WheelShake:
	case ECPGDTFAttributeType::Gobo_n_SelectShake:
	case ECPGDTFAttributeType::Gobo_n_PosShake: // Gobo shake
		// We stop interpolation
		this->CurrentWheelIndex.IsUpdating = false;
		this->CurrentWheelIndex.TargetValue = this->CurrentWheelIndex.CurrentValue;

		if (*RunningEffectType != AttributeType) this->WheelCurrentTime = 0;
		if (PhysicalValue == 0) this->GoboWheelPeriod = 0;
		else this->GoboWheelPeriod = 1 / PhysicalValue; // Physical value is a Frequency
		this->ShakeBaseIndex = DMXBehaviour.Value->WheelSlotIndex;
		break;

	case ECPGDTFAttributeType::Gobo_n_WheelRandom:
		// Here the Physical value is a frenquency in Hz. If the GDTF use default physical values we fallback on default ones
		if (DMXBehaviour.Value->PhysicalFrom == 0 && DMXBehaviour.Value->PhysicalTo == 1) this->GoboWheelPeriod = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, 0.2, 5);
		else this->GoboWheelPeriod = PhysicalValue;
		break;

	case ECPGDTFAttributeType::Gobo_n_WheelAudio: // Very complex to implement. For now we disable ColorWheel
	default:// Not supported behaviour
		if (IsFirstChannel) {
			if (DMXValue == this->GDTFDMXChannelDescriptionOne.Default.Value) this->SetTargetValue(0); // To avoid stack overflows
			else this->ApplyEffectToBeam(this->GDTFDMXChannelDescriptionOne.Default.Value);
		} else {
			if (DMXValue == this->GDTFDMXChannelDescriptionTwo.Default.Value) break; // To avoid stack overflows
			else this->ApplyEffectToBeam(this->GDTFDMXChannelDescriptionTwo.Default.Value);
		}
		break;
	}
	*RunningEffectType = AttributeType;
}

void UCPGDTFGoboWheelFixtureComponent::InterpolateComponent(float DeltaSeconds) {

	if (this->AttachedBeams.Num() < 1) return;

	float WheelIndex;

	for (int i = 0; i < 2; i++) {

		ECPGDTFAttributeType RunningEffectType;
		if (i == 0) RunningEffectType = this->RunningEffectTypeChannelOne;
		else RunningEffectType = this->RunningEffectTypeChannelTwo;		

		switch (RunningEffectType) {

		case ECPGDTFAttributeType::Gobo_n_PosRotate: // Gobo rotation

			if (this->GoboRotationPeriod == 0) break;

			WheelIndex = 360.0f * (DeltaSeconds / this->GoboRotationPeriod);
			this->SetValueNoInterpGoboDeltaRotation(WheelIndex);
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelSpin:
		case ECPGDTFAttributeType::Gobo_n_SelectSpin: // Gobo wheel rotation

			if (this->GoboWheelPeriod == 0) break;

			WheelIndex = (this->CurrentWheelIndex.CurrentValue + this->NbrGobos * (DeltaSeconds / this->GoboWheelPeriod));
			while (WheelIndex >= this->NbrGobos) {
				WheelIndex -= this->NbrGobos;
			}
			while (WheelIndex <= 0) {
				WheelIndex += NbrGobos;
			}
			this->SetValueNoInterp(WheelIndex);
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelShake:
		case ECPGDTFAttributeType::Gobo_n_SelectShake:
		case ECPGDTFAttributeType::Gobo_n_PosShake: // Gobo shake

			if (this->GoboWheelPeriod == 0) break;

			this->WheelCurrentTime += DeltaSeconds;
			this->SetValueNoInterp(this->ShakeBaseIndex + FMath::Cos(UE_TWO_PI * (this->WheelCurrentTime / this->GoboWheelPeriod)) / 20); // /20 to reduce the shake range 
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelRandom:

			this->WheelCurrentTime += DeltaSeconds;
			if (this->WheelCurrentTime >= this->GoboWheelPeriod) {
				this->WheelCurrentTime = 0;
				this->SetTargetValue(FMath::RandHelper(this->NbrGobos));
			}
			break;

		default:
			break;
		}

		if (DMXChannelTreeTwo.IsEmpty()) break; // If we have only one channel we quit the loop
	}

	if (this->CurrentWheelIndex.IsUpdating) {
		this->CurrentWheelIndex.Travel(DeltaSeconds); // Update
		if (this->CurrentWheelIndex.IsInterpolationDone()) this->CurrentWheelIndex.EndInterpolation(); // Is done?
		this->SetValueNoInterp(this->CurrentWheelIndex.CurrentValue);
	}
}

/**
 * Apply a Gobo to the light entire output
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 27 july 2022
 *
 * @param Beam
 * @param WheelIndex
 */
void UCPGDTFGoboWheelFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float WheelIndex) {

	if (Beam == nullptr) return;

	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetScalarParameterValue("DMX Gobo Index", WheelIndex);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetScalarParameterValue("DMX Gobo Index", WheelIndex);
	if (Beam->DynamicMaterialSpotLight) Beam->DynamicMaterialSpotLight->SetScalarParameterValue("DMX Gobo Index", WheelIndex);
}

/**
 * Set a rotation on the gobo
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 29 july 2022
 *
 * @param RotationAngle
 */
void UCPGDTFGoboWheelFixtureComponent::SetValueNoInterpGoboRotation(float RotationAngle) {

	for (UCPGDTFBeamSceneComponent* Beam : AttachedBeams) {
		if (Beam == nullptr) continue;
		Beam->SetRelativeRotation(FRotator(-90, 0, RotationAngle));
	}
	this->GoboCurrentAngle = RotationAngle;
}

/**
 * Set a rotation on the gobo
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 03 august 2022
 *
 * @param DeltaRotationAngle
 */
void UCPGDTFGoboWheelFixtureComponent::SetValueNoInterpGoboDeltaRotation(float DeltaRotationAngle) {

	for (UCPGDTFBeamSceneComponent* Beam : AttachedBeams) {
		if (Beam == nullptr) continue;
		Beam->SetRelativeRotation(FRotator(-90, 0, this->GoboCurrentAngle + DeltaRotationAngle));
	}
	this->GoboCurrentAngle += DeltaRotationAngle;
}