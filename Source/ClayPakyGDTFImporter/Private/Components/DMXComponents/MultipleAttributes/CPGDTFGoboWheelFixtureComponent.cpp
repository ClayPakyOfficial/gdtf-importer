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

#define ENABLE_OLD_RENDER

#include "Components/DMXComponents/MultipleAttributes/CPGDTFGoboWheelFixtureComponent.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "PackageTools.h"

bool UCPGDTFGoboWheelFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	Super::Setup(AttachedGeometryNamee, DMXChannels, attributeIndex);

	FDMXImportGDTFWheel Wheel;
	findWheelObject(Wheel);
	FString TextureLoadPath = this->GetParentFixtureActor()->FixturePathInContentBrowser;
	FString TextureLoadPathFrosted = TextureLoadPath;
	FString SanitizedWheelName = UPackageTools::SanitizePackageName(Wheel.Name.ToString());
	TextureLoadPath.Append("/textures/" + SanitizedWheelName + "/Wheel_" + SanitizedWheelName + ".Wheel_" + SanitizedWheelName);
	TextureLoadPathFrosted.Append("/textures/" + SanitizedWheelName + "/Wheel_" + SanitizedWheelName + "_Frosted.Wheel_" + SanitizedWheelName + "_Frosted");
	this->WheelTexture = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(TextureLoadPath));
	this->WheelTextureFrosted = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(TextureLoadPathFrosted));
	this->NbrGobos = Wheel.Slots.Num();
	this->bIsRawDMXEnabled = true;

	return true;
}

void UCPGDTFGoboWheelFixtureComponent::BeginPlay() {
	this->mIndexParamName = *CPGDTFRenderPipelineBuilder::getIndexParamName(FCPGDTFWheelImporter::WheelType::Gobo, this->mAttributeIndexNo);
	UE_LOG_CPGDTFIMPORTER(Display, TEXT("UCPGDTFGoboWheelFixtureComponent::Setup: mIndexParamName: '%s'"), *this->mIndexParamName.ToString());

	TArray<FCPDMXChannelData> data;
	FCPDMXChannelData goboRotData = *this->attributesData.getChannelData(ECPGDTFAttributeType::Gobo_n_Pos);
	FCPDMXChannelData wheelRotData = *this->attributesData.getChannelData(ECPGDTFAttributeType::Gobo_n_WheelIndex);
	wheelRotData.MinValue = 0;
	wheelRotData.MaxValue = NbrGobos;
	data.Add(goboRotData);
	data.Add(wheelRotData);
	Super::BeginPlay(data);
	this->interpolations[InterpolationIds::WHEEL_ROTATION].bSpeedCapEnabled = false;
	this->interpolations[InterpolationIds::GOBO_ROTATION].bSpeedCapEnabled = false;

	for (UCPGDTFBeamSceneComponent* Beam : this->AttachedBeams) {

		if (!Beam->HasBegunPlay()) Beam->BeginPlay(); // To avoid a skip of gobos disk

		FName diskName = *CPGDTFRenderPipelineBuilder::getDiskParamName(FCPGDTFWheelImporter::WheelType::Gobo, false, this->mAttributeIndexNo);
		FName diskFrostedName = *CPGDTFRenderPipelineBuilder::getDiskParamName(FCPGDTFWheelImporter::WheelType::Gobo, true, this->mAttributeIndexNo);
		FName numSlot = *CPGDTFRenderPipelineBuilder::getNumSlotsParamName(FCPGDTFWheelImporter::WheelType::Gobo, this->mAttributeIndexNo);

		UE_LOG_CPGDTFIMPORTER(Display, TEXT("UCPGDTFGoboWheelFixtureComponent::BeginPlay: Got params names. Disk: '%s'\t Frosted: '%s'\t Num: '%s'"), *diskName.ToString(), *diskFrostedName.ToString(), *numSlot.ToString());

		setAllTextureParameters(Beam, diskName, this->WheelTexture);
		setAllTextureParameters(Beam, diskFrostedName, this->WheelTextureFrosted);
		setAllScalarParameters(Beam, numSlot, this->NbrGobos);
		#ifdef ENABLE_OLD_RENDER //Keeping them for test old lights
			setAllTextureParameters(Beam, "DMX Gobo Disk", this->WheelTexture);
			setAllTextureParameters(Beam, "DMX Gobo Disk Frosted", this->WheelTextureFrosted);
			setAllScalarParameters(Beam, "DMX Gobo Num Mask", this->NbrGobos);
		#endif
	}

	
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->bUseInterpolation = true;
}

