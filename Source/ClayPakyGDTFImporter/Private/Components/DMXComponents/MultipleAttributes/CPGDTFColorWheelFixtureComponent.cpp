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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFColorWheelFixtureComponent.h"
#include "Factories/Importers/Wheels/CPGDTFWheelImporter.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "PackageTools.h"

void UCPGDTFColorWheelFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels) {

	Super::Setup(AttachedGeometryNamee, DMXChannels);
	if (DMXChannels[0].Offset.Num() > 0) this->ChannelAddress = DMXChannels[0].Offset[0];
	else this->ChannelAddress = -1;
	this->GDTFDMXChannelDescription = DMXChannels[0];
	this->DMXChannelTree.Insert(this->GDTFDMXChannelDescription.LogicalChannels[0], this->GDTFDMXChannelDescription.Offset.Num());

	FDMXImportGDTFWheel Wheel = this->GDTFDMXChannelDescription.LogicalChannels[0].ChannelFunctions[0].Wheel;
	this->WheelColors = FCPGDTFWheelImporter::GenerateColorArray(Wheel);
	
	FString TextureLoadPath = this->GetParentFixtureActor()->GDTFDescription->GetFixtureSavePath();
	FString SanitizedWheelName = UPackageTools::SanitizePackageName(Wheel.Name.ToString());
	TextureLoadPath.Append("textures/" + SanitizedWheelName + "/Wheel_" + SanitizedWheelName + ".Wheel_" + SanitizedWheelName);
	this->WheelTexture = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(TextureLoadPath));
	
	this->InterpolationScale = 0.25f;
	this->bIsRawDMXEnabled = true;
	this->bUseInterpolation = true;
}

void UCPGDTFColorWheelFixtureComponent::BeginPlay() {

	Super::BeginPlay();
	for (UCPGDTFBeamSceneComponent* Beam : this->AttachedBeams) {

		if (!Beam->HasBegunPlay()) Beam->BeginPlay(); // To avoid a skip of color disk

		if (Beam->DynamicMaterialBeam) {
			Beam->DynamicMaterialBeam->SetTextureParameterValue("DMX Color Disk", this->WheelTexture);
			Beam->DynamicMaterialBeam->SetScalarParameterValue("DMX Color Wheel Num Mask", this->WheelColors.Num());
		}
		if (Beam->DynamicMaterialLens) {
			Beam->DynamicMaterialLens->SetTextureParameterValue("DMX Color Disk", this->WheelTexture);
			Beam->DynamicMaterialLens->SetScalarParameterValue("DMX Color Wheel Num Mask", this->WheelColors.Num());
		}
	}

	this->CurrentWheelIndex = FChannelInterpolation();
	this->CurrentWheelIndex.InterpolationScale = this->InterpolationScale;
	this->CurrentWheelIndex.RangeSize = this->WheelColors.Num();
	this->CurrentWheelIndex.CurrentValue = 0;
	this->CurrentWheelIndex.TargetValue = 0;
	this->CurrentWheelIndex.StartTravel(0);


	if (this->DMXChannelTree.IsEmpty()) this->DMXChannelTree.Insert(this->GDTFDMXChannelDescription.LogicalChannels[0], this->GDTFDMXChannelDescription.Offset.Num());
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->bIsMacroColor = false;
	this->bUseInterpolation = true;
	this->CurrentColor = FLinearColor(0, 0, 0);
}

void UCPGDTFColorWheelFixtureComponent::PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap) {

	const int32* DMXValuePtr = RawValuesMap.Find(this->ChannelAddress);
	if (DMXValuePtr) this->ApplyEffectToBeam(*DMXValuePtr);
}

void UCPGDTFColorWheelFixtureComponent::SetTargetValue(float WheelIndex) {

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
 * @date 18 july 2022
 *
 * @param DMXValue
*/
void UCPGDTFColorWheelFixtureComponent::ApplyEffectToBeam(int32 DMXValue) {

	if (this->DMXChannelTree.IsEmpty()) return;

	TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> DMXBehaviour = this->DMXChannelTree.GetBehaviourByDMXValue(DMXValue);	
	// If we are unable to find the behaviour in the tree we can't do anything
	if (DMXBehaviour.Key == nullptr || DMXBehaviour.Value == nullptr) return;

	ECPGDTFAttributeType AttributeType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(DMXBehaviour.Key->Attribute.Name.ToString());
	float PhysicalValue = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, DMXBehaviour.Value->PhysicalFrom, DMXBehaviour.Value->PhysicalTo);

	switch (AttributeType) {

	case ECPGDTFAttributeType::Color_n_:
	case ECPGDTFAttributeType::Color_n_WheelIndex:
		this->ColorWheelPeriod = 0;
		this->CurrentTime = 0;
		this->SetTargetValue(PhysicalValue + (float)DMXBehaviour.Value->WheelSlotIndex);
		break;

	case ECPGDTFAttributeType::ColorMacro_n_: // Color Macro or virtual color wheel
		this->ColorWheelPeriod = 0;
		this->CurrentTime = 0;
		this->CurrentWheelIndex.SetValueNoInterp(PhysicalValue + (float)DMXBehaviour.Value->WheelSlotIndex);
		this->SetValueNoInterp_OverWrite(PhysicalValue + (float)DMXBehaviour.Value->WheelSlotIndex);
		break;

	case ECPGDTFAttributeType::Color_n_WheelSpin:
		// We stop interpolation
		this->CurrentWheelIndex.IsUpdating = false;
		this->CurrentWheelIndex.TargetValue = this->CurrentWheelIndex.CurrentValue;

		this->CurrentTime = 0;
		if (PhysicalValue == 0) this->ColorWheelPeriod = 0;
		else this->ColorWheelPeriod = 360 / PhysicalValue; // PhysicalValue is an Angular speed in deg/s
		break;

	case ECPGDTFAttributeType::Color_n_WheelRandom:
		// We stop interpolation
		this->CurrentWheelIndex.IsUpdating = false;
		this->CurrentWheelIndex.TargetValue = this->CurrentWheelIndex.CurrentValue;

		// Here the Physical value is a frenquency in Hz. If the GDTF use default physical values we fallback on default ones
		if (DMXBehaviour.Value->PhysicalFrom == 0 && DMXBehaviour.Value->PhysicalTo == 1) this->ColorWheelPeriod = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, 0.2, 5);
		else this->ColorWheelPeriod = PhysicalValue;
		break;

	case ECPGDTFAttributeType::Color_n_WheelAudio: // Very complex to implement. For now we disable ColorWheel
	default:// Not supported behaviour
		if (DMXValue == this->GDTFDMXChannelDescription.Default.Value) this->SetTargetValue(0); // To avoid stack overflows
		else this->ApplyEffectToBeam(this->GDTFDMXChannelDescription.Default.Value);
		break;
	}
	this->RunningEffectType = AttributeType;
}

