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


#include "Components/DMXComponents/CPGDTFFixtureComponentBase.h"
#include "Library/DMXEntityFixturePatch.h"
#include "Kismet/KismetMathLibrary.h"

UCPGDTFFixtureComponentBase::UCPGDTFFixtureComponentBase() {
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	this->bIsRawDMXEnabled = false;
}

void UCPGDTFFixtureComponentBase::DestroyComponent(bool bPromoteChildren) {
	lclDestroy();
	Super::DestroyComponent(bPromoteChildren);
}

ACPGDTFFixtureActor* UCPGDTFFixtureComponentBase::GetParentFixtureActor() {
	AActor* Parent = GetOwner();
	if (Parent) {
		ACPGDTFFixtureActor* ParentFixtureActor = Cast<ACPGDTFFixtureActor>(Parent);
		return ParentFixtureActor;
	} else return nullptr;
}

bool UCPGDTFFixtureComponentBase::findWheelObject(FDMXImportGDTFWheel& dest) {
	for (int k = 0; k < channels.Num(); k++) {
		for (int i = 0; i < channels[k].GDTFDMXChannelDescription.LogicalChannels.Num(); i++) {
			for (int j = 0; j < channels[k].GDTFDMXChannelDescription.LogicalChannels[i].ChannelFunctions.Num(); j++) {
				if (!channels[k].GDTFDMXChannelDescription.LogicalChannels[i].ChannelFunctions[j].Wheel.Name.IsNone()) {
					dest = channels[k].GDTFDMXChannelDescription.LogicalChannels[i].ChannelFunctions[j].Wheel;
					return true;
				}
			}
		}
	}
	return false;
}

bool UCPGDTFFixtureComponentBase::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) {
	this->AttachedGeometryName = AttachedGeometryNamee;
	this->mAttributeIndexNo = attributeIndex;

	for (int i = 0; i < DMXChannels.Num(); i++) {
		FCPComponentChannelData data;
		FDMXImportGDTFDMXChannel ch = DMXChannels[i];
		data.address = ch.Offset.Num() > 0 ? FMath::Max(1, FMath::Min(512, ch.Offset[0])) : -1;
		data.GDTFDMXChannelDescription = ch;
		data.DMXChannelTree.Insert(ch.LogicalChannels[0], ch.Offset.Num());
		data.RunningEffectTypeChannel = ECPGDTFAttributeType::DefaultValue;
		this->channels.Add(data);
	}
	attributesData.initAttributeGroups(getAttributeGroups());
	attributesData.analizeDMXChannels(DMXChannels);

	return true;
}

inline void UCPGDTFFixtureComponentBase::setAllScalarParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, float value) {
	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetScalarParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetScalarParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightR) Beam->DynamicMaterialSpotLightR->SetScalarParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightG) Beam->DynamicMaterialSpotLightG->SetScalarParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightB) Beam->DynamicMaterialSpotLightB->SetScalarParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialPointLight) Beam->DynamicMaterialPointLight->SetScalarParameterValue(ParameterName, value);
}
inline void UCPGDTFFixtureComponentBase::setAllVectorParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, const FVector& value) {
	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightR) Beam->DynamicMaterialSpotLightR->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightG) Beam->DynamicMaterialSpotLightG->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightB) Beam->DynamicMaterialSpotLightB->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialPointLight) Beam->DynamicMaterialPointLight->SetVectorParameterValue(ParameterName, value);
}
inline void UCPGDTFFixtureComponentBase::setAllVectorParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, const FVector4& value) {
	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightR) Beam->DynamicMaterialSpotLightR->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightG) Beam->DynamicMaterialSpotLightG->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightB) Beam->DynamicMaterialSpotLightB->SetVectorParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialPointLight) Beam->DynamicMaterialPointLight->SetVectorParameterValue(ParameterName, value);
}
inline void UCPGDTFFixtureComponentBase::setAllTextureParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, UTexture* value) {
	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetTextureParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetTextureParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightR) Beam->DynamicMaterialSpotLightR->SetTextureParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightG) Beam->DynamicMaterialSpotLightG->SetTextureParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialSpotLightB) Beam->DynamicMaterialSpotLightB->SetTextureParameterValue(ParameterName, value);
	if (Beam->DynamicMaterialPointLight) Beam->DynamicMaterialPointLight->SetTextureParameterValue(ParameterName, value);
}

