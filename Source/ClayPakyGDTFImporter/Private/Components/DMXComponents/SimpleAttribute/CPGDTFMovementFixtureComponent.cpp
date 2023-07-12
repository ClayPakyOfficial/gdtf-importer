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


#include "Components/DMXComponents/SimpleAttribute/CPGDTFMovementFixtureComponent.h"

// Initializes the component on spawn on a world
void UCPGDTFMovementFixtureComponent::OnConstruction() {
	Super::OnConstruction();

	USceneComponent** AttachedFixturePartPtr = this->GetParentFixtureActor()->GeometryTree.Components.Find(this->AttachedGeometryName);
	if (AttachedFixturePartPtr == nullptr) return;
	this->AttachedGeometry = *AttachedFixturePartPtr;
}

bool UCPGDTFMovementFixtureComponent::Setup(FDMXImportGDTFDMXChannel DMXChannell, int attributeIndex) {
	ECPGDTFAttributeType DMXChannelAttributeType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(DMXChannell.LogicalChannels[0].Attribute.Name.ToString());
	this->MovementType = DMXChannelAttributeType == ECPGDTFAttributeType::Pan ? ECPGDTFMovementFixtureType::Pan : ECPGDTFMovementFixtureType::Tilt;
	mMainAttributes.Add(DMXChannelAttributeType);
	Super::Setup(DMXChannell, attributeIndex);
	this->bUseInterpolation = true;
	return true;
}

void UCPGDTFMovementFixtureComponent::BeginPlay() {
	Super::BeginPlay();
	this->mChannelData = *attributesData.getChannelData(this->MovementType == ECPGDTFMovementFixtureType::Pan ? ECPGDTFAttributeType::Pan : ECPGDTFAttributeType::Tilt);
	this->bUseInterpolation = true;
}

void UCPGDTFMovementFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* beam, float value, int interpolationId) {
	FRotator CurrentRotation = this->AttachedGeometry->GetRelativeRotation();
	switch (this->MovementType) {
		case ECPGDTFMovementFixtureType::Pan:
			this->AttachedGeometry->SetRelativeRotation(FRotator(CurrentRotation.Pitch, value, CurrentRotation.Roll));
			break;
		case ECPGDTFMovementFixtureType::Tilt:
			this->AttachedGeometry->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, value));
			break;
	}
}

  /*******************************************/
 /*           Component Specific            */
/*******************************************/

double UCPGDTFMovementFixtureComponent::GetRotation() {

	if (this->AttachedGeometry == nullptr) return 0;
	switch (this->MovementType) {
	case ECPGDTFMovementFixtureType::Pan:
		return this->AttachedGeometry->GetRelativeRotation().Yaw;
	case ECPGDTFMovementFixtureType::Tilt:
		return this->AttachedGeometry->GetRelativeRotation().Roll;
	}
	return 0; // Impossible but here to avoid compilation error
}

  /*******************************************/
 /*          Interpolation Related          */
/*******************************************/

void UCPGDTFMovementFixtureComponent::SetTargetValue(float AbsoluteValue) {
	// We invert the rotation
	if (this->bInvertRotation) AbsoluteValue = this->mChannelData.MaxValue - AbsoluteValue + this->mChannelData.MinValue;
	Super::SetTargetValue(AbsoluteValue);
}

float UCPGDTFMovementFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	return MovementType == ECPGDTFMovementFixtureType::Pan ? 1.00 : 1.00;
}
float UCPGDTFMovementFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	return MovementType == ECPGDTFMovementFixtureType::Pan ? 1.00 : 1.00;
}
float UCPGDTFMovementFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	return MovementType == ECPGDTFMovementFixtureType::Pan ? 1.00 : 1.00;
}
float UCPGDTFMovementFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	return MovementType == ECPGDTFMovementFixtureType::Pan ? 1.00 : 1.00;
}