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


#include "Components/DMXComponents/MultipleAttributes/CPGDTFShaperFixtureComponent.h"

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFShaperFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> attributes;

	TSet<ECPGDTFAttributeType> shapeA;
	shapeA.Add(ECPGDTFAttributeType::Blade_n_A);
	shapeA.Add(ECPGDTFAttributeType::BladeSoft_n_A);
	attributes.Add(shapeA);

	TSet<ECPGDTFAttributeType> shapeBRot;
	shapeBRot.Add(ECPGDTFAttributeType::Blade_n_Rot);
	shapeBRot.Add(ECPGDTFAttributeType::Blade_n_B);
	shapeBRot.Add(ECPGDTFAttributeType::BladeSoft_n_B);
	attributes.Add(shapeBRot);

	TSet<ECPGDTFAttributeType> shaperRot;
	shaperRot.Add(ECPGDTFAttributeType::ShaperRot);
	attributes.Add(shaperRot);

	TSet<ECPGDTFAttributeType> shaperMacro;
	shaperMacro.Add(ECPGDTFAttributeType::ShaperMacros);
	attributes.Add(shaperMacro);

	TSet<ECPGDTFAttributeType> shaperMacroSpeed;
	shaperMacroSpeed.Add(ECPGDTFAttributeType::ShaperMacrosSpeed);
	attributes.Add(shaperMacroSpeed);

	return attributes;
}

float UCPGDTFShaperFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[3] = { 0.6521, 0.6521, 1.0854 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFShaperFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[3] = { 0.1854, 0.1854, 0.1917 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}

bool UCPGDTFShaperFixtureComponent::Setup(TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(DMXChannels, attributeIndex);
	this->orientation = attributeIndex;

	//Setup method could be called without any blade attribute (so with only shaperRot/shperMacro/shaperMacroSpeed). We have to check if we really have a blade and return an error otherwise
	bool foundBladeAttribute = false;
	for (int k = 0; k < DMXChannels.Num(); k++) {
		FDMXImportGDTFDMXChannel ch = DMXChannels[k];
		for (int i = 0; i < ch.LogicalChannels.Num(); i++) {
			for (int j = 0; j < ch.LogicalChannels[i].ChannelFunctions.Num(); j++) {
				FDMXImportGDTFChannelFunction cf = ch.LogicalChannels[i].ChannelFunctions[j];
				ECPGDTFAttributeType attrType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(cf.Attribute.Name.ToString());

				if (attrType == ECPGDTFAttributeType::Blade_n_Rot) {
					abMode = false;
				} else if (attrType == ECPGDTFAttributeType::Blade_n_B || attrType == ECPGDTFAttributeType::BladeSoft_n_B) {
					abMode = true;
				}

				if (attrType == ECPGDTFAttributeType::Blade_n_Rot ||
					attrType == ECPGDTFAttributeType::Blade_n_B ||
					attrType == ECPGDTFAttributeType::BladeSoft_n_B ||
					attrType == ECPGDTFAttributeType::Blade_n_A ||
					attrType == ECPGDTFAttributeType::BladeSoft_n_A
					) foundBladeAttribute = true;
			}
		}
	}

	return foundBladeAttribute;
}

void UCPGDTFShaperFixtureComponent::BeginPlay() {
	this->mBladeAParamName = *CPGDTFRenderPipelineBuilder::getBladeABRotParamName(true, this->mAttributeIndexNo);
	this->mBladeBRotParamName = *CPGDTFRenderPipelineBuilder::getBladeABRotParamName(false, this->mAttributeIndexNo);
	this->mBladeOrientationName = *CPGDTFRenderPipelineBuilder::getBladeOrientationParamName(this->mAttributeIndexNo);

	TArray<FCPDMXChannelData> data;
	data.Add(*this->attributesData.getChannelData(ECPGDTFAttributeType::Blade_n_A)); //ORDER METTERS!
	data.Add(*this->attributesData.getChannelData(ECPGDTFAttributeType::Blade_n_B)); //It has to match the UCPGDTFShaperFixtureComponent::InterpolationIds enum
	data.Add(*this->attributesData.getChannelData(ECPGDTFAttributeType::ShaperRot)); //It has to match the UCPGDTFShaperFixtureComponent::InterpolationIds enum
	Super::BeginPlay(data);

	this->bIsRawDMXEnabled = true; // Just to make sure
	this->bUseInterpolation = true;
}

/**
 * Read DMXValue and apply the effect to the Beam
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 31 january 2022
 *
 * @param DMXValue
 */
void UCPGDTFShaperFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	switch (AttributeType) {

		case ECPGDTFAttributeType::BladeSoft_n_A:
		case ECPGDTFAttributeType::Blade_n_A: {
			this->SetTargetValue(physicalValue, InterpolationIds::BLADE_A);
			break;
		}
		case ECPGDTFAttributeType::BladeSoft_n_B:
		case ECPGDTFAttributeType::Blade_n_Rot:
		case ECPGDTFAttributeType::Blade_n_B: {
			this->SetTargetValue(physicalValue, InterpolationIds::BLADE_BROT);
			break;
		}
		case ECPGDTFAttributeType::ShaperRot: {
			this->SetTargetValue(physicalValue, InterpolationIds::SHAPER_ROT);
			break;
		}
		default: break; // Not supported behaviour
	}
}

void UCPGDTFShaperFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float value, int interpolationId) {
	InterpolationIds iid = (InterpolationIds)interpolationId;
	switch (iid) {
		case InterpolationIds::BLADE_A: {
			this->setAllScalarParameters(Beam, this->mBladeAParamName, value);
			break;
		}
		case InterpolationIds::BLADE_BROT: {
			this->setAllScalarParameters(Beam, this->mBladeBRotParamName, value);
			break;
		}
		case InterpolationIds::SHAPER_ROT: {
			value = value + orientation * 90 + 180; //Blades are in the opposite direction
			value = FMath::DegreesToRadians(value);
			this->setAllScalarParameters(Beam, this->mBladeOrientationName, value);
			break;
		}
	}
}