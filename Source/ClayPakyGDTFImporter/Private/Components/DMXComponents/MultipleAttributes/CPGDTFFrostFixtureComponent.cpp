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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFFrostFixtureComponent.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "PackageTools.h"

void UCPGDTFFrostFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels) {

	Super::Setup(AttachedGeometryNamee, DMXChannels);
	if (DMXChannels[0].Offset.Num() > 0) this->ChannelAddress = DMXChannels[0].Offset[0];
	else this->ChannelAddress = -1;
	this->GDTFDMXChannelDescription = DMXChannels[0];
	this->DMXChannelTree.Insert(this->GDTFDMXChannelDescription.LogicalChannels[0], this->GDTFDMXChannelDescription.Offset.Num());
	
	this->bIsRawDMXEnabled = true;
}

void UCPGDTFFrostFixtureComponent::BeginPlay() {

	Super::BeginPlay();
	if (this->DMXChannelTree.IsEmpty()) this->DMXChannelTree.Insert(this->GDTFDMXChannelDescription.LogicalChannels[0], this->GDTFDMXChannelDescription.Offset.Num());
	this->bIsRawDMXEnabled = true; // Just to make sure
}

void UCPGDTFFrostFixtureComponent::PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap) {

	const int32* DMXValuePtr = RawValuesMap.Find(this->ChannelAddress);
	if (DMXValuePtr) this->ApplyEffectToBeam(*DMXValuePtr);
}

/**
 * Read DMXValue and apply the effect to the Beam
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 04 august 2022
 *
 * @param DMXValue
 */
void UCPGDTFFrostFixtureComponent::ApplyEffectToBeam(int32 DMXValue) {

	if (this->DMXChannelTree.IsEmpty()) return;

	TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> DMXBehaviour = this->DMXChannelTree.GetBehaviourByDMXValue(DMXValue);	
	// If we are unable to find the behaviour in the tree we can't do anything
	if (DMXBehaviour.Key == nullptr || DMXBehaviour.Value == nullptr) return;

	ECPGDTFAttributeType AttributeType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(DMXBehaviour.Key->Attribute.Name.ToString());
	float PhysicalValue = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, DMXBehaviour.Value->PhysicalFrom, DMXBehaviour.Value->PhysicalTo);

	switch (AttributeType) {

	case ECPGDTFAttributeType::Frost_n_:
		this->SetValueNoInterp(PhysicalValue);
		break;

	case ECPGDTFAttributeType::Frost_n_Ramp:
	case ECPGDTFAttributeType::Frost_n_PulseOpen:
	case ECPGDTFAttributeType::Frost_n_PulseClose:
		if (this->RunningEffectType != AttributeType) this->StartPulseEffect(AttributeType, 1 / PhysicalValue, DMXBehaviour.Key);
		else this->PulseManager.ChangePeriod(1 / PhysicalValue);
		break;

	default:// Not supported behaviour
		if (DMXValue == this->GDTFDMXChannelDescription.Default.Value) this->SetValueNoInterp(0); // To avoid stack overflows
		else this->ApplyEffectToBeam(this->GDTFDMXChannelDescription.Default.Value);
		break;
	}
	this->RunningEffectType = AttributeType;
}

/**
 * Start a PulseEffect for given parameters
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 04 august 2022
 * 
 * @param AttributeType 
 * @param Period 
 * @param ChannelFunction 
 */
void UCPGDTFFrostFixtureComponent::StartPulseEffect(ECPGDTFAttributeType AttributeType, float Period, FCPGDTFDescriptionChannelFunction* ChannelFunction) {

	float DutyCycle = 1, TimeOffset = 1;

	EPulseEffectType PulseEffectType;
	switch (AttributeType) {
	case ECPGDTFAttributeType::Frost_n_Ramp:
		PulseEffectType = EPulseEffectType::Pulse;
		break;

	case ECPGDTFAttributeType::Frost_n_PulseOpen:
		PulseEffectType = EPulseEffectType::PulseOpen;
		break;

	case ECPGDTFAttributeType::Frost_n_PulseClose:
		PulseEffectType = EPulseEffectType::PulseClose;
		break;
	default:
		return;
	}

	for (FDMXImportGDTFSubPhysicalUnit SubPhysical : ChannelFunction->Attribute.SubPhysicalUnits) {
		switch (SubPhysical.Type) {
		case EDMXImportGDTFSubPhysicalUnitType::DutyCycle:
			if (SubPhysical.PhysicalUnit != EDMXImportGDTFPhysicalUnit::Percent) {
				UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Fixture %s, On Frost Pulse effect DutyCycle value unit '%s' is unsupported. Value ignored, please provide percent."), *this->GetParentFixtureActor()->GetFName().ToString(), *StaticEnum<EDMXImportGDTFPhysicalUnit>()->GetNameStringByValue((uint8)SubPhysical.PhysicalUnit));
			} else DutyCycle = (SubPhysical.PhysicalFrom + SubPhysical.PhysicalTo) / 200;
			break;
		case EDMXImportGDTFSubPhysicalUnitType::TimeOffset:
			if (SubPhysical.PhysicalUnit != EDMXImportGDTFPhysicalUnit::Percent) {
				UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Fixture %s, On Frost Pulse effect TimeOffset value unit '%s' is unsupported. Value ignored, please provide percent."), *this->GetParentFixtureActor()->GetFName().ToString(), *StaticEnum<EDMXImportGDTFPhysicalUnit>()->GetNameStringByValue((uint8)SubPhysical.PhysicalUnit));
			} else TimeOffset = (SubPhysical.PhysicalFrom + SubPhysical.PhysicalTo) / 200;
			break;
		default:
			break;
		}
	}
	this->PulseManager.SetSettings(PulseEffectType, Period, DutyCycle, TimeOffset);
}

void UCPGDTFFrostFixtureComponent::InterpolateComponent(float DeltaSeconds) {

	if (this->AttachedBeams.Num() < 1) return;

	switch (this->RunningEffectType) {

	case ECPGDTFAttributeType::Frost_n_Ramp:
	case ECPGDTFAttributeType::Frost_n_PulseOpen:
	case ECPGDTFAttributeType::Frost_n_PulseClose:
		this->SetValueNoInterp(this->PulseManager.InterpolatePulse(DeltaSeconds));
		break;

	default:
		break;
	}
}

void UCPGDTFFrostFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value) {
	
	if (Beam->DynamicMaterialBeam) {
		Beam->DynamicMaterialBeam->SetScalarParameterValue("DMX Frost", Value);
	}
	if (Beam->DynamicMaterialLens) {
		Beam->DynamicMaterialLens->SetScalarParameterValue("DMX Frost", Value);
	}
	if (Beam->DynamicMaterialSpotLight) {
		Beam->DynamicMaterialSpotLight->SetScalarParameterValue("DMX Frost", Value);
	}
}