TArray<TSet<ECPGDTFAttributeType>> UCPGDTFGoboWheelFixtureComponent::getAttributeGroups() {
	TArray<TSet<ECPGDTFAttributeType>> attributes;

	TSet<ECPGDTFAttributeType> goboIndex;
	goboIndex.Add(ECPGDTFAttributeType::Gobo_n_);
	goboIndex.Add(ECPGDTFAttributeType::Gobo_n_WheelIndex);
	attributes.Add(goboIndex);

	TSet<ECPGDTFAttributeType> goboPos;
	goboPos.Add(ECPGDTFAttributeType::Gobo_n_Pos);
	attributes.Add(goboPos);

	TSet<ECPGDTFAttributeType> goboPosRot;
	goboPosRot.Add(ECPGDTFAttributeType::Gobo_n_PosRotate);
	attributes.Add(goboPosRot);

	TSet<ECPGDTFAttributeType> goboSpin;
	goboSpin.Add(ECPGDTFAttributeType::Gobo_n_WheelSpin);
	goboSpin.Add(ECPGDTFAttributeType::Gobo_n_SelectSpin);
	attributes.Add(goboSpin);

	TSet<ECPGDTFAttributeType> goboShake;
	goboShake.Add(ECPGDTFAttributeType::Gobo_n_WheelShake);
	goboShake.Add(ECPGDTFAttributeType::Gobo_n_SelectShake);
	goboShake.Add(ECPGDTFAttributeType::Gobo_n_PosShake);
	attributes.Add(goboShake);

	TSet<ECPGDTFAttributeType> goboRandom;
	goboRandom.Add(ECPGDTFAttributeType::Gobo_n_WheelRandom);
	attributes.Add(goboRandom);

	TSet<ECPGDTFAttributeType> goboAudio;
	goboAudio.Add(ECPGDTFAttributeType::Gobo_n_WheelAudio);
	attributes.Add(goboAudio);

	return attributes;
}

float UCPGDTFGoboWheelFixtureComponent::getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[2] = { 0.4854, 0.4312 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFGoboWheelFixtureComponent::getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[2] = { 0.1750, 0.0667 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFGoboWheelFixtureComponent::getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[2] = { 0.7738, 4.4688 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}
float UCPGDTFGoboWheelFixtureComponent::getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) {
	const float defaults[2] = { 0.3605, 0.1546 }; //This has to match InterpolationIds enum order!
	return defaults[interpolationId];
}

/**
 * Read DMXValue and apply the effect to the Beam
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 27 july 2022
 *
 * @param DMXValue
 * @param IsFirstChannel
*/
void UCPGDTFGoboWheelFixtureComponent::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {
	switch (AttributeType) {

		case ECPGDTFAttributeType::Gobo_n_:
		case ECPGDTFAttributeType::Gobo_n_WheelIndex: // Gobo selection
			this->GoboWheelPeriod = 0;
			this->WheelCurrentTime = 0;
			this->SetTargetValue(physicalValue + (float)DMXBehaviour.Value->WheelSlotIndex, InterpolationIds::WHEEL_ROTATION);
			break;
			
		case ECPGDTFAttributeType::Gobo_n_Pos: // Gobo indexing
			if (this->channels.Num() < 2) this->SetTargetValue((float)DMXBehaviour.Value->WheelSlotIndex, InterpolationIds::WHEEL_ROTATION); // If there is only one channel set the Gobo
			this->SetTargetValue(physicalValue, InterpolationIds::GOBO_ROTATION); // Set the rotation
			break;

		case ECPGDTFAttributeType::Gobo_n_PosRotate: // Gobo rotation
			if (this->channels.Num() < 2) this->SetTargetValue((float)DMXBehaviour.Value->WheelSlotIndex, InterpolationIds::WHEEL_ROTATION); // If there is only one channel set the Gobo
			// We stop interpolation
			this->interpolations[InterpolationIds::GOBO_ROTATION].EndInterpolation(false);

			if (channel.RunningEffectTypeChannel != AttributeType) this->RotationCurrentTime = 0;
			if (physicalValue == 0) this->GoboRotationPeriod = 0;
			else this->GoboRotationPeriod = 360 / physicalValue; // PhysicalValue is an Angular speed in deg/s
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelSpin:
		case ECPGDTFAttributeType::Gobo_n_SelectSpin: // Gobo wheel rotation
			// We stop interpolation
			this->interpolations[InterpolationIds::GOBO_ROTATION].EndInterpolation(false);

			if (channel.RunningEffectTypeChannel != AttributeType) this->WheelCurrentTime = 0;
			if (physicalValue == 0) this->GoboWheelPeriod = 0;
			else this->GoboWheelPeriod = 360 / physicalValue; // PhysicalValue is an Angular speed in deg/s
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelShake:
		case ECPGDTFAttributeType::Gobo_n_SelectShake:
		case ECPGDTFAttributeType::Gobo_n_PosShake: // Gobo shake
			//if (this->channels.Num() < 2) this->SetTargetValue((float)DMXBehaviour.Value->WheelSlotIndex, InterpolationIds::WHEEL_ROTATION); // If there is only one channel set the Gobo
			// We stop interpolation
			this->interpolations[InterpolationIds::GOBO_ROTATION].EndInterpolation(false);

			if (channel.RunningEffectTypeChannel != AttributeType) this->WheelCurrentTime = 0;
			if (physicalValue == 0) this->GoboWheelPeriod = 0;
			else this->GoboWheelPeriod = 1 / physicalValue; // Physical value is a Frequency
			this->ShakeBaseIndex = DMXBehaviour.Value->WheelSlotIndex;
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelRandom:
			// Here the Physical value is a frenquency in Hz. If the GDTF use default physical values we fallback on default ones
			if (DMXBehaviour.Value->PhysicalFrom == 0 && DMXBehaviour.Value->PhysicalTo == 1) this->GoboWheelPeriod = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, 0.2, 5);
			else this->GoboWheelPeriod = physicalValue;
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelAudio: // Very complex to implement. For now we disable ColorWheel
		default: break;// Not supported behaviour
	}
}