void UCPGDTFFixtureComponentBase::BeginPlay(int interpolationsNeededNo, float RealFade, float RealAcceleration, float rangeSize, float defaultValue) {
	initializeInterpolations(interpolationsNeededNo, RealFade, RealAcceleration, rangeSize, defaultValue);
	for (int i = 0; i < this->channels.Num(); i++) 
		if (this->channels[i].DMXChannelTree.IsEmpty())
			this->channels[i].DMXChannelTree.Insert(this->channels[i].GDTFDMXChannelDescription.LogicalChannels[0], this->channels[i].GDTFDMXChannelDescription.Offset.Num());
	Super::BeginPlay();
}
void UCPGDTFFixtureComponentBase::BeginPlay(int interpolationsNeededNo, float RealFade, float RealAcceleration, float maxValue, float minValue, float defaultValue) {
	BeginPlay(interpolationsNeededNo, RealFade, RealAcceleration, FMath::Abs(maxValue - minValue), defaultValue);
}
void UCPGDTFFixtureComponentBase::BeginPlay(TArray<FCPDMXChannelData> interpolationValues) {
	initializeInterpolations(interpolationValues);
	Super::BeginPlay();
}
void UCPGDTFFixtureComponentBase::BeginPlay(TSet<ECPGDTFAttributeType> mainAttributesGroup) {
	if (mainAttributesGroup.Num() > 0) {
		ECPGDTFAttributeType a;
		for (auto& attr : mainAttributesGroup) { a = attr; break; } //pretty inefficent way to get a single random attribute from the set
		BeginPlay(a);
	} else {
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("UCPGDTFFixtureComponentBase: Called BeginPlay(TSet<ECPGDTFAttributeType> mainAttributes) with an empty set! Falling back on the default case"));
		UCPGDTFFixtureComponentBase::BeginPlay(1, 2, 0.5, 1, 0);
	}
}
void UCPGDTFFixtureComponentBase::BeginPlay(ECPGDTFAttributeType mainAttribute) {
	TArray<FCPDMXChannelData> data;
	data.Add(*attributesData.getChannelData(mainAttribute));
	BeginPlay(data);
}

void UCPGDTFFixtureComponentBase::BeginPlay() {
	BeginPlay(attributesData.getStoredChannelDatas());
}

  /*******************************************/
 /*               DMX Related               */
/*******************************************/
	
void UCPGDTFFixtureComponentBase::PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) {
	if (this->bIsRawDMXEnabled) {
		TMap<int32, int32> RawChannelsValues;
		FixturePatch->GetRawChannelsValues(RawChannelsValues);
		this->PushDMXRawValues(FixturePatch, RawChannelsValues);
	}
}

void UCPGDTFFixtureComponentBase::PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap) {
	for (int i = 0; i < this->channels.Num(); i++) {
		FCPComponentChannelData ch = this->channels[i];
		int32 addr = ch.address;
		const int32* DMXValuePtr = RawValuesMap.Find(addr);
		if (DMXValuePtr) {
			int32 DMXValue = *DMXValuePtr;
			if (ch.DMXChannelTree.IsEmpty()) continue;
			this->ApplyEffectToBeam(DMXValue, this->channels[i]);
		}
	}
}

void UCPGDTFFixtureComponentBase::ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel) {
	if (DMXValue == channel.lastDMXValue) return; //Nothing changed so far
	channel.lastDMXValue = DMXValue;

	TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> DMXBehaviour = channel.DMXChannelTree.GetBehaviourByDMXValue(DMXValue);
	// If we are unable to find the behaviour in the tree we can't do anything
	if (DMXBehaviour.Key == nullptr || DMXBehaviour.Value == nullptr) return;
	ECPGDTFAttributeType AttributeType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(DMXBehaviour.Key->Attribute.Name.ToString());
	float PhysicalValue = UKismetMathLibrary::MapRangeClamped(DMXValue, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, DMXBehaviour.Value->PhysicalFrom, DMXBehaviour.Value->PhysicalTo);

	this->ApplyEffectToBeam(DMXValue, channel, DMXBehaviour, AttributeType, PhysicalValue);
	channel.RunningEffectTypeChannel = AttributeType;
}

  /****************************************************/
 /*              Interpolation Related               */
/****************************************************/

