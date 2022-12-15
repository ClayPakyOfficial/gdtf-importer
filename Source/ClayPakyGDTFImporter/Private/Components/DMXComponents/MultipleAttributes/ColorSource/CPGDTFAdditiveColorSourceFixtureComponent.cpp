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


#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFAdditiveColorSourceFixtureComponent.h"
#include "Utils/CPGDTFColorWizard.h"
#include "CPGDTFFixtureActor.h"

UCPGDTFAdditiveColorSourceFixtureComponent::UCPGDTFAdditiveColorSourceFixtureComponent() {
	this->bUseInterpolation = false; // No interpolation here
};

void UCPGDTFAdditiveColorSourceFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables) {

	Super::Setup(AttachedGeometryNamee, InputsAvailables);
	
	for (FDMXImportGDTFDMXChannel Channel : InputsAvailables) {
		
		ECPGDTFAttributeType AttrType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(Channel.LogicalChannels[0].Attribute.Name.ToString());

		switch (AttrType) {

		case ECPGDTFAttributeType::ColorAdd_R:
			this->DMXChannelRed = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_G:
			this->DMXChannelGreen = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_B:
			this->DMXChannelBlue = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_W:
			this->DMXChannelWhite = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_C:
			this->DMXChannelCyan = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_M:
			this->DMXChannelMagenta = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_Y:
			this->DMXChannelYellow = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_RY:
			this->DMXChannelAmber = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_GY:
			this->DMXChannelLime = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_GC:
			this->DMXChannelBlueGreen = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_BC:
			this->DMXChannelLightBlue = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_BM:
			this->DMXChannelPurple = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_RM:
			this->DMXChannelPink = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_WW:
			this->DMXChannelWarmWhite = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_CW:
			this->DMXChannelCoolWhite = FCPDMXColorChannelData(Channel, AttrType);
			break;
		case ECPGDTFAttributeType::ColorAdd_UV:
			this->DMXChannelUV = FCPDMXColorChannelData(Channel, AttrType);
			break;
		default:
			UE_LOG_CPGDTFIMPORTER(Warning, TEXT("%s attribute is not supported by the CPGDTFColorSourceFixtureComponent for now."), *Channel.LogicalChannels[0].Attribute.Name.ToString());
			break;
		}
	}
}

void UCPGDTFAdditiveColorSourceFixtureComponent::PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) {

	FCPColorWizard ColorWizard = FCPColorWizard();

	// We use pointers here to reduce memory consumption
	TArray<FCPDMXColorChannelData*> DMXChannels = { &DMXChannelRed, &DMXChannelGreen, &DMXChannelBlue, &DMXChannelWhite,
		&DMXChannelCyan, &DMXChannelMagenta, &DMXChannelYellow, &DMXChannelAmber,
		&DMXChannelLime, &DMXChannelBlueGreen, &DMXChannelLightBlue, &DMXChannelPurple,
		&DMXChannelPink, &DMXChannelWarmWhite, &DMXChannelCoolWhite, &DMXChannelUV};

	for (FCPDMXColorChannelData* DMXChannel : DMXChannels) {

		if (DMXChannel->IsAddressValid()) {
			const float* TargetValuePtr = RawValuesMap.Map.Find(DMXChannel->Address);
			if (TargetValuePtr) {
				const float RemappedValue = FMath::Max(0.0f, FMath::Min(1.0f, *TargetValuePtr));

				switch (DMXChannel->ColorAttribute) {

				case ECPGDTFAttributeType::ColorAdd_R:
					ColorWizard.BlendRed(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_G:
					ColorWizard.BlendGreen(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_B:
					ColorWizard.BlendBlue(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_W:
					ColorWizard.BlendWhite(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_C:
					ColorWizard.BlendCyan(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_M:
					ColorWizard.BlendMagenta(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_Y:
					ColorWizard.BlendYellow(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_RY:
					ColorWizard.BlendAmber(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_GY:
					ColorWizard.BlendLime(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_GC:
					ColorWizard.BlendBlueGreen(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_BC:
					ColorWizard.BlendLightBlue(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_BM:
					ColorWizard.BlendPurple(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_RM:
					ColorWizard.BlendPink(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_WW:
					ColorWizard.BlendWarmWhite(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_CW:
					ColorWizard.BlendCoolWhite(RemappedValue);
					break;
				case ECPGDTFAttributeType::ColorAdd_UV:
					ColorWizard.BlendUV(RemappedValue);
					break;
				default:
					break;
				}
			}
		}
	}

	this->CurrentColor = ColorWizard.GetColor();
}