void UCPGDTFGoboWheelFixtureComponent::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {
	float WheelIndex;

	switch (channel.RunningEffectTypeChannel) {
		case ECPGDTFAttributeType::Gobo_n_PosRotate: // Gobo rotation

			if (this->GoboRotationPeriod == 0) break;

			WheelIndex = this->interpolations[InterpolationIds::GOBO_ROTATION].getCurrentValue() + 360.0f * (deltaSeconds / this->GoboRotationPeriod);
			WheelIndex = FMath::Fmod(WheelIndex, 360);
			if (WheelIndex < 0)
				WheelIndex += 360;
			this->SetValueNoInterp(WheelIndex, InterpolationIds::GOBO_ROTATION);
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelSpin:
		case ECPGDTFAttributeType::Gobo_n_SelectSpin: // Gobo wheel rotation

			if (this->GoboWheelPeriod == 0) break;

			WheelIndex = (this->interpolations[InterpolationIds::WHEEL_ROTATION].getCurrentValue() + this->NbrGobos * (deltaSeconds / this->GoboWheelPeriod));
			WheelIndex = FMath::Fmod(WheelIndex, this->NbrGobos);
			if (WheelIndex < 0)
				WheelIndex += this->NbrGobos;
			this->SetValueNoInterp(WheelIndex, InterpolationIds::WHEEL_ROTATION);
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelShake:
		case ECPGDTFAttributeType::Gobo_n_SelectShake:
		case ECPGDTFAttributeType::Gobo_n_PosShake: // Gobo shake

			if (this->GoboWheelPeriod == 0) break;

			this->WheelCurrentTime += deltaSeconds;
			this->SetTargetValue(this->ShakeBaseIndex + FMath::Cos(UE_TWO_PI * (this->WheelCurrentTime / this->GoboWheelPeriod)) / 20, InterpolationIds::WHEEL_ROTATION); // /20 to reduce the shake range 
			break;

		case ECPGDTFAttributeType::Gobo_n_WheelRandom:

			this->WheelCurrentTime += deltaSeconds;
			if (this->WheelCurrentTime >= this->GoboWheelPeriod) {
				this->WheelCurrentTime = 0;
				this->SetTargetValue(FMath::RandHelper(this->NbrGobos), InterpolationIds::WHEEL_ROTATION);
			}
			break;

		default:
			break;
	}
}

/**
 * Apply a Gobo to the light entire output
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 27 july 2022
 *
 * @param Beam
 * @param WheelIndex
 */
void UCPGDTFGoboWheelFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float value, int interpolationId) {
	InterpolationIds iid = (InterpolationIds)interpolationId;
	switch (iid) {
		case InterpolationIds::GOBO_ROTATION: {
			Beam->SetRelativeRotation(FRotator(-90, 0, value));
			break;
		}
		case InterpolationIds::WHEEL_ROTATION: {
			#ifdef ENABLE_OLD_RENDER //Keeping them for test old lights
				setAllScalarParameters(Beam, "DMX Gobo Index", value);
			#endif
			setAllScalarParameters(Beam, this->mIndexParamName, value);

			break;
		}
	}
}

#ifdef ENABLE_OLD_RENDER
#undef ENABLE_OLD_RENDER
#endif