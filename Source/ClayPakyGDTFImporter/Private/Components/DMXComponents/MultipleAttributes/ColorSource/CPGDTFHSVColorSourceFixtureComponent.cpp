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


#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFHSVColorSourceFixtureComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/CPGDTFColorWizard.h"
#include "CPGDTFFixtureActor.h"

UCPGDTFHSVColorSourceFixtureComponent::UCPGDTFHSVColorSourceFixtureComponent() {
	this->bUseInterpolation = false; // No interpolation here
};

void UCPGDTFHSVColorSourceFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables) {

	Super::Setup(AttachedGeometryNamee, InputsAvailables);
	
	for (FDMXImportGDTFDMXChannel Channel : InputsAvailables) {
		
		ECPGDTFAttributeType AttrType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(Channel.LogicalChannels[0].Attribute.Name.ToString());

		switch (AttrType) {

		case ECPGDTFAttributeType::HSB_Hue:
			this->DMXChannelH = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::HSB_Saturation:
			this->DMXChannelS= FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::HSB_Brightness:
			this->DMXChannelV= FCPDMXColorChannelData(Channel, AttrType);
			break;
		default:
			UE_LOG_CPGDTFIMPORTER(Warning, TEXT("%s attribute is not supported by the CPGDTFColorSourceFixtureComponent for now."), *Channel.LogicalChannels[0].Attribute.Name.ToString());
			break;
		}
	}
}

void UCPGDTFHSVColorSourceFixtureComponent::PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) {

	// We use pointers here to reduce memory consumption
	TArray<FCPDMXColorChannelData*> DMXChannels = { &DMXChannelH, &DMXChannelS, &DMXChannelV};
	float H = 0, S = 0, V = 0;

	for (FCPDMXColorChannelData* DMXChannel : DMXChannels) {

		if (DMXChannel->IsAddressValid()) {
			const float* TargetValuePtr = RawValuesMap.Map.Find(DMXChannel->Address);
			if (TargetValuePtr) {
				const float RemappedValue = FMath::Lerp(DMXChannel->MinValue, DMXChannel->MaxValue, *TargetValuePtr);

				switch (DMXChannel->ColorAttribute) {

				case ECPGDTFAttributeType::HSB_Hue:
					H = UKismetMathLibrary::MapRangeClamped(RemappedValue, DMXChannel->MinValue, DMXChannel->MaxValue, 0.0f, 1.0f);
					break;
				case ECPGDTFAttributeType::HSB_Saturation:
					S = UKismetMathLibrary::MapRangeClamped(RemappedValue, DMXChannel->MinValue, DMXChannel->MaxValue, 0.0f, 1.0f);
					break;
				case ECPGDTFAttributeType::HSB_Brightness:
					V = UKismetMathLibrary::MapRangeClamped(RemappedValue, DMXChannel->MinValue, DMXChannel->MaxValue, 0.0f, 100.0f);
					break;
				default:
					break;
				}
			}
		}
	}

	this->CurrentColor = FCPColorWizard::ColorHSVToRGB(H, S, V);
}