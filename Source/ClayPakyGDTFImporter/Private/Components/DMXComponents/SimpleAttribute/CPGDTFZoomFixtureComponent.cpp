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


#include "Components/DMXComponents/SimpleAttribute/CPGDTFZoomFixtureComponent.h"

bool UCPGDTFZoomFixtureComponent::Setup(FDMXImportGDTFDMXChannel DMXChannell, int attributeIndex) {
	mMainAttributes.Add(ECPGDTFAttributeType::Zoom);
	mMainAttributes.Add(ECPGDTFAttributeType::ZoomModeBeam);
	mMainAttributes.Add(ECPGDTFAttributeType::ZoomModeSpot);
	Super::Setup(DMXChannell, attributeIndex);
	this->bUseInterpolation = true;
	return true;
}

void UCPGDTFZoomFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Angle, int interpolationId) {

	if (Beam == nullptr) return;

	float HalfAngle = Angle / 2.0f; // / 2.0 because the angle in Unreal is between the center and the border of the beam
	
	setAllScalarParameters(Beam, "DMX Zoom", HalfAngle);
	if (Beam->SpotLightR) {
		Beam->SpotLightR->SetOuterConeAngle(HalfAngle);
		Beam->SpotLightR->SetInnerConeAngle(HalfAngle * 0.85f); /// TODO \todo Find something on GDTF to be more accurate
	}
	if (Beam->SpotLightG) {
		Beam->SpotLightG->SetOuterConeAngle(HalfAngle);
		Beam->SpotLightG->SetInnerConeAngle(HalfAngle * 0.85f); /// TODO \todo Find something on GDTF to be more accurate
	}
	if (Beam->SpotLightB) {
		Beam->SpotLightB->SetOuterConeAngle(HalfAngle);
		Beam->SpotLightB->SetInnerConeAngle(HalfAngle * 0.85f); /// TODO \todo Find something on GDTF to be more accurate
	}
}

float UCPGDTFZoomFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.4292 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFZoomFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.0667 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFZoomFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 4.4375 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFZoomFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.1553 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}