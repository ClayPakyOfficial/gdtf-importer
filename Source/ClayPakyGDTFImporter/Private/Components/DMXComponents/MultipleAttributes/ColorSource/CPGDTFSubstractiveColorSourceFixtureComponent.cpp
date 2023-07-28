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


#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFSubstractiveColorSourceFixtureComponent.h"
#include "Utils/CPGDTFColorWizard.h"
#include "CPGDTFFixtureActor.h"

UCPGDTFSubstractiveColorSourceFixtureComponent::UCPGDTFSubstractiveColorSourceFixtureComponent() {
	this->bUseInterpolation = false; // No interpolation here
};

bool UCPGDTFSubstractiveColorSourceFixtureComponent::Setup(TArray<FDMXImportGDTFDMXChannel> InputsAvailables, int attributeIndex) {

	Super::Setup(InputsAvailables, attributeIndex);
	
	for (FDMXImportGDTFDMXChannel Channel : InputsAvailables) {
		
		ECPGDTFAttributeType AttrType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(Channel.LogicalChannels[0].Attribute.Name.ToString());

		switch (AttrType) {

		case ECPGDTFAttributeType::ColorSub_R:
			this->DMXChannelRed = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorSub_G:
			this->DMXChannelGreen = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorSub_B:
			this->DMXChannelBlue = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorSub_C:
			this->DMXChannelCyan = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorSub_M:
			this->DMXChannelMagenta = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorSub_Y:
			this->DMXChannelYellow = FCPDMXColorChannelData(Channel, AttrType);
			break;
		default:
			UE_LOG_CPGDTFIMPORTER(Warning, TEXT("%s attribute is not supported by the CPGDTFColorSourceFixtureComponent for now."), *Channel.LogicalChannels[0].Attribute.Name.ToString());
			break;
		}
	}
	return true;
}

void UCPGDTFSubstractiveColorSourceFixtureComponent::PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) {

	FLinearColor FilterColor = FLinearColor(0, 0, 0); // The filter color is the complementary one of the rendered one

	// We use pointers here to reduce memory consumption
	TArray<FCPDMXColorChannelData*> DMXChannels = { &DMXChannelRed, &DMXChannelGreen, &DMXChannelBlue,
		&DMXChannelCyan, &DMXChannelMagenta, &DMXChannelYellow};

	for (FCPDMXColorChannelData* DMXChannel : DMXChannels) {

		if (DMXChannel->IsAddressValid()) {
			const float* TargetValuePtr = RawValuesMap.Map.Find(DMXChannel->address);
			if (TargetValuePtr) {
				const float RemappedValue = FMath::Max(0.0f, FMath::Min(1.0f, *TargetValuePtr));

				switch (DMXChannel->ColorAttribute) {

				case ECPGDTFAttributeType::ColorSub_R:
					FilterColor += RemappedValue * FLinearColor(0, 1, 1);
					break;
				case ECPGDTFAttributeType::ColorSub_G:
					FilterColor += RemappedValue * FLinearColor(1, 0, 1);
					break;
				case ECPGDTFAttributeType::ColorSub_B:
					FilterColor += RemappedValue * FLinearColor(1, 1, 0);
					break;
				case ECPGDTFAttributeType::ColorSub_C:
					FilterColor += RemappedValue * FLinearColor(1, 0, 0);
					break;
				case ECPGDTFAttributeType::ColorSub_M:
					FilterColor += RemappedValue * FLinearColor(0, 1, 0);
					break;
				case ECPGDTFAttributeType::ColorSub_Y:
					FilterColor += RemappedValue * FLinearColor(0, 0, 1);
					break;
				default:
					break;
				}
			}
		}
	}

	this->CurrentColor = FilterColor;
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFSubstractiveColorSourceFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> ret;
	return ret;
}

float UCPGDTFSubstractiveColorSourceFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.0958 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFSubstractiveColorSourceFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.0292 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFSubstractiveColorSourceFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 1.2857 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFSubstractiveColorSourceFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.3043 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}