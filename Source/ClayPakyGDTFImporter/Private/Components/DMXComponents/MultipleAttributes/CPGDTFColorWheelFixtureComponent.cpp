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

//#define ENABLE_OLD_RENDER

#include "Components/DMXComponents/MultipleAttributes/CPGDTFColorWheelFixtureComponent.h"
#include "Factories/Importers/Wheels/CPGDTFWheelImporter.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "PackageTools.h"

bool UCPGDTFColorWheelFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(AttachedGeometryNamee, DMXChannels, attributeIndex);
	FDMXImportGDTFWheel Wheel;
	findWheelObject(Wheel);
	this->WheelColors = FCPGDTFWheelImporter::GenerateColorArray(Wheel);
	
	FString TextureLoadPath = this->GetParentFixtureActor()->FixturePathInContentBrowser;
	FString TextureLoadPathFrosted = TextureLoadPath;
	FString SanitizedWheelName = UPackageTools::SanitizePackageName(Wheel.Name.ToString());
	TextureLoadPath.Append("/textures/" + SanitizedWheelName + "/Wheel_" + SanitizedWheelName + ".Wheel_" + SanitizedWheelName);
	TextureLoadPathFrosted.Append("/textures/" + SanitizedWheelName + "/Wheel_" + SanitizedWheelName + "_Frosted.Wheel_" + SanitizedWheelName + "_Frosted");
	this->WheelTexture = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(TextureLoadPath));
	this->WheelTextureFrosted = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(TextureLoadPathFrosted));
	this->bUseInterpolation = true;
	this->bIsRawDMXEnabled = true;

	return true;
}

void UCPGDTFColorWheelFixtureComponent::BeginPlay() {
	this->mIndexParamName = *CPGDTFRenderPipelineBuilder::getIndexParamName(FCPGDTFWheelImporter::WheelType::Color, this->mAttributeIndexNo);
	FCPDMXChannelData wheelData = *this->attributesData.getChannelData(ECPGDTFAttributeType::Color_n_WheelIndex);
	fixMissingAccelFadeValues(wheelData, 0);
	Super::BeginPlay(1, wheelData.interpolationFade, wheelData.interpolationAcceleration, this->WheelColors.Num(), 0);
	this->interpolations[0].bSpeedCapEnabled = false;

	for (UCPGDTFBeamSceneComponent* Beam : this->AttachedBeams) {

		if (!Beam->HasBegunPlay()) Beam->BeginPlay(); // To avoid a skip of color disk

		FName diskName = *CPGDTFRenderPipelineBuilder::getDiskParamName(FCPGDTFWheelImporter::WheelType::Color, false, this->mAttributeIndexNo);
		FName diskFrostedName = *CPGDTFRenderPipelineBuilder::getDiskParamName(FCPGDTFWheelImporter::WheelType::Color, true, this->mAttributeIndexNo);
		FName numSlot = *CPGDTFRenderPipelineBuilder::getNumSlotsParamName(FCPGDTFWheelImporter::WheelType::Color, this->mAttributeIndexNo);

		setAllTextureParameters(Beam, diskName, this->WheelTexture);
		setAllTextureParameters(Beam, diskFrostedName, this->WheelTextureFrosted);
		setAllScalarParameters(Beam, numSlot, this->WheelColors.Num());
		#ifdef ENABLE_OLD_RENDER
			setAllTextureParameters(Beam, "DMX Color Disk", this->WheelTexture);
			setAllScalarParameters(Beam, "DMX Color Wheel Num Mask", this->WheelColors.Num());
		#endif
	}

	this->CurrentColor = FLinearColor(0, 0, 0);
	this->bUseInterpolation = true;
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->bIsMacroColor = false;
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFColorWheelFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> attributes;

	TSet<ECPGDTFAttributeType> colorIndex;
	colorIndex.Add(ECPGDTFAttributeType::Color_n_);
	colorIndex.Add(ECPGDTFAttributeType::Color_n_WheelIndex);
	attributes.Add(colorIndex);

	TSet<ECPGDTFAttributeType> colorMacro;
	colorMacro.Add(ECPGDTFAttributeType::ColorMacro_n_);
	attributes.Add(colorMacro);
	
	TSet<ECPGDTFAttributeType> colorSpin;
	colorSpin.Add(ECPGDTFAttributeType::Color_n_WheelSpin);
	attributes.Add(colorSpin);
	
	TSet<ECPGDTFAttributeType> colorRandom;
	colorRandom.Add(ECPGDTFAttributeType::Color_n_WheelRandom);
	attributes.Add(colorRandom);

	TSet<ECPGDTFAttributeType> colorAudio;
	colorAudio.Add(ECPGDTFAttributeType::Color_n_WheelAudio);
	attributes.Add(colorAudio);

	return attributes;
}

float UCPGDTFColorWheelFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.5167 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFColorWheelFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.0167 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFColorWheelFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 29.000 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFColorWheelFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[1] = { 0.0323 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}

