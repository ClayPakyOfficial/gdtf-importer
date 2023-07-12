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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFShutterFixtureComponent.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "PackageTools.h"

bool UCPGDTFShutterFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(AttachedGeometryNamee, DMXChannels, attributeIndex);
	this->bIsRawDMXEnabled = true;
	return true;
}

void UCPGDTFShutterFixtureComponent::BeginPlay() {
	//TODO LINK VALUES TO GDTF. This shouldn't be crucial, but if we find an irl fixture behaving differently on the shutter than this, check this out
	TArray<FCPDMXChannelData> data;
	FCPDMXChannelData strobeData;
	strobeData.address = channels[0].address;
	data.Add(strobeData); //Strobe
	FCPDMXChannelData intensityData;
	intensityData.address = channels[0].address;
	intensityData.DefaultValue = 1;
	data.Add(intensityData); //intensity
	Super::BeginPlay(data);
	interpolations[InterpolationIds::STROBE].bInterpolationEnabled = false;
	interpolations[InterpolationIds::INTENSITY].bInterpolationEnabled = false;
	this->SetValueNoInterp(0.0f, InterpolationIds::INTENSITY);
	this->SetValueNoInterp(0.0f, InterpolationIds::STROBE);
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFShutterFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> attributes;

	TSet<ECPGDTFAttributeType> shutter;
	shutter.Add(ECPGDTFAttributeType::Shutter_n_);
	shutter.Add(ECPGDTFAttributeType::StrobeModeShutter);
	attributes.Add(shutter);

	TSet<ECPGDTFAttributeType> strobe;
	strobe.Add(ECPGDTFAttributeType::Shutter_n_Strobe);
	strobe.Add(ECPGDTFAttributeType::Shutter_n_StrobeEffect);
	strobe.Add(ECPGDTFAttributeType::StrobeFrequency);
	strobe.Add(ECPGDTFAttributeType::StrobeModeStrobe);
	strobe.Add(ECPGDTFAttributeType::StrobeModeEffect);
	attributes.Add(strobe);

	TSet<ECPGDTFAttributeType> random;
	random.Add(ECPGDTFAttributeType::Shutter_n_StrobeRandom);
	random.Add(ECPGDTFAttributeType::StrobeModeRandom);
	attributes.Add(random);

	TSet<ECPGDTFAttributeType> pulse;
	pulse.Add(ECPGDTFAttributeType::Shutter_n_StrobePulse);
	pulse.Add(ECPGDTFAttributeType::Shutter_n_StrobePulseOpen);
	pulse.Add(ECPGDTFAttributeType::Shutter_n_StrobePulseClose);
	pulse.Add(ECPGDTFAttributeType::StrobeModePulse);
	pulse.Add(ECPGDTFAttributeType::StrobeModePulseOpen);
	pulse.Add(ECPGDTFAttributeType::StrobeModePulseClose);
	attributes.Add(pulse);

	TSet<ECPGDTFAttributeType> randomPulse;
	randomPulse.Add(ECPGDTFAttributeType::Shutter_n_StrobeRandomPulse);
	randomPulse.Add(ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseOpen);
	randomPulse.Add(ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseClose);
	randomPulse.Add(ECPGDTFAttributeType::StrobeModeRandomPulse);
	randomPulse.Add(ECPGDTFAttributeType::StrobeModeRandomPulseOpen);
	randomPulse.Add(ECPGDTFAttributeType::StrobeModeRandomPulseClose);
	attributes.Add(randomPulse);

	return attributes;
}

/**
 * Read DMXValue and apply the effect to the Beam
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 21 july 2022
 *
 * @param DMXValue
 */
void UCPGDTFShutterFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	switch (AttributeType) {

	case ECPGDTFAttributeType::Shutter_n_:
	case ECPGDTFAttributeType::StrobeModeShutter:
		if (channel.RunningEffectTypeChannel != AttributeType) this->SetValueNoInterp(0, InterpolationIds::STROBE);
		this->SetValueNoInterp(physicalValue, InterpolationIds::INTENSITY);
		break;

	case ECPGDTFAttributeType::Shutter_n_Strobe:
	case ECPGDTFAttributeType::Shutter_n_StrobeEffect: // I don't understand the difference with Shutter_n_Strobe
	case ECPGDTFAttributeType::StrobeFrequency:
	case ECPGDTFAttributeType::StrobeModeStrobe:
	case ECPGDTFAttributeType::StrobeModeEffect: // I don't understand the difference with StrobeModeStrobe
		if (channel.RunningEffectTypeChannel != AttributeType) this->SetValueNoInterp(1, InterpolationIds::INTENSITY);
		this->SetValueNoInterp(physicalValue, InterpolationIds::STROBE);
		break;

	case ECPGDTFAttributeType::Shutter_n_StrobeRandom:
	case ECPGDTFAttributeType::StrobeModeRandom:
		this->StartRandomEffect(DMXValue, DMXBehaviour.Key, DMXBehaviour.Value);
		if (channel.RunningEffectTypeChannel != AttributeType) {
			this->RandomCurrentTime = 0;
			this->SetValueNoInterp(1, InterpolationIds::INTENSITY);
		}
		this->SetValueNoInterp(this->RandomCurrentFrequency, InterpolationIds::STROBE);
		break;

	case ECPGDTFAttributeType::Shutter_n_StrobePulse:
	case ECPGDTFAttributeType::Shutter_n_StrobePulseOpen:
	case ECPGDTFAttributeType::Shutter_n_StrobePulseClose:
	case ECPGDTFAttributeType::StrobeModePulse:
	case ECPGDTFAttributeType::StrobeModePulseOpen:
	case ECPGDTFAttributeType::StrobeModePulseClose:
		if (channel.RunningEffectTypeChannel != AttributeType) this->StartPulseEffect(AttributeType, 1 / physicalValue, DMXBehaviour.Key);
		else this->PulseManager.ChangePeriod(1 / physicalValue);
		break;

	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulse:
	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseOpen:
	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseClose:
	case ECPGDTFAttributeType::StrobeModeRandomPulse:
	case ECPGDTFAttributeType::StrobeModeRandomPulseOpen:
	case ECPGDTFAttributeType::StrobeModeRandomPulseClose:
		this->StartRandomEffect(DMXValue, DMXBehaviour.Key, DMXBehaviour.Value);
		if (channel.RunningEffectTypeChannel == AttributeType) this->StartPulseEffect(AttributeType, this->RandomCurrentFrequency, DMXBehaviour.Key);
		else this->RandomCurrentTime = 0;
		break;

	default:// Not supported behaviour
		//if (DMXValue == channel.DMXChannelData.DefaultValue) this->SetValueNoInterp(0, 0); // To avoid stack overflows
		//else this->ApplyEffectToBeam(channel.DMXChannelData.DefaultValue, channel, DMXBehaviour, AttributeType, physicalValue);
		break;
	}
}

void UCPGDTFShutterFixtureComponent::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {
	switch (channel.RunningEffectTypeChannel) {

	case ECPGDTFAttributeType::Shutter_n_StrobeRandom:
		this->RandomCurrentTime += deltaSeconds;
		if (this->RandomCurrentTime > 1 / this->RandomCurrentFrequency) {
			this->RandomCurrentFrequency = FMath::FRandRange(this->RandomPhysicalFrom, this->RandomPhysicalTo);
			this->SetValueNoInterp(this->RandomCurrentFrequency, InterpolationIds::STROBE);
			this->RandomCurrentTime = 0;
		}
		break;

	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulse:
	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseOpen:
	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseClose:
		this->SetValueNoInterp(this->PulseManager.InterpolatePulse(deltaSeconds), InterpolationIds::INTENSITY);
		this->RandomCurrentTime += deltaSeconds;
		if (this->RandomCurrentTime > 1 / this->RandomCurrentFrequency) {
			this->RandomCurrentFrequency = FMath::FRandRange(this->RandomPhysicalFrom, this->RandomPhysicalTo);
			this->PulseManager.ChangePeriod(1 / this->RandomCurrentFrequency);
			this->RandomCurrentTime = 0;
		}
		break;

	case ECPGDTFAttributeType::Shutter_n_StrobePulse:
	case ECPGDTFAttributeType::Shutter_n_StrobePulseOpen:
	case ECPGDTFAttributeType::Shutter_n_StrobePulseClose:
		this->SetValueNoInterp(this->PulseManager.InterpolatePulse(deltaSeconds), InterpolationIds::INTENSITY);
		break;

	default:
		break;
	}
}

void UCPGDTFShutterFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float value, int interpolationId) {
	InterpolationIds iid = (InterpolationIds)interpolationId;
	switch (iid) {
		case InterpolationIds::STROBE: {
			setAllScalarParameters(Beam, "DMX Strobe Frequency", value);
			break;
		}
		case InterpolationIds::INTENSITY: {
			setAllScalarParameters(Beam, "DMX Strobe Open", value);
			break;
		}
	}
}

