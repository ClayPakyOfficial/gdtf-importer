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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFFrostFixtureComponent.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "PackageTools.h"

bool UCPGDTFFrostFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(AttachedGeometryNamee, DMXChannels, attributeIndex);	
	this->bIsRawDMXEnabled = true;
	return true;
}

void UCPGDTFFrostFixtureComponent::BeginPlay() {
	this->mFrostParamName = *CPGDTFRenderPipelineBuilder::getFrostParamName();
	Super::BeginPlay(ECPGDTFAttributeType::Frost_n_);
	this->bIsRawDMXEnabled = true; // Just to make sure
}

float UCPGDTFFrostFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.1812 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFFrostFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.0563 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFFrostFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 1.2222 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFFrostFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.3103 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFFrostFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> attributes;

	TSet<ECPGDTFAttributeType> frost;
	frost.Add(ECPGDTFAttributeType::Frost_n_);
	attributes.Add(frost);

	TSet<ECPGDTFAttributeType> frostPulse;
	frostPulse.Add(ECPGDTFAttributeType::Frost_n_Ramp);
	frostPulse.Add(ECPGDTFAttributeType::Frost_n_PulseOpen);
	frostPulse.Add(ECPGDTFAttributeType::Frost_n_PulseClose);
	attributes.Add(frostPulse);

	return attributes;
}

/**
 * Read DMXValue and apply the effect to the Beam
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 04 august 2022
 *
 * @param DMXValue
 */
void UCPGDTFFrostFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	switch (AttributeType) {

	case ECPGDTFAttributeType::Frost_n_:
		this->SetTargetValue(physicalValue, 0);
		break;

	case ECPGDTFAttributeType::Frost_n_Ramp:
	case ECPGDTFAttributeType::Frost_n_PulseOpen:
	case ECPGDTFAttributeType::Frost_n_PulseClose:
		if (channel.RunningEffectTypeChannel != AttributeType) this->StartPulseEffect(AttributeType, 1 / physicalValue, DMXBehaviour.Key);
		else this->PulseManager.ChangePeriod(1 / physicalValue);
		break;

	default: break; // Not supported behaviour
	}
}

void UCPGDTFFrostFixtureComponent::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {
	switch (channel.RunningEffectTypeChannel) {
		case ECPGDTFAttributeType::Frost_n_Ramp:
		case ECPGDTFAttributeType::Frost_n_PulseOpen:
		case ECPGDTFAttributeType::Frost_n_PulseClose:
			this->SetTargetValue(this->PulseManager.InterpolatePulse(deltaSeconds), 0);
			break;
		default:
			break;
	}
}

void UCPGDTFFrostFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value, int interpolationId) {
	setAllScalarParameters(Beam, this->mFrostParamName, Value);
}


/**
 * Start a PulseEffect for given parameters
 * @author Dorian Gardes - Clay Paky S.R.L.
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
			}
			else DutyCycle = (SubPhysical.PhysicalFrom + SubPhysical.PhysicalTo) / 200;
			break;
		case EDMXImportGDTFSubPhysicalUnitType::TimeOffset:
			if (SubPhysical.PhysicalUnit != EDMXImportGDTFPhysicalUnit::Percent) {
				UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Fixture %s, On Frost Pulse effect TimeOffset value unit '%s' is unsupported. Value ignored, please provide percent."), *this->GetParentFixtureActor()->GetFName().ToString(), *StaticEnum<EDMXImportGDTFPhysicalUnit>()->GetNameStringByValue((uint8)SubPhysical.PhysicalUnit));
			}
			else TimeOffset = (SubPhysical.PhysicalFrom + SubPhysical.PhysicalTo) / 200;
			break;
		default:
			break;
		}
	}
	this->PulseManager.SetSettings(PulseEffectType, Period, DutyCycle, TimeOffset);
}