/**
 * Read DMXValue and apply the effect to the Beam
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 18 july 2022
 *
 * @param DMXValue
*/
void UCPGDTFColorWheelFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	switch (AttributeType) {

		case ECPGDTFAttributeType::Color_n_: //Color indexing
		case ECPGDTFAttributeType::Color_n_WheelIndex:
			this->ColorWheelPeriod = 0;
			this->CurrentTime = 0;
			this->SetTargetValue(physicalValue + (float)DMXBehaviour.Value->WheelSlotIndex, 0);
			break;

		case ECPGDTFAttributeType::ColorMacro_n_: // Color Macro or virtual color wheel
			this->ColorWheelPeriod = 0;
			this->CurrentTime = 0;
			this->interpolations[0].SetValueNoInterp(physicalValue + (float)DMXBehaviour.Value->WheelSlotIndex);
			this->SetValueNoInterp_OverWrite(physicalValue + (float)DMXBehaviour.Value->WheelSlotIndex);
			break;

		case ECPGDTFAttributeType::Color_n_WheelSpin:
			// We stop interpolation
			this->interpolations[0].EndInterpolation(false);

			this->CurrentTime = 0;
			if (physicalValue == 0) this->ColorWheelPeriod = 0;
			else this->ColorWheelPeriod = 360 / physicalValue; // PhysicalValue is an Angular speed in deg/s
			break;

		case ECPGDTFAttributeType::Color_n_WheelRandom:
			// We stop interpolation
			this->interpolations[0].EndInterpolation(false);

			// Here the Physical value is a frenquency in Hz. If the GDTF use default physical values we fallback on default ones
			if (DMXBehaviour.Value->PhysicalFrom == 0 && DMXBehaviour.Value->PhysicalTo == 1) this->ColorWheelPeriod = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, 0.2, 5);
			else this->ColorWheelPeriod = physicalValue;
			break;

		case ECPGDTFAttributeType::Color_n_WheelAudio: // TODO Very complex to implement. For now we disable ColorWheel
		default: break;// Not supported behaviour
		}
}

void UCPGDTFColorWheelFixtureComponent::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {
	float WheelIndex;

	switch (channel.RunningEffectTypeChannel) {
		case ECPGDTFAttributeType::ColorMacro_n_:
			SetValueNoInterp_OverWrite(this->interpolations[0].getCurrentValue());
			break;

		case ECPGDTFAttributeType::Color_n_WheelSpin:
			if (this->ColorWheelPeriod == 0) break;
		
			WheelIndex = (this->interpolations[0].getCurrentValue() + (this->WheelColors.Num()) * (deltaSeconds / this->ColorWheelPeriod));
			WheelIndex = FMath::Fmod(WheelIndex, this->WheelColors.Num());
			if(WheelIndex < 0)
				WheelIndex += this->WheelColors.Num();
			this->SetValueNoInterp(WheelIndex, 0);
			this->interpolations[0].SetValueNoInterp(WheelIndex);
			break;

		case ECPGDTFAttributeType::Color_n_WheelRandom:

			this->CurrentTime += deltaSeconds;
			if (this->CurrentTime >= this->ColorWheelPeriod) {
				this->CurrentTime = 0;
				this->SetTargetValue(FMath::RandHelper(this->WheelColors.Num()), 0);
			}
			break;
		
		default:
			break;
	}
}

/**
 * Apply a color to the light entire output
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 18 july 2022
 *
 * @param Beam
 * @param WheelIndex
 */
void UCPGDTFColorWheelFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float WheelIndex, int interpolationId) {
	//Keeping them for test old lights
	#ifdef ENABLE_OLD_RENDER
		setAllScalarParameters(Beam, "DMX Color Wheel Index", WheelIndex);
	#endif
	setAllScalarParameters(Beam, this->mIndexParamName, WheelIndex);

	#ifdef ENABLE_OLD_RENDER
		int32 IntWheelIndex = FMath::TruncToInt(WheelIndex);
		if (IntWheelIndex < 0) IntWheelIndex += this->WheelColors.Num();
		FLinearColor CrossfadedColor;
		if (IntWheelIndex >= this->WheelColors.Num()) CrossfadedColor = UKismetMathLibrary::LinearColorLerp(this->WheelColors[this->WheelColors.Num() - 1], this->WheelColors[0], FMath::Fractional(WheelIndex));
		else CrossfadedColor = UKismetMathLibrary::LinearColorLerp(this->WheelColors[IntWheelIndex], this->WheelColors[IntWheelIndex + 1], FMath::Fractional(WheelIndex));

		this->CurrentColor = FLinearColor(1, 1, 1, 1) - CrossfadedColor; // We need to invert the color to have the filter one and not the beam color
		this->bIsMacroColor = false;
	#endif
}

/**
 * Apply a color to the light output overwriting other color components values.
 * Used for color macros and virtual color wheels
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 04 august 2022
 *
 * @param WheelIndex
 */
void UCPGDTFColorWheelFixtureComponent::SetValueNoInterp_OverWrite(float WheelIndex) {
	for (UCPGDTFBeamSceneComponent* Component : this->AttachedBeams) {
		Component->ResetLightColorTemp();
		Component->SetLightColor(FLinearColor(1, 1, 1)); // Set White base color
		this->interpolations[0].lastValue = WheelIndex;
		SetValueNoInterp_BeamInternal(Component, WheelIndex, 0);
		this->bIsMacroColor = true;
	}
}

#ifdef ENABLE_OLD_RENDER
#undef ENABLE_OLD_RENDER
#endif