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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFIrisFixtureComponent.h"

bool UCPGDTFIrisFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(AttachedGeometryNamee, DMXChannels, attributeIndex);
	this->bIsRawDMXEnabled = true;
	this->bUseInterpolation = true;
	return true;
}

void UCPGDTFIrisFixtureComponent::BeginPlay() {
	this->mIrisParamName = *CPGDTFRenderPipelineBuilder::getIrisParamName();
	Super::BeginPlay(ECPGDTFAttributeType::Iris);
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->bUseInterpolation = true;
}

float UCPGDTFIrisFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.7 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFIrisFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.15 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFIrisFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 2.66 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFIrisFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.214 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFIrisFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> attributes;

	TSet<ECPGDTFAttributeType> iris;
	iris.Add(ECPGDTFAttributeType::Iris);
	attributes.Add(iris);

	TSet<ECPGDTFAttributeType> irisPulse;
	irisPulse.Add(ECPGDTFAttributeType::IrisPulse);
	irisPulse.Add(ECPGDTFAttributeType::IrisPulseOpen);
	irisPulse.Add(ECPGDTFAttributeType::IrisPulseClose);
	attributes.Add(irisPulse);

	TSet<ECPGDTFAttributeType> irisStrobe;
	irisStrobe.Add(ECPGDTFAttributeType::IrisStrobe);
	attributes.Add(irisStrobe);

	TSet<ECPGDTFAttributeType> irisStrobeRand;
	irisStrobeRand.Add(ECPGDTFAttributeType::IrisStrobeRandom);
	attributes.Add(irisStrobeRand);

	TSet<ECPGDTFAttributeType> irisPulseRand;
	irisPulseRand.Add(ECPGDTFAttributeType::IrisRandomPulseOpen);
	irisPulseRand.Add(ECPGDTFAttributeType::IrisRandomPulseClose);
	attributes.Add(irisPulseRand);

	return attributes;
}

void UCPGDTFIrisFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	switch (AttributeType) {

	case ECPGDTFAttributeType::Iris:
		this->SetTargetValue(physicalValue, 0);
		break;

	case ECPGDTFAttributeType::IrisPulse:
	case ECPGDTFAttributeType::IrisPulseOpen:
	case ECPGDTFAttributeType::IrisPulseClose:
		if (channel.RunningEffectTypeChannel != AttributeType) this->StartPulseEffect(AttributeType, 1 / physicalValue, DMXBehaviour.Key);
		else this->PulseManager.ChangePeriod(1 / physicalValue);
		break;

	case ECPGDTFAttributeType::IrisStrobe:
		break;
	case ECPGDTFAttributeType::IrisStrobeRandom:
		break;
	case ECPGDTFAttributeType::IrisRandomPulseOpen:
	case ECPGDTFAttributeType::IrisRandomPulseClose:
		break;

	default: break; // Not supported behaviour
	}
}

void UCPGDTFIrisFixtureComponent::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {
	switch (channel.RunningEffectTypeChannel) {
	case ECPGDTFAttributeType::IrisPulse:
	case ECPGDTFAttributeType::IrisPulseOpen:
	case ECPGDTFAttributeType::IrisPulseClose:
		this->SetTargetValue(this->PulseManager.InterpolatePulse(deltaSeconds), 0);
		break;
	default:
		break;
	}
}

void UCPGDTFIrisFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value, int interpolationId) {
	setAllScalarParameters(Beam, this->mIrisParamName, Value);
}


void UCPGDTFIrisFixtureComponent::StartPulseEffect(ECPGDTFAttributeType AttributeType, float Period, FCPGDTFDescriptionChannelFunction* ChannelFunction) {

	float DutyCycle = 1, TimeOffset = 1;

	EPulseEffectType PulseEffectType;
	switch (AttributeType) {
	case ECPGDTFAttributeType::IrisPulse:
		PulseEffectType = EPulseEffectType::Pulse;
		break;

	case ECPGDTFAttributeType::IrisPulseOpen:
		PulseEffectType = EPulseEffectType::PulseOpen;
		break;

	case ECPGDTFAttributeType::IrisPulseClose:
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