void UCPGDTFColorWheelFixtureComponent::InterpolateComponent(float DeltaSeconds) {

	if (this->AttachedBeams.Num() < 1) return;

	float WheelIndex;

	switch (this->RunningEffectType) {

	case ECPGDTFAttributeType::ColorMacro_n_:
		SetValueNoInterp_OverWrite(this->CurrentWheelIndex.CurrentValue);
		break;

	case ECPGDTFAttributeType::Color_n_WheelSpin:

		if (this->ColorWheelPeriod == 0) break;
		
		WheelIndex = (this->CurrentWheelIndex.CurrentValue + (this->WheelColors.Num()) * (DeltaSeconds / this->ColorWheelPeriod));
		while (WheelIndex >= this->WheelColors.Num()) {
			WheelIndex -= this->WheelColors.Num();
		}
		while (WheelIndex <= 0) {
			WheelIndex += this->WheelColors.Num();
		}
		this->SetValueNoInterp(WheelIndex);
		this->CurrentWheelIndex.SetValueNoInterp(WheelIndex);
		break;

	case ECPGDTFAttributeType::Color_n_WheelRandom:

		this->CurrentTime += DeltaSeconds;
		if (this->CurrentTime >= this->ColorWheelPeriod) {
			this->CurrentTime = 0;
			this->SetTargetValue(FMath::RandHelper(this->WheelColors.Num()));
		}
		break;
		
	default:
		break;
	}

	if (this->CurrentWheelIndex.IsUpdating) {
		this->CurrentWheelIndex.Travel(DeltaSeconds); // Update
		if (this->CurrentWheelIndex.IsInterpolationDone()) this->CurrentWheelIndex.EndInterpolation(); // Is done?
		this->SetValueNoInterp(this->CurrentWheelIndex.CurrentValue);
	}
}

/**
 * Apply a color to the light entire output
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 18 july 2022
 *
 * @param Beam
 * @param WheelIndex
 */
void UCPGDTFColorWheelFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float WheelIndex) {

	if (Beam == nullptr) return;

	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetScalarParameterValue("DMX Color Wheel Index", WheelIndex);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetScalarParameterValue("DMX Color Wheel Index", WheelIndex);

	int32 IntWheelIndex = FMath::TruncToInt(WheelIndex);
	if (IntWheelIndex < 0) IntWheelIndex += this->WheelColors.Num();
	FLinearColor CrossfadedColor;
	if (IntWheelIndex + 1 >= this->WheelColors.Num()) CrossfadedColor = UKismetMathLibrary::LinearColorLerp(this->WheelColors[IntWheelIndex], this->WheelColors[0], FMath::Fractional(WheelIndex));
	else CrossfadedColor = UKismetMathLibrary::LinearColorLerp(this->WheelColors[IntWheelIndex], this->WheelColors[IntWheelIndex + 1], FMath::Fractional(WheelIndex));

	this->CurrentColor = FLinearColor(1, 1, 1, 1) - CrossfadedColor; // We need to invert the color to have the filter one and not the beam color
	this->bIsMacroColor = false;
}

/**
 * Apply a color to the light output overwriting other color components values.
 * Used for color macros and virtual color wheels
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 04 august 2022
 *
 * @param WheelIndex
 */
void UCPGDTFColorWheelFixtureComponent::SetValueNoInterp_OverWrite(float WheelIndex) {

	for (UCPGDTFBeamSceneComponent* Component : this->AttachedBeams) {

		Component->ResetLightColorTemp();
		Component->SetLightColor(FLinearColor(1, 1, 1)); // Set White base color
		SetValueNoInterp_BeamInternal(Component, WheelIndex);
		this->bIsMacroColor = true;
	}
}