void UCPGDTFFixtureComponentBase::fixMissingAccelFadeValues(FCPDMXChannelData& channelData, int interpolationId) {
	bool accelPresent = channelData.isRealAccelerationPresent();
	bool fadePresent = channelData.isRealFadePresent();

	if (!accelPresent && !fadePresent) {
		channelData.interpolationAcceleration = getDefaultRealAcceleration(channelData, interpolationId);
		channelData.interpolationFade = getDefaultRealFade(channelData, interpolationId);
	} else if (!accelPresent || !fadePresent) {
		if (!accelPresent) channelData.interpolationAcceleration = getAccelFromFadeAndRatio(channelData.interpolationFade, getDefaultAccelerationRatio(channelData.interpolationFade, channelData, interpolationId));
		if (!fadePresent) channelData.interpolationFade = getFadeFromAccelAndRatio(channelData.interpolationAcceleration, getDefaultFadeRatio(channelData.interpolationAcceleration, channelData, interpolationId));
	}
}

float UCPGDTFFixtureComponentBase::getAccelFromFadeAndRatio(float fade, float accelRatio) {
	return fade * accelRatio;
}
float UCPGDTFFixtureComponentBase::getFadeFromAccelAndRatio(float accel, float fadeRatio) {
	return accel * 2 + accel * fadeRatio;
}

void UCPGDTFFixtureComponentBase::SetValueNoInterp(float value, int interpolationId, bool updateInterpObject) {
	if (value == this->interpolations[interpolationId].lastValue) return;
	this->interpolations[interpolationId].lastValue = value;
	
	if (updateInterpObject) this->interpolations[interpolationId].SetValueNoInterp(value);
	for (UCPGDTFBeamSceneComponent* component : AttachedBeams) {
		if (component == nullptr) continue;
		SetValueNoInterp_BeamInternal(component, value, interpolationId);
	}
};

void UCPGDTFFixtureComponentBase::SetTargetValue(float value, int interpolationId) {
	FChannelInterpolation *interpolation = &interpolations[interpolationId];
	if (this->HasBegunPlay()) {
		if (this->bUseInterpolation && interpolation->bInterpolationEnabled) {
			interpolation->setTargetValue(value);
		} else {
			interpolation->SetValueNoInterp(value);
			this->SetValueNoInterp(value, interpolationId, true);
		}
	}
}

void UCPGDTFFixtureComponentBase::updateInterpolation(float deltaSeconds, int interpolationId) {
	FChannelInterpolation *interpolation = &interpolations[interpolationId];
	interpolation->Update(deltaSeconds); // Update
	this->SetValueNoInterp(interpolation->getCurrentValue(), interpolationId, false); //This will return immediately if value has not changed, calling this anyway it's not a huge performance loss
}

void UCPGDTFFixtureComponentBase::initializeInterpolation(FChannelInterpolation& interpolation, float RealFade, float RealAcceleration, float range, float defaultValue, int interpolationId) {
	interpolation = FChannelInterpolation(defaultValue);
	interpolation.setFadeAndAcceleration(RealAcceleration, RealFade, range);
	for (UCPGDTFBeamSceneComponent* component : AttachedBeams) {
		if (component == nullptr) continue;
		SetValueNoInterp_BeamInternal(component, defaultValue, interpolationId);
	}
}

void UCPGDTFFixtureComponentBase::initializeInterpolations(int interpolationsNeededNo, float RealFade, float RealAcceleration, float range, float defaultValue) {
	interpolations.Empty();
	for (int i = 0; i < interpolationsNeededNo; i++) {
		FChannelInterpolation interpolation;
		initializeInterpolation(interpolation, RealFade, RealAcceleration, range, defaultValue, i);
		interpolations.Add(interpolation);
	}
}

void UCPGDTFFixtureComponentBase::initializeInterpolations(TArray<FCPDMXChannelData> interpolationValues) {
	interpolations.Empty();
	for (int i = 0; i < interpolationValues.Num(); i++) {
		FChannelInterpolation interpolation;
		fixMissingAccelFadeValues(interpolationValues[i], i);
		initializeInterpolation(interpolation, interpolationValues[i].interpolationFade, interpolationValues[i].interpolationAcceleration, FMath::Abs(interpolationValues[i].MaxValue - interpolationValues[i].MinValue), interpolationValues[i].DefaultValue, i);
		interpolations.Add(interpolation);
	}
}

void UCPGDTFFixtureComponentBase::InterpolateComponent(float deltaSeconds) {
	if (this->AttachedBeams.Num() < 1) return;
	for (int i = 0; i < this->channels.Num(); i++) 
		InterpolateComponent_BeamInternal(deltaSeconds, this->channels[i]);
	for (int i = 0; i < this->interpolations.Num(); i++)
		updateInterpolation(deltaSeconds, i);
}

//not every component has to implement this
void UCPGDTFFixtureComponentBase::InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) {}