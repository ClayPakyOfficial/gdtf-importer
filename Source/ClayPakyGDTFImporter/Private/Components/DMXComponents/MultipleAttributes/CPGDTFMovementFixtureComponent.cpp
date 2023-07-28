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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFMovementFixtureComponent.h"

bool UCPGDTFMovementFixtureComponent::Setup(TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(DMXChannels, attributeIndex);
	this->bUseInterpolation = true;
	return true;
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFMovementFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> attributes;

	TSet<ECPGDTFAttributeType> movP, movT;
	movP.Add(ECPGDTFAttributeType::Pan);
	movT.Add(ECPGDTFAttributeType::Tilt);
	attributes.Add(movP);
	attributes.Add(movT);

	TSet<ECPGDTFAttributeType> infP, infT;
	infP.Add(ECPGDTFAttributeType::PanRotate);
	infT.Add(ECPGDTFAttributeType::TiltRotate);
	attributes.Add(infP);
	attributes.Add(infT);

	return attributes;
}
float UCPGDTFMovementFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	return (ECPGDTFMovementFixtureType) interpolationId == ECPGDTFMovementFixtureType::Pan ? 1.00 : 1.00;
}
float UCPGDTFMovementFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	return (ECPGDTFMovementFixtureType) interpolationId == ECPGDTFMovementFixtureType::Pan ? 1.00 : 1.00;
}

void UCPGDTFMovementFixtureComponent::BeginPlay() {
	geometryP = *this->AttachedGeometries.Find(ECPGDTFAttributeType::Pan);
	geometryT = *this->AttachedGeometries.Find(ECPGDTFAttributeType::Tilt);
	FCPDMXChannelData chDataP = *this->attributesData.getChannelData(ECPGDTFAttributeType::Pan);
	FCPDMXChannelData chDataT = *this->attributesData.getChannelData(ECPGDTFAttributeType::Tilt);
	TArray<FCPDMXChannelData> data;
	data.Add(chDataP);
	data.Add(chDataT);
	Super::BeginPlay(data);
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->bUseInterpolation = true;
	minValP = chDataP.MinValue;
	minValT = chDataT.MinValue;
}

void UCPGDTFMovementFixtureComponent::SetTargetValue(float value, int interpolationId) {
	// We invert the rotation
	if ((ECPGDTFMovementFixtureType) interpolationId == ECPGDTFMovementFixtureType::Pan) {
		if (this->bInvertPan) value = this->maxValP - value + this->minValP;
	} else if ((ECPGDTFMovementFixtureType) interpolationId == ECPGDTFMovementFixtureType::Tilt) {
		if (this->bInvertTilt) value = this->maxValT - value + this->minValP;
	}
	Super::SetTargetValue(value, interpolationId);
}

void UCPGDTFMovementFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	switch (AttributeType) {

		case ECPGDTFAttributeType::Pan: {
			valueP = physicalValue;
			if (!infinitePenabled)
				this->SetTargetValue(physicalValue, (int)ECPGDTFMovementFixtureType::Pan);
			break;
		}
		case ECPGDTFAttributeType::Tilt: {
			valueT = physicalValue;
			if (!infiniteTenabled)
				this->SetTargetValue(physicalValue, (int)ECPGDTFMovementFixtureType::Tilt);
			break;
		}
		
		default:
		case ECPGDTFAttributeType::TiltRotate:
		case ECPGDTFAttributeType::PanRotate: {
			bool isPan = AttributeType == ECPGDTFAttributeType::PanRotate;
			bool shouldDisable = !isPan && AttributeType != ECPGDTFAttributeType::TiltRotate;
			if (shouldDisable) {
				//If shouldDisable = true it means we're in the default cause, so it means that this channel that was previously controlling OR PanRotate OR TiltRotate has changed to another kind of attribute.
				//We need to check what we were previously and select the correct variables according to that
				bool found = false;
				if (channel.RunningEffectTypeChannel == ECPGDTFAttributeType::PanRotate ) { isPan = true ; found = true; }
				if (channel.RunningEffectTypeChannel == ECPGDTFAttributeType::TiltRotate) { isPan = false; found = true; }
				if (!found) break; //If in the previous dmx packet we were not pan/tilt/panrotate/tilt rotate, we just fall as default case
			}
			bool* infEnabled = isPan ? &infinitePenabled : &infiniteTenabled;
			FPulseEffectManager* pulseManager = isPan ? &this->PulseManagerP : &this->PulseManagerT;
			int type = isPan ? (int)ECPGDTFMovementFixtureType::Pan : (int)ECPGDTFMovementFixtureType::Tilt;
			float value = isPan ? valueP : valueT;
			float* dir = isPan ? &directionP : &directionT;

			FCPGDTFDescriptionChannelSet* cs = DMXBehaviour.Value;
			//Some fixtures (EG minixtylos) use physicalFrom and physicalTo both equals to 0 when the continuous rotation is off
			bool enabled = (cs->PhysicalFrom != 0 || cs->PhysicalTo != 0) && !shouldDisable;
			if (enabled) {
				*dir = (physicalValue > 0) * 2 - 1;
				physicalValue = abs(physicalValue);
				float period = 360 / physicalValue;
				if (!*infEnabled) {
					pulseManager->SetSettings(EPulseEffectType::PulseOpen, period, 1, 1);
				} else {
					pulseManager->ChangePeriod(period);
				}
			} else {
				if (*infEnabled) {
					//The following line makes the light ALWAYS end its previous rotation until it reaches Value.
					//Disabling it will make the light ALWAYS stop and come back to Value.
					//After I, Luca Sorace, talked to some folks in the R&D department of Clay Paky, I ended up
					//choosing to always end the previous rotation without changing the movement direction, since it looks better.
					//Some fixtures have a switch to turn this feature (called shortcuts) on or off, but since it's not defined in GDTF I had
					//to arbitrary choose what I should do globally. I ended up choosing always this path without first checking if the light is in the first
					//180 degree of the rotation, so you will always get the same "deterministic" effect when you both stop the continuous rotation too early or too late
					this->interpolations[type].offset(-360 * (*dir));
					this->SetTargetValue(value, type);
					*dir = 0;
				}
			}
			*infEnabled = enabled;
			break;
		}
	}
}

void UCPGDTFMovementFixtureComponent::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {
	ECPGDTFMovementFixtureType movType;
	FPulseEffectManager *pulseManager;
	float dir, value;
	switch (channel.RunningEffectTypeChannel) {
		case ECPGDTFAttributeType::PanRotate: {
			if (!infinitePenabled) return;
			movType = ECPGDTFMovementFixtureType::Pan;
			pulseManager = &this->PulseManagerP;
			dir = directionP;
			value = valueP;
			break;
		}
		case ECPGDTFAttributeType::TiltRotate: {
			movType = ECPGDTFMovementFixtureType::Tilt;
			if (!infiniteTenabled) return;
			pulseManager = &this->PulseManagerT;
			dir = directionT;
			value = valueT;
			break;
		}
		default: return;
	}
	int type = (int)movType;

	float val = pulseManager->InterpolatePulse(deltaSeconds);
	bool loopback = pulseManager->hasLoopedBack();
	//If we have looped, let's offset our value back of 360 degrees
	if (loopback) this->interpolations[type].offset(-360 * dir);
	//We set the target value to a relative position from our "normal" value specified from the plain Pan/Tilt attribute
	this->SetTargetValue(value + val * dir * 360, type);
}

void UCPGDTFMovementFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float value, int interpolationId) {
	switch ((ECPGDTFMovementFixtureType) interpolationId) {
		case ECPGDTFMovementFixtureType::Pan: {
			FRotator CurrentRotation = this->geometryP->GetRelativeRotation();
			this->geometryP->SetRelativeRotation(FRotator(CurrentRotation.Pitch, value, CurrentRotation.Roll));
			break;
		}
		case ECPGDTFMovementFixtureType::Tilt: {
			FRotator CurrentRotation = this->geometryT->GetRelativeRotation();
			this->geometryT->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, value));
			break;
		}
	}
}