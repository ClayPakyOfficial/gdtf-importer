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

#include "Components/DMXComponents/CPGDTFSimpleAttributeFixtureComponent.h"

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFSimpleAttributeFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> mainArr;
	mainArr.Add(mMainAttributes);
	return mainArr;
}

bool UCPGDTFSimpleAttributeFixtureComponent::Setup(FDMXImportGDTFDMXChannel DMXChannell, int attributeIndex) {
	TArray<FDMXImportGDTFDMXChannel> chs;
	chs.Add(DMXChannell);
	Super::Setup(DMXChannell.Geometry, chs, attributeIndex);
	bIsRawDMXEnabled = true;
	return true;
}

void UCPGDTFSimpleAttributeFixtureComponent::BeginPlay() {
	Super::BeginPlay(mMainAttributes);
	bIsRawDMXEnabled = true;
}


/*******************************************/
/*               DMX Related               */
/*******************************************/

void UCPGDTFSimpleAttributeFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	if(mMainAttributes.Contains(AttributeType))
		this->SetTargetValue(physicalValue);
}

  /*******************************************/
 /*          Interpolation Related          */
/*******************************************/

void UCPGDTFSimpleAttributeFixtureComponent::SetTargetValue(float AbsoluteValue) {
	if (IsTargetValid(AbsoluteValue)) Super::SetTargetValue(AbsoluteValue, 0);
}

bool UCPGDTFSimpleAttributeFixtureComponent::IsTargetValid(float Target) {
	return this->HasBegunPlay() ? this->interpolations[0].IsTargetValid(Target, SkipThreshold) : false;
}