/**
 * Start a PulseEffect for given parameters
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 22 july 2022
 *
 * @param AttributeType
 * @param Period
 * @param ChannelFunction
 */
void UCPGDTFShutterFixtureComponent::StartPulseEffect(ECPGDTFAttributeType AttributeType, float Period, FCPGDTFDescriptionChannelFunction* ChannelFunction) {

	float DutyCycle = 1, TimeOffset = 1;

	EPulseEffectType PulseEffectType;
	switch (AttributeType) {
	case ECPGDTFAttributeType::Shutter_n_StrobePulse:
	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulse:
		PulseEffectType = EPulseEffectType::Pulse;
		break;

	case ECPGDTFAttributeType::Shutter_n_StrobePulseOpen:
	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseOpen:
		PulseEffectType = EPulseEffectType::PulseOpen;
		break;

	case ECPGDTFAttributeType::Shutter_n_StrobePulseClose:
	case ECPGDTFAttributeType::Shutter_n_StrobeRandomPulseClose:
		PulseEffectType = EPulseEffectType::PulseClose;
		break;
	default:
		return;
	}

	for (FDMXImportGDTFSubPhysicalUnit SubPhysical : ChannelFunction->Attribute.SubPhysicalUnits) {
		switch (SubPhysical.Type) {
		case EDMXImportGDTFSubPhysicalUnitType::DutyCycle:
			if (SubPhysical.PhysicalUnit != EDMXImportGDTFPhysicalUnit::Percent) {
				UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Fixture %s, On Shutter Pulse effect DutyCycle value unit '%s' is unsupported. Value ignored, please provide percent."), *this->GetParentFixtureActor()->GetFName().ToString(), *StaticEnum<EDMXImportGDTFPhysicalUnit>()->GetNameStringByValue((uint8)SubPhysical.PhysicalUnit));
			}
			else DutyCycle = (SubPhysical.PhysicalFrom + SubPhysical.PhysicalTo) / 200;
			break;
		case EDMXImportGDTFSubPhysicalUnitType::TimeOffset:
			if (SubPhysical.PhysicalUnit != EDMXImportGDTFPhysicalUnit::Percent) {
				UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Fixture %s, On Shutter Pulse effect TimeOffset value unit '%s' is unsupported. Value ignored, please provide percent."), *this->GetParentFixtureActor()->GetFName().ToString(), *StaticEnum<EDMXImportGDTFPhysicalUnit>()->GetNameStringByValue((uint8)SubPhysical.PhysicalUnit));
			}
			else TimeOffset = (SubPhysical.PhysicalFrom + SubPhysical.PhysicalTo) / 200;
			break;
		default:
			break;
		}
	}
	this->PulseManager.SetSettings(PulseEffectType, Period, DutyCycle, TimeOffset);
}

/**
 * Setup the object for given parameters
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 22 july 2022
 *
 * @param DMXValue
 * @param ChannelFunction
 * @param ChannelSet
*/
void UCPGDTFShutterFixtureComponent::StartRandomEffect(int32 DMXValue, FCPGDTFDescriptionChannelFunction* ChannelFunction, FCPGDTFDescriptionChannelSet* ChannelSet) {
	if (ChannelSet->PhysicalFrom == 0 && ChannelSet->PhysicalTo == 1) { // If the ChannelSet values are the default one we use the ChannelFunction's ones
		if (ChannelFunction->PhysicalFrom == 0 && ChannelFunction->PhysicalTo == 1) { // If the ChannelFunction values are the default one we fallback to genereic default ones
			this->RandomPhysicalFrom = 0.2;
			this->RandomPhysicalTo = 5;
		}
		else {
			this->RandomPhysicalFrom = ChannelFunction->PhysicalFrom;
			this->RandomPhysicalTo = ChannelFunction->PhysicalTo;
		}
	}
	else {
		this->RandomPhysicalFrom = ChannelSet->PhysicalFrom;
		this->RandomPhysicalTo = ChannelSet->PhysicalTo;
	}
	this->RandomPhysicalTo = UKismetMathLibrary::MapRangeClamped(DMXValue, ChannelSet->DMXFrom.Value, ChannelSet->DMXTo.Value, this->RandomPhysicalFrom, this->RandomPhysicalTo);
	this->RandomCurrentFrequency = FMath::FRandRange(this->RandomPhysicalFrom, this->RandomPhysicalTo);
}