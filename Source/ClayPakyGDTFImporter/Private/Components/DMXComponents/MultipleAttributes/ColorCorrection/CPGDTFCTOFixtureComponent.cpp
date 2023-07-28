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


#include "Components/DMXComponents/MultipleAttributes/ColorCorrection/CPGDTFCTOFixtureComponent.h"
#include "Kismet/KismetMathLibrary.h"

bool UCPGDTFCTOFixtureComponent::Setup(TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(DMXChannels, attributeIndex);
	this->bIsRawDMXEnabled = true;
	return true;
}

void UCPGDTFCTOFixtureComponent::BeginPlay() {
	TArray<FCPDMXChannelData> data;
	FCPDMXChannelData cd;
	data.Add(cd);
	Super::BeginPlay(data);
	interpolations[0].bInterpolationEnabled = false;
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->IsCTOEnabled = false;
}

void UCPGDTFCTOFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	if (AttributeType == ECPGDTFAttributeType::CTO) {
		if (DMXBehaviour.Value->DMXFrom.Value == 0) { // Disable CTO
			this->IsCTOEnabled = false;
			this->SetValueNoInterp(-1, 0);
			return;
		}

		this->IsCTOEnabled = true;
		this->ColorTemperature = physicalValue;
		this->SetValueNoInterp(physicalValue, 0);
	}
}

void UCPGDTFCTOFixtureComponent::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {
	if (this->IsCTOEnabled) this->SetValueNoInterp(this->ColorTemperature, 0);
}

void UCPGDTFCTOFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* beam, float value, int interpolationId) {
	if (this->IsCTOEnabled) beam->SetLightColorTemp(value);
	else if (value == -1) beam->ResetLightColorTemp();
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFCTOFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> ret;
	return ret;
}
float UCPGDTFCTOFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.1000 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFCTOFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.0229 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFCTOFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 2.3636 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFCTOFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.2292 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}