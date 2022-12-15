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


#include "Components/DMXComponents/CPGDTFSimpleAttributeFixtureComponent.h"

void UCPGDTFSimpleAttributeFixtureComponent::BeginPlay() {
	
	Super::BeginPlay();

	this->CurrentValue = FChannelInterpolation();

	// Init interpolation speed scale
	this->CurrentValue.InterpolationScale = this->InterpolationScale;

	// Init interpolation range 
	this->CurrentValue.RangeSize = FMath::Abs(this->DMXChannel.MaxValue - this->DMXChannel.MinValue);

	// Set the default value as the target and the current
	this->CurrentValue.CurrentValue = this->DMXChannel.DefaultValue;
	this->CurrentValue.TargetValue = this->DMXChannel.DefaultValue;

	this->CurrentValue.StartTravel(this->DMXChannel.DefaultValue);

	SetValueNoInterp(this->DMXChannel.DefaultValue);
}

void UCPGDTFSimpleAttributeFixtureComponent::Setup(FDMXImportGDTFDMXChannel DMXChannell) {

	Super::Setup(DMXChannell.Geometry);
	this->DMXChannel = FCPDMXChannelData(DMXChannell);
}

  /*******************************************/
 /*               DMX Related               */
/*******************************************/

void UCPGDTFSimpleAttributeFixtureComponent::PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) {

	const float* TargetValuePtr = RawValuesMap.Map.Find(this->DMXChannel.Address);
	if (TargetValuePtr) {
		const float RemappedValue = this->NormalizedToAbsoluteValue(*TargetValuePtr);
		this->SetTargetValue(RemappedValue);
	}
}

  /*******************************************/
 /*          Interpolation Related          */
/*******************************************/

void UCPGDTFSimpleAttributeFixtureComponent::InterpolateComponent(float DeltaSeconds) {
	
	if (this->CurrentValue.IsUpdating) {
		this->CurrentValue.Travel(DeltaSeconds); // Update
		if (this->CurrentValue.IsInterpolationDone()) this->CurrentValue.EndInterpolation(); // Is done?
		this->SetValueNoInterp(this->CurrentValue.CurrentValue);
	}
}

float UCPGDTFSimpleAttributeFixtureComponent::GetDMXInterpolatedStep() const {

	if (this->HasBegunPlay()) return (this->CurrentValue.CurrentSpeed * this->CurrentValue.Direction);
	else return 0.f;
}

float UCPGDTFSimpleAttributeFixtureComponent::GetDMXInterpolatedValue() const {

	if (this->HasBegunPlay()) return this->CurrentValue.CurrentValue;
	else return 0.f;
}

float UCPGDTFSimpleAttributeFixtureComponent::GetDMXTargetValue() const {

	if (this->HasBegunPlay()) return this->CurrentValue.TargetValue;
	else return 0.f;
}

bool UCPGDTFSimpleAttributeFixtureComponent::IsDMXInterpolationDone() const {

	if (this->HasBegunPlay()) return this->CurrentValue.IsInterpolationDone();
	else return true;
}

void UCPGDTFSimpleAttributeFixtureComponent::SetTargetValue(float AbsoluteValue) {

	if (this->HasBegunPlay() && IsTargetValid(AbsoluteValue)) {
		if (this->bUseInterpolation) {
			if (this->CurrentValue.bFirstValueWasSet)
				// Only 'Push' the next value into interpolation. BPs will read the resulting value on tick.
				this->CurrentValue.Push(AbsoluteValue);
			else {
				// Jump to the first value if it never was set
				this->CurrentValue.SetValueNoInterp(AbsoluteValue);
				this->CurrentValue.bFirstValueWasSet = true;

				SetValueNoInterp(AbsoluteValue);
			}
		} else {
			this->CurrentValue.SetValueNoInterp(AbsoluteValue);
			SetValueNoInterp(AbsoluteValue);
		}
	}
}

void UCPGDTFSimpleAttributeFixtureComponent::ApplySpeedScale() {

	this->CurrentValue.InterpolationScale = InterpolationScale;
}

float UCPGDTFSimpleAttributeFixtureComponent::NormalizedToAbsoluteValue(float Alpha) const {

	if (this->HasBegunPlay()) {

		const float AbsoluteValue = FMath::Lerp(DMXChannel.MinValue, DMXChannel.MaxValue, Alpha);
		return AbsoluteValue;
	} else return 0.f;
}

bool UCPGDTFSimpleAttributeFixtureComponent::IsTargetValid(float Target) {

	if (this->HasBegunPlay()) return this->CurrentValue.IsTargetValid(Target, SkipThreshold);
	else return false;
}