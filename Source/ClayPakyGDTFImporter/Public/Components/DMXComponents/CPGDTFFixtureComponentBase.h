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

#pragma once

#include "CoreMinimal.h"
#include "CPGDTFDescription.h"
#include "CPGDTFFixtureActor.h"
#include "CPGDTFInterpolation.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Utils/CPGDTFDMXChannelTree.h"
#include "CPGDTFFixtureComponentBase.generated.h"

#define DEFAULT_REAL_FADE 0.0001
#define DEFAULT_REAL_ACCELERATION 0.00004

/// Struct used to describe the basic DMXChannels properties
USTRUCT(BlueprintType)
struct FCPDMXChannelData {
	GENERATED_BODY()

private:
	
	/// Internally used by FCPDMXChannelData, it's not meant to be used elsewhere
	struct MinMaxHelper {
		int normalNo = 0; //Number of from/to pairs with from < to
		int invertedNo = 0; //Number of from/to pairs with from > to
		TArray<float> minArr; //List with all "from" values
		TArray<float> maxArr; //List with all "to" values

		float fadeAvarage = 0, accelAvarage = 0;

		float totalNo = 0;
		~MinMaxHelper() {
			minArr.Empty();
			maxArr.Empty();
		}
	};

	MinMaxHelper* minMaxHelper = nullptr;

public:
	FCPDMXChannelData() {
		this->address = -1;
		this->MinValue = 0.0f;
		this->MaxValue = 1.0f;
		this->DefaultValue = 0.0f;
		this->interpolationFade = 0.8f;
		this->interpolationAcceleration = 0.4f;
	}

	FCPDMXChannelData(FDMXImportGDTFDMXChannel Channel) {
		this->address = Channel.Offset.Num() > 0 ? FMath::Max(1, FMath::Min(512, Channel.Offset[0])) : -1;
		this->DefaultValue = Channel.Default.Value;
		updateMinMaxFadeAccelCalc(Channel);
		finalizeMinMaxFadeAccelCalc();
	}
	~FCPDMXChannelData() {
		destroy();
	}
	void destroy() {
		if (minMaxHelper != nullptr) {
			delete minMaxHelper;
			minMaxHelper = nullptr;
		}
	}

	/**
	 * Adds the from/to values of a channel function to the min/max calc
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	void updateMinMaxFadeAccelCalc(FDMXImportGDTFChannelFunction channelFunction) {
		if (minMaxHelper == nullptr) minMaxHelper = new MinMaxHelper;
		minMaxHelper->totalNo++;

		//min/max part
		float cfFrom = channelFunction.PhysicalFrom, cfTo = channelFunction.PhysicalTo;
		minMaxHelper->maxArr.Add(cfTo);
		minMaxHelper->minArr.Add(cfFrom);
		if (cfFrom > cfTo) minMaxHelper->invertedNo++;
		else minMaxHelper->normalNo++;

		//fade / acceleration
		minMaxHelper->fadeAvarage += channelFunction.RealFade;
		minMaxHelper->accelAvarage += channelFunction.RealAcceleration;
	}
	/**
	 * Adds all the from/to values in a dmx channel to the min/max calc
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	void updateMinMaxFadeAccelCalc(FDMXImportGDTFDMXChannel Channel) {
		if (minMaxHelper == nullptr) minMaxHelper = new MinMaxHelper;

		for (int i = 0; i < Channel.LogicalChannels.Num(); i++)
			for (int j = 0; j < Channel.LogicalChannels[i].ChannelFunctions.Num(); j++)
				updateMinMaxFadeAccelCalc(Channel.LogicalChannels[i].ChannelFunctions[j]);
	}
	/**
	 * Finalize the calcs of the from/to values and sets the real min and max values
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	void finalizeMinMaxFadeAccelCalc() {
		if (minMaxHelper != nullptr) {
			minMaxHelper->minArr.Sort();
			minMaxHelper->maxArr.Sort(); //Sorts to find the absolute min/max value
			if (minMaxHelper->normalNo > minMaxHelper->invertedNo) { //If we have more normal than inverted min/max values, we take the max from the maxArray and the min from the min array
				this->MaxValue = minMaxHelper->maxArr[minMaxHelper->maxArr.Num() - 1];
				this->MinValue = minMaxHelper->minArr[0];
			} else {
				this->MaxValue = minMaxHelper->maxArr[0];
				this->MinValue = minMaxHelper->minArr[minMaxHelper->minArr.Num() - 1];
			}

			interpolationFade = minMaxHelper->fadeAvarage / minMaxHelper->totalNo;
			interpolationAcceleration = minMaxHelper->accelAvarage / minMaxHelper->totalNo;

			delete minMaxHelper;
			minMaxHelper = nullptr;
		}
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels", meta=(ClampMin="0", ClampMax="512"))
	int32 address = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channel")
	float interpolationFade = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channel")
	float interpolationAcceleration = 0.0f;
	inline bool isRealFadePresent() { return interpolationFade != 0.0f; }
	inline bool isRealAccelerationPresent() { return interpolationAcceleration != 0.0f; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channel")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channel")
	float MaxValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channel")
	float DefaultValue = 0.0f;

	inline bool IsAddressValid() { return this->address != -1; }
};

/// Internally used by FAttributesData, it's not meant to be used elsewhere
USTRUCT(BlueprintType)
struct F__SingleAttributeData {
	GENERATED_BODY()
	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	TSet<ECPGDTFAttributeType> attributeGroup;
	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	FCPDMXChannelData data;
};

/// This will hold the min/max/default values regarding each attribute group 
USTRUCT(BlueprintType)
struct FAttributesData {
	GENERATED_BODY()
private:

	//Map for fast access of the attribute. Multiple attribute types can point to the same attributeData
	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	TMap<ECPGDTFAttributeType, int32> attributeData; //We use ints as index inside attributeList. We can't use pointers since they're not preserved after object loading/unloading
	//List of ALL instanced attributeData
	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	TArray<F__SingleAttributeData> attributeList;

private:
	F__SingleAttributeData* addAttributeData(TSet<ECPGDTFAttributeType> attributeGroup) {
		F__SingleAttributeData attrData = F__SingleAttributeData();
		attrData.attributeGroup.Append(attributeGroup);

		int32 idx = this->attributeList.Add(attrData);
		for (auto& attr : attributeGroup)
			this->attributeData.Add(attr, idx);
		return &this->attributeList[idx];
	}
	F__SingleAttributeData* addAttributeData(ECPGDTFAttributeType attr) {
		TSet<ECPGDTFAttributeType> set;
		set.Add(attr);
		return addAttributeData(set);
	}
public:
	FAttributesData() {}
	~FAttributesData() {
		destroy();
	}
	void destroy() {
		while (!attributeList.IsEmpty()) {
			F__SingleAttributeData current = attributeList[0];
			for (ECPGDTFAttributeType attr : current.attributeGroup)
				attributeData.Remove(attr);
			current.data.destroy();
			//delete current;
			attributeList.RemoveAt(0);
		}
	}
	/**
	 * Analyze the channels finding the min/max/default value per each attribute group
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	void initAttributeGroups(TArray<TSet<ECPGDTFAttributeType>> attributeGroups) {
		attributeData.Empty();
		attributeList.Empty();
		for (int i = 0; i < attributeGroups.Num(); i++)
			addAttributeData(attributeGroups[i]);
	}

	/**
	 * Analyze the channels finding the min/max/default value per each attribute group
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	void analizeDMXChannels(TArray<FDMXImportGDTFDMXChannel> DMXChannels) {
		for (int k = 0; k < DMXChannels.Num(); k++) {
			FDMXImportGDTFDMXChannel ch = DMXChannels[k];
			for (int i = 0; i < ch.LogicalChannels.Num(); i++) {
				for (int j = 0; j < ch.LogicalChannels[i].ChannelFunctions.Num(); j++) {
					FDMXImportGDTFChannelFunction cf = ch.LogicalChannels[i].ChannelFunctions[j];
					ECPGDTFAttributeType attrType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(cf.Attribute.Name.ToString());
					FCPDMXChannelData* channelData = getChannelData(attrType);
					channelData->updateMinMaxFadeAccelCalc(cf);
				}
			}
		}

		for (int i = 0; i < attributeList.Num(); i++)
			attributeList[i].data.finalizeMinMaxFadeAccelCalc();
	}

	/**
	 * Gets the attribute group linked to the input attribute
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	TSet<ECPGDTFAttributeType> getGroupedAttributes(ECPGDTFAttributeType attr) {
		TSet<ECPGDTFAttributeType> ret;
		int32* idx = attributeData.Find(attr);
		if (idx != nullptr) ret.Append(attributeList[*idx].attributeGroup);
			else ret.Append(addAttributeData(attr)->attributeGroup); //if the attribute was not found, create a new one
		return ret;
	}
	/**
	 * Obtains the channel data linked to the input attribute
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	FCPDMXChannelData* getChannelData(ECPGDTFAttributeType attr) {
		FCPDMXChannelData* ret = nullptr;
		int32* idx = attributeData.Find(attr);
		//UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: DIO CANE EH %08x"), ptr);
		if (idx != nullptr) ret = &(attributeList[*idx].data);
			else ret = &(addAttributeData(attr)->data); //if the attribute was not found, create a new one
		return ret;
	}
	/**
	 * Obtains every attribute group stored so far
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	TSet<ECPGDTFAttributeType> getStoredAttributeGroups() {
		//I may be dumb, but I haven't found a way to get the keys from the attributeData map
		TSet<ECPGDTFAttributeType> ret;
		for (int i = 0; i < attributeList.Num(); i++)
			ret.Append(attributeList[i].attributeGroup);
		return ret;
	}
	/**
	 * Obtains every attribute data stored so far
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	TArray<FCPDMXChannelData> getStoredChannelDatas() {
		TArray<FCPDMXChannelData> ret;
		for (int i = 0; i < attributeList.Num(); i++)
			ret.Add(attributeList[i].data);
		return ret;
	}
};

/// Struct containing data about one specific "physical" dmx channel
USTRUCT(BlueprintType)
struct FCPComponentChannelData {
	GENERATED_BODY()

	/// Dmx channel number inside the dmx universe
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "512", DisplayName = "Gobo Selection Channel Address"), Category = "DMX Channels")
	int32 address;
	/// Channel description of the dmx channel
	UPROPERTY()
	FDMXImportGDTFDMXChannel GDTFDMXChannelDescription;

	/// Channel tree of the logical channel of this dmx channel
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	FDMXChannelTree DMXChannelTree;
	/// Last attribute type that this channel had. It's automatically updated by ApplyEffectToBeam
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	ECPGDTFAttributeType RunningEffectTypeChannel;

	/// Last dmx value that this channel had
	int32 lastDMXValue = -1;
};

//TODO Rewrite these, since they changed when I rewrote the interpolation
/** \defgroup DMXComp DMXComponents
\startuml
title Unreal ClaypackyGDTFImporter : DMXComponents Class Diagram
!theme blueprint
left to right direction

'Top Classes
abstract class FixtureComponentBase{
	# FName AttachedGeometryName
	# USceneComponent* AttachedGeometry
	# bool bIsRawDMXEnabled
	+ float SkipThreshold = 0.003f;
	+ bool bUseInterpolation = true;
	+ float InterpolationScale = 1.0f;

	+ UCPGDTFFixtureComponentBase()
	+ {virtual} void OnConstruction()
	+ void Setup(FName AttachedGeometryName)
	+ class ACPGDTFFixtureActor* GetParentFixtureActor()
	+ {virtual} void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap)
	+ {virtual} void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap)
	+ {virtual} {override} void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
	+ {virtual} void InterpolateComponent(float DeltaSeconds)
	+ {virtual} const bool IsDMXInterpolationDone()
	+ {virtual} void ApplySpeedScale()
}

abstract class SimpleAttributeFixtureComponent {
	+ FCPDMXChannelData DMXChannel
	+ FChannelInterpolation CurrentValue

	+ UCPGDTFSimpleAttributeFixtureComponent()
	+ {virtual} void BeginPlay()
	+ {virtual} void Setup(FDMXImportGDTFDMXChannel DMXChannel)
	+ {override} void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap)
	+ {virtual} {override} void InterpolateComponent(float DeltaSeconds)
	+ const float GetDMXInterpolatedStep()
	+ const	float GetDMXInterpolatedValue()
	+ const float GetDMXTargetValue()
	+ {override} const bool IsDMXInterpolationDone()
	+ {virtual} void SetValueNoInterp(float NewValue)
	+ {virtual} void SetTargetValue(float AbsoluteValue)
	+ {override} ApplySpeedScale()
	+ const float NormalizedToAbsoluteValue(float Alpha)
	+ bool IsTargetValid(float Target)
}

abstract class SimpleAttributeBeamFixtureComponent {
	+ TArray<UCPGDTFBeamSceneComponent*> AttachedBeams

	+ {virtual} {override} void OnConstruction()
	+ {virtual} {override} void SetValueNoInterp(float Value)
	# {virtual} void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value)
}

abstract class MultipleAttributeFixtureComponent {
	+ UCPGDTFMultipleAttributeFixtureComponent()
	+ {virtual} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels)
	+ {virtual} void SetValueNoInterp(float Value)
}

abstract class MultipleAttributeBeamFixtureComponent {
	+ TArray<UCPGDTFBeamSceneComponent*> AttachedBeams;

	+ {virtual} {override} void OnConstruction()
	+ {virtual} {override} void SetValueNoInterp(float Value)
	# {virtual} void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value)
}

FixtureComponentBase <|-- SimpleAttributeFixtureComponent
SimpleAttributeFixtureComponent <|-- SimpleAttributeBeamFixtureComponent
FixtureComponentBase <|-- MultipleAttributeFixtureComponent
MultipleAttributeFixtureComponent <|-- MultipleAttributeBeamFixtureComponent

'Simple attributes components
class MovementFixtureComponent {
	# bool bInvertRotation
	# ECPGDTFMovementFixtureType MovementType

	+ UCPGDTFMovementFixtureComponent()
	+ {override} void OnConstruction()
	+ {override} void Setup(FDMXImportGDTFDMXChannel DMXChannel)
	+ {override} void SetValueNoInterp(float Rotation)
	+ double GetRotation()
	+ {override} void SetTargetValue(float AbsoluteValue)
	+ {override} void InterpolateComponent(float DeltaSeconds)
}

class DimmerFixtureComponent {
	+ UCPGDTFDimmerFixtureComponent()
	+ {override} void BeginPlay()
	+ {override} void InterpolateComponent(float DeltaSeconds)
	# {override} void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Intensity)
}

class ZoomFixtureComponent {
	+ UCPGDTFZoomFixtureComponent()
	+ {override} void InterpolateComponent(float DeltaSeconds)
	# {override} void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Angle)
}

SimpleAttributeFixtureComponent <|-- MovementFixtureComponent
SimpleAttributeBeamFixtureComponent <|-- DimmerFixtureComponent
SimpleAttributeBeamFixtureComponent <|-- ZoomFixtureComponent

'Color source components
abstract class ColorSourceFixtureComponent {
	# FLinearColor CurrentColor

	+ void BeginPlay()
	+ FLinearColor GetCurrentColor()
}

abstract class AdditiveColorFixtureComponent {
	+ bool IsTargetValid(FLinearColor NewColor)
}

abstract class SubstractiveColorFixtureComponent {
	+ FLinearColor ApplyFilter(FLinearColor)
}

abstract class ColorCorrectionFixtureComponent {
	# FDMXChannelTree DMXChannelTree
	# FDMXImportGDTFDMXChannel GDTFDMXChannelDescription
	+ int32 ChannelAddress
}

class AdditiveSourceFixtureComponent {
	+ FCPDMXColorChannelData DMXChannelRed
	+ FCPDMXColorChannelData DMXChannelGreen
	+ FCPDMXColorChannelData DMXChannelBlue
	+ FCPDMXColorChannelData DMXChannelWhite
	+ FCPDMXColorChannelData DMXChannelCyan
	+ FCPDMXColorChannelData DMXChannelMagenta
	+ FCPDMXColorChannelData DMXChannelYellow
	+ FCPDMXColorChannelData DMXChannelAmber
	+ FCPDMXColorChannelData DMXChannelLime
	+ FCPDMXColorChannelData DMXChannelBlueGreen
	+ FCPDMXColorChannelData DMXChannelLightBlue
	+ FCPDMXColorChannelData DMXChannelPurple
	+ FCPDMXColorChannelData DMXChannelPink
	+ FCPDMXColorChannelData DMXChannelWarmWhite
	+ FCPDMXColorChannelData DMXChannelCoolWhite
	+ FCPDMXColorChannelData DMXChannelUV

	+ UCPGDTFAdditiveColorSourceFixtureComponent()
	+ {virtual} {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables)
	+ {virtual} {override} void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap)
}

class SubstractiveSourceFixtureComponent {
	+ FCPDMXColorChannelData DMXChannelRed
	+ FCPDMXColorChannelData DMXChannelGreen
	+ FCPDMXColorChannelData DMXChannelBlue
	+ FCPDMXColorChannelData DMXChannelCyan
	+ FCPDMXColorChannelData DMXChannelMagenta
	+ FCPDMXColorChannelData DMXChannelYellow

	+ UCPGDTFSubstractiveColorSourceFixtureComponent()
	+ {virtual} {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables)
	+ {virtual} {override} void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap)
}

class CIEColorSourceFixtureComponent {
	+ FCPDMXColorChannelData DMXChannelX
	+ FCPDMXColorChannelData DMXChannelY
	+ FCPDMXColorChannelData DMXChannelYY

	+ UCPGDTFCIEColorSourceFixtureComponent()
	+ {virtual} {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables)
	+ {virtual} {override} void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap)
}

class HSVColorSourceFixtureComponent {
	+ FCPDMXColorChannelData DMXChannelH
	+ FCPDMXColorChannelData DMXChannelS
	+ FCPDMXColorChannelData DMXChannelV

	+ UCPGDTFHSVColorSourceFixtureComponent()
	+ {virtual} {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables)
	+ {virtual} {override} void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap)
}

class ColorWheelFixtureComponent {
	# ECPGDTFAttributeType RunningEffectType
	# FDMXChannelTree DMXChannelTree
	# FDMXImportGDTFDMXChannel GDTFDMXChannelDescription
	# UTexture2D* WheelTexture
	# TArray<FLinearColor> WheelColors
	# float ColorWheelPeriod
	# float CurrentTime
	# float CurrentWheelIndex
	# bool bIsMacroColor
	+ int32 ChannelAddress
  
	+ UCPGDTFColorWheelFixtureComponent()
	+ {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels)
	+ void BeginPlay()
	+ {virtual} {override} void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap)
	+ void ApplyEffectToBeam(int32 DMXValue)
	+ {override} void InterpolateComponent(float DeltaSeconds)
	+ FLinearColor GetCurrentColor()
	+ bool IsMacroColor()
	# {override}void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float WheelIndex)
}

class CTOFixtureComponent {
	# bool IsCTOEnabled
	# float ColorTemperature
  
	+ UCPGDTFCTOFixtureComponent()
	+ {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels)
	+ void BeginPlay()
	+ {virtual} {override} void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap)
	+ {override} void InterpolateComponent(float DeltaSeconds)
	# {override} void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value)
}

MultipleAttributeBeamFixtureComponent <|-- ColorSourceFixtureComponent
ColorSourceFixtureComponent <|-- AdditiveColorFixtureComponent
ColorSourceFixtureComponent <|-- SubstractiveColorFixtureComponent
ColorSourceFixtureComponent <|-- ColorCorrectionFixtureComponent
AdditiveColorFixtureComponent <|-- AdditiveSourceFixtureComponent
AdditiveColorFixtureComponent <|-- CIEColorSourceFixtureComponent
AdditiveColorFixtureComponent <|-- HSVColorSourceFixtureComponent
SubstractiveColorFixtureComponent <|-- ColorWheelFixtureComponent
SubstractiveColorFixtureComponent <|-- SubstractiveSourceFixtureComponent
ColorCorrectionFixtureComponent <|-- CTOFixtureComponent

'Multiple attributes components
class GoboWheelFixtureComponent {
	# ECPGDTFAttributeType RunningEffectTypeChannelOne
	# ECPGDTFAttributeType RunningEffectTypeChannelTwo
	# FDMXChannelTree DMXChannelTreeOne
	# FDMXChannelTree DMXChannelTreeTwo
	# FDMXImportGDTFDMXChannel GDTFDMXChannelDescriptionOne
	# FDMXImportGDTFDMXChannel GDTFDMXChannelDescriptionTwo
	# UTexture2D* WheelTexture
	# UTexture2D* WheelTextureFrosted
	# int NbrGobos
	# float WheelCurrentIndex
	# float GoboCurrentAngle
	# float ShakeBaseIndex
	# float GoboWheelPeriod
	# float WheelCurrentTime
	# float GoboRotationPeriod
	# float RotationCurrentTime
	+ int32 AddressChannelOne
	+ int32 AddressChannelTwo

	+ UCPGDTFGoboWheelFixtureComponent()
	+ {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels)
	+ void BeginPlay()
	+ {virtual} {override} void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap)
	+ void ApplyEffectToBeam(int32 DMXValue, bool IsFirstChannel)
	+ {override} void InterpolateComponent(float DeltaSeconds)
	# {override} void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float WheelIndex)
	# void SetValueNoInterpGoboRotation(float RotationAngle)
	# void SetValueNoInterpGoboDeltaRotation(float DeltaRotationAngle)
}

class ShutterFixtureComponent {
	# ECPGDTFAttributeType RunningEffectType
	# FDMXChannelTree DMXChannelTree
	# FDMXImportGDTFDMXChannel GDTFDMXChannelDescription
	# FPulseEffectManager PulseManager
	# float RandomPhysicalFrom
	# float RandomPhysicalTo
	# float RandomCurrentFrequency
	# float RandomCurrentTime
	+ int32 ChannelAddress

	+ UCPGDTFShutterFixtureComponent()
	+ {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels)
	+ void BeginPlay()
	+ {virtual} {override} void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap)
	+ void ApplyEffectToBeam(int32 DMXValue)
	+ {override}void InterpolateComponent(float DeltaSeconds)
	+ {override}void SetValueNoInterp(float Value)
	# void ApplyParametersToBeam(float Intensity, float Frequency)
	# void StartPulseEffect(ECPGDTFAttributeType AttributeType, float Period, FCPGDTFChannelFunction* ChannelFunction)
	# void StartRandomEffect(int32 DMXValue, FCPGDTFChannelFunction* ChannelFunction, FCPGDTFChannelSet* ChannelSet)
}

class FrostFixtureComponent {
	# ECPGDTFAttributeType RunningEffectType
	# FDMXChannelTree DMXChannelTree
	# FDMXImportGDTFDMXChannel GDTFDMXChannelDescription
	# FPulseEffectManager PulseManager
	+ int32 ChannelAddress

	+ UCPGDTFFrostFixtureComponent()
	+ {override} void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels)
	+ void BeginPlay()
	+ {virtual} {override} void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap)
	+ void ApplyEffectToBeam(int32 DMXValue)
	+ {override}void InterpolateComponent(float DeltaSeconds)
	# void StartPulseEffect(ECPGDTFAttributeType AttributeType, float Period, FCPGDTFChannelFunction* ChannelFunction)
	# {override} SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value)
}

MultipleAttributeBeamFixtureComponent <|-- GoboWheelFixtureComponent
MultipleAttributeBeamFixtureComponent <|-- ShutterFixtureComponent
MultipleAttributeBeamFixtureComponent <|-- FrostFixtureComponent
\enduml
*/

/** Base Object of all the DMXComponents */
/// \ingroup DMXComp
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = "DMX", Abstract, meta=(RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFFixtureComponentBase : public UActorComponent {
	GENERATED_BODY()

protected:

	/// Array of dmx channels info. Each channel is related to one "physical" input dmx channel of the light
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	TArray<FCPComponentChannelData> channels;
	/// Array of interpolation objects. Each interpolation controls a "physical" feature of the light
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	TArray<FChannelInterpolation> interpolations;
	/// Object containing the info about each attribute group
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	FAttributesData attributesData;

	/// Geometry name
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	FName AttachedGeometryName;

	/// Attached geometries
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	USceneComponent* AttachedGeometry;

	/// Enable the PushDMXRawValues() method if we need no normalized values
	UPROPERTY()
	bool bIsRawDMXEnabled;

	/// Index per attribute type (EG: Gobo1, Color1, Color2, etc)
	UPROPERTY()
	int mAttributeIndexNo;

public:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	TArray<UCPGDTFBeamSceneComponent*> AttachedBeams;

	/// Value changes smaller than this threshold are ignored 
	UPROPERTY(EditAnywhere, Category = "DMX Parameters")
	float SkipThreshold = 0.003f;

	/// If used within a DMX Fixture Actor or Fixture Matrix Actor, the plugin interpolates towards the last set value. 
	UPROPERTY(EditAnywhere, Category = "DMX Parameters")
	bool bUseInterpolation = true;

	UCPGDTFFixtureComponentBase();
	void DestroyComponent(bool bPromoteChildren = false) override;
private:
	void lclDestroy() {
		for (FChannelInterpolation& i : interpolations) i.destroyInterpolation();
		attributesData.destroy();
	}
public:
	~UCPGDTFFixtureComponentBase() {
		lclDestroy();
	}

	// Initializes the component on spawn on a world
	virtual void OnConstruction() {
		this->AttachedBeams = this->GetParentFixtureActor()->GeometryTree.GetBeamsUnderGeometry(this->AttachedGeometryName);
	};

	/// If attached to a DMX Fixture Actor, returns the parent fixture actor. 
	UFUNCTION(BlueprintCallable, Category = "DMX")
	class ACPGDTFFixtureActor* GetParentFixtureActor();

protected:

	/**
	 * Finds the first wheel object it's able to find between all channels, logical channels and channel functions
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param dest Destination wheel
	 */
	bool findWheelObject(FDMXImportGDTFWheel& dest);

	/**
	 * Sets the specified scalar/float parameter to all material instances attached to the beam
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param Beam Beam holding the material instances
	 * @param ParameterName Name of the parameter to set
	 * @param value Scalar/float to apply to the parameter
	 */
	inline void setAllScalarParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, float value);
	/**
	 * Sets the specified vector parameter to all material instances attached to the beam
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param Beam Beam holding the material instances
	 * @param ParameterName Name of the parameter to set
	 * @param value Vector to apply to the parameter
	 */
	inline void setAllVectorParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, const FVector& value);
	/**
	 * Sets the specified vector4 parameter to all material instances attached to the beam
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param Beam Beam holding the material instances
	 * @param ParameterName Name of the parameter to set
	 * @param value Vector4 to apply to the parameter
	 */
	inline void setAllVectorParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, const FVector4& value);
	/**
	 * Sets the specified texture parameter to all material instances attached to the beam
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param Beam Beam holding the material instances
	 * @param ParameterName Name of the parameter to set
	 * @param value Texture object to apply to the parameter
	 */
	inline void setAllTextureParameters(UCPGDTFBeamSceneComponent* Beam, FName ParameterName, UTexture* value);

	/**
	 * Initialize the specified amout of interpolations using the input values
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param interpolationsNeededNo Number of interpolations you want to initialize
	 * @param RealFade Time in second to move through the entire range size
	 * @param RealAcceleration Time in seconds to accelerate from stop to max velocity
	 * @param range Range between the maximum and the minimum value that the interpolations can reach
ì	 * @param defaultValue Initial value of the interpolations
	 */
	virtual void BeginPlay(int interpolationsNeededNo, float RealFade, float RealAcceleration, float rangeSize, float defaultValue);
	/**
	 * Initialize the specified amout of interpolations using the input values
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param interpolationsNeededNo Number of interpolations you want to initialize
	 * @param RealFade Time in second to move through the entire range size
	 * @param RealAcceleration Time in seconds to accelerate from stop to max velocity
	 * @param maxValue Max physical value the interpolations can reach
	 * @param minValue Min physical value the interpolations can reach
	 * @param defaultValue Initial value of the interpolations
	 */
	virtual void BeginPlay(int interpolationsNeededNo, float RealFade, float RealAcceleration, float maxValue, float minValue, float defaultValue);
	/**
	 * Initialize one interpolation per each FCPDMXChannelData using its min/max/default values
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param interpolationValues reference attributes
	 */
	virtual void BeginPlay(TArray<FCPDMXChannelData> interpolationValues);
	/**
	 * Initialize a single interpolation using the min/max/default value of the input attribute group
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param mainAttributesGroup reference attributeGroup
	 */
	virtual void BeginPlay(TSet<ECPGDTFAttributeType> mainAttributesGroup);
	/**
	 * Initialize a single interpolation using the min/max/default value of the input attribute
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param mainAttribute reference attribute
	 */
	virtual void BeginPlay(ECPGDTFAttributeType mainAttribute);

public:

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/**
	 * Handles new DMX packets with values in the "normalized" range of 0.0f - 1.0f
	 *
	 * @param FixturePatch
	 * @param RawValuesMap The full dmx universe in the 0.0f - 1.0f range
+	 */
	UFUNCTION(BlueprintCallable, Category = "DMX Component")
	virtual void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap);

	/**
	 * Handles new DMX packets with values in the "classic" range of 0-255
	 * From the incoming dmx universe, for every channel that this component "has", we obtain its specific dmx value and
	 * we pass it to ApplyEffectToBeam to handle it
	 *
	 * @param FixturePatch 
	 * @param RawValuesMap The full dmx universe in the 0-255 range
+	 */
	UFUNCTION(BlueprintCallable, Category = "DMX Component")
	virtual void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap);

protected:
	/**
	 * This is the function that's called each time we receive a DMX packet.
	 * Its purpose is to analyze it obtaining the dmx behaviour based on the channel and on the dmx value, parse the attribute type, calculate
	 * the physical value, and pass everything to the internal ApplyEffectToBeam that the final user MUST redefine
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param DMXValue The original dmx value we have receviced
	 * @param channel The channel where we have received the dmx value
	 */
	void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel);

	/*******************************************/
   /*          Interpolation Related          */
  /*******************************************/

	/**
	 * Checks if RealFade and RealAcceleration values are present in the channel data and if not generates them using default values and ratios.
	 * If both RealFade and RealAcceleration values are missing (so they're both equals to 0 in the GDTF file), we will use component's default values
	 * If only one of RealFade or RealAcceleration is missing we generate it using the component's acceleration/fade ratio and the other, present, value
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param channelData data about the channel we're using to create the interpolation. You can get the min/max values (and the range) or the RealAcceleration from that
	 * @param interpolationId id of the interpolation we're creating. You can use that to return different default values for different interpolations
	 */
	inline void fixMissingAccelFadeValues(FCPDMXChannelData& channelData, int interpolationId);

	/**
	 * Calcs a presumed RealAcceleration value from the RealFade and the ratio between the RealAcceleration and the RealFade
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param fade RealFade
	 * @param accelRatio Ratio between RealAcceleration and RealFade (= RealAcceleration / RealFade)
	 * @return Presumed original RealAcceleration
	 */
	inline float getAccelFromFadeAndRatio(float fade, float accelRatio);
	/**
	 * Calcs a presumed RealFade value from the RealAcceleration and the ratio between the RealFade and the RealAcceleration
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param accel RealAcceleration
	 * @param fadeRatio Ratio between RealFade and the RealAcceleration (= (RealFade - 2 * RealAcceleration) / RealAcceleration)
	 * @return Presumed original RealFade
	 */
	inline float getFadeFromAccelAndRatio(float accel, float fadeRatio);

	/**
	 * This function should take the input value and applies it to the level's component parameters.
	 * It also takes an interpolationId to differentiate different parameters to apply,
	 * in case there are multiple attributes to control at runtime to get this feature working (EG Blades who have blade A and blade Rot).
	 * When interpolation is enabled this function is automatically called by the plugin until the target value is reached, else just once.
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param value value to set to the parameters
	 * @param interpolationId Selects which parameter you should apply for components that needs more runtime parameters
	 * @param updateInterpObject If true it will set the value without interpolation also to the specified interpolation object
	*/
	UFUNCTION(BlueprintCallable, Category = "DMX")
	virtual void SetValueNoInterp(float value, int interpolationId, bool updateInterpObject = true);

	/**
	 * Sets the target value to the specified interpolation
	 *
	 * @param value The new interpolation's target
	 * @param interpolationId id of the interpolation we're updating
+	 */
	UFUNCTION(BlueprintCallable, Category = "DMX")
	void SetTargetValue(float value, int interpolationId);

	/**
	 * Updates the specified interpolation, by making it travel, ending the interpolation if it is done and "sending" the updated values to the light.
	 * It's called each tick by updateInterpolations()
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 02 february 2023
	 *
	 * @param deltaSeconds Seconds passed since last time this method was called
	 * @param interpolationId id of the interpolation we're updating
+	 */
	UFUNCTION(BlueprintCallable, Category = "DMX")
	void updateInterpolation(float deltaSeconds, int interpolationId);

	/**
	 * Initialize the input interpolation object using the input values
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 02 february 2023
	 *
	 * @param interpolation interpolation object to initialize
	 * @param RealFade Time in second to move through the entire range size
	 * @param RealAcceleration Time in seconds to accelerate from stop to max velocity
	 * @param range Dimension of the interpolation (EG max-min value)
	 * @param defaultValue default value that the interpolation assumes originally
	 * @param interpolationId id of the interpolation used to set the correct values in SetValueNoInterp()
	 */
	void initializeInterpolation(FChannelInterpolation& interpolation, float RealFade, float RealAcceleration, float range, float defaultValue, int interpolationId);
	/**
	 * Initialize the input interpolation object using the input values
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 02 february 2023
	 *
	 * @param interpolation interpolation object to initialize
	 * @param RealFade Time in second to move through the entire range size
	 * @param RealAcceleration Time in seconds to accelerate from stop to max velocity
	 * @param min Minimum value that this interpolation can reach
	 * @param max Maximum value that this interpolation can reach
	 * @param defaultValue default value that the interpolation assumes originally
	 * @param interpolationId id of the interpolation used to set the correct values in SetValueNoInterp()
	 */
	inline void initializeInterpolation(FChannelInterpolation& interpolation, float RealFade, float RealAcceleration, float min, float max, float defaultValue, int interpolationId) {
		initializeInterpolation(interpolation, RealFade, RealAcceleration, FMath::Abs(max - min), defaultValue, interpolationId);
	}
	/**
	 * Initialize the specified amount of interpolation objects needed by the component
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 02 february 2023
	 *
	 * @param interpolation interpolation object to initialize
	 * @param RealFade Time in second to move through the entire range size
	 * @param RealAcceleration Time in seconds to accelerate from stop to max velocity
	 * @param range Dimension of the interpolation (EG max-min value)
	 * @param defaultValue default value that the interpolation assumes originally
	*/
	void initializeInterpolations(int interpolationsNeededNo, float RealFade, float RealAcceleration, float range, float defaultValue);
	/**
	 * Initialize the specified amount of interpolation objects needed by the component
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 02 february 2023
	 *
	 * @param interpolation interpolation object to initialize
	 * @param RealFade Time in second to move through the entire range size
	 * @param RealAcceleration Time in seconds to accelerate from stop to max velocity
	 * @param min Minimum value that this interpolation can reach
	 * @param max Maximum value that this interpolation can reach	 * @param defaultValue default value that the interpolation assumes originally
	*/
	inline void initializeInterpolations(int interpolationsNeededNo, float RealFade, float RealAcceleration, float min, float max, float defaultValue) {
		initializeInterpolations(interpolationsNeededNo, RealFade, RealAcceleration, FMath::Abs(max - min), defaultValue);
	}

	/**
	 * Initialize an interpolation object for each element in the input array, using its min/max/default value
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	inline void initializeInterpolations(TArray<FCPDMXChannelData> interpolationValues);

public:


	/**
	 * Called each tick to update the component's state of running effects and interpolations
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param deltaSeconds Seconds passed since last time this method was called
	 */
	UFUNCTION(BlueprintCallable, Category = "DMX")
	void InterpolateComponent(float deltaSeconds);


	/*
	                ██████████████████
	        ██████████████████████████████████
	██████████████████████████████████████████████████
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	TO HAVE A NEW WORKING FIXTURE COMPONENT, JUST IMPLEMENT THE FOLLOWING FUNCTIONS

	Calling Super::Setup and Super::BeginPlay is MANDATORY if you reimplement them!!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	██████████████████████████████████████████████████
	        ██████████████████████████████████
	                ██████████████████
	*/

	/**
	Component arch explained:

	Inside your component you have  ̶(̶t̶w̶o̶ ̶w̶o̶l̶v̶e̶s̶) access to 3 main attributes:
	- TArray<FCPComponentChannelData> channels: Since each light could have multiple channels to control a single feature (like gobo selector + a second channel to control the gobo rotation)
		we store multiple FCPComponentChannelData. Each one contains information about that DMX channel, like its address, its description, its channel functions, etc.
		This array is used widely inside the component: For each dmx packet we receive we have to handle again all of the dmx channels and each tick we have to update what the channels are doing
	- TArray<FChannelInterpolation> interpolations: kinda unrelated to the channel concept, we have the interpolations. An interpolation is something that the light phisically moves irl. The pan?
		An interpolation. The gobo rotation? An interpolation. The gobo wheel rotation? Another interpolation! A component can have N channels and M interpolations, there's no relations between them.
		Interpolation's targets are updated by the "user" each time we receive a dmx packet (ApplyEffectToBeam()) or at each tick (InterpolateComponent_BeamInternal()), and the value is updated automatically at each tick
	- FAttributesData attributesData: A struct you can query to obtain the FCPDMXChannelData or the attribute group related to a single attribute. It can be used by the user in the BeginPlay phase to initialize the interpolations.
		If no BeginPlay() method is reimplemented by the user, by default all of the attributesData found in the input channels are used to initialize the interpolations

	████ It's MANDATORY to implement the getAttributesGroup(), ApplyEffectToBeam() and SetValueNoInterp_BeamInternal() since they're heavily related to the component we're implementing ████

	Component life:
	- It all starts with the OnConstruct(), which is automatically called by UnrealEngine. It's not mandatory to reimplement this, actually only one component has reimplemented this so far, but if you want i'm just a text, I can't phisically stop you :D
	
	- Then FCPFActorComponentsLoader searches with a regex all the dmx channel descriptors related to this component and calls Setup() with the said component
	- The component, in the Setup() phase, will initialize the channels list, finding, per each attribute, the default/min/max/realFade/realAcceleration values, and store them in the attributesData struct
	- Users have the chance to override the Setup method (But a call to Super::Setup() is MANDATORY!) to implement more stuff needed by the component (EG: Loading a texture)

	- When the level will start running, the BeginPlay() method is called. This will initialize the interpolations using what's stored in the attributesData struct. By default it will create an interpolation per each
		attribute found in the Setup() phase, however users can control on which attributes we should create an interpolation. With "interpolation made over an attribute" I mean that the interpolation object will take its range and default value from the attribute's min/max/default.
		The BeingPlay() will also try to set the RealFade and RealAcceleration per each component using what's found in the GDTF. If a value is missing (EG the value == 0) there will be a call first to getDefaultRealAcceleration() and then to getDefaultRealFade().
		They return 0 by default, but you can override them so you can set a default value or calculate one based on the acceleration/fade (check if they're present tho!).
		You can always override the accel/fade values by manually calling setMaxSpeed()/setAcceleration()/setFadeAndAcceleration() on the interpolation object
	- Users, in the BeginPlay method, should also initialize the UE functional parameters that needs to be set before the simulation starts (EG: The wheel's textures)

	At this point, the component is running

	- Each time we receive a DMX packet, ACPGDTFFixtureActor calls PushNormalizedRawValues on each component with the whole dmx universe. This function can be overridden by the user if they need directly the normalized value
	- By default, PushNormalizedRawValues will call PushDMXRawValues
	- Per each channel in the channels array, we obtain its dmx value from the universe and we call ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel) with the value and the channel
	- ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel) will obtain the current dmx behaviour from the channel and value, parse the attribute type,
	    and calc the physical value of the dmx behaviour. Later it will call the internal ApplyEffectToBeam, the one that the user MUST implement
	  - The internal ApplyEffectToBeam is OVERRIDDEN BY THE USER and will, based on the attribute type, do its stuff, setting the target value of the interpolations, or setting values without interpolation on the component and/or on the interpolation object
	- After that ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel) sets the channel's current effect to the attribute type

	- At each tick ACPGDTFFixtureActor calls InterpolateComponent()
	- InterpolateComponent() first calls, per each channel in the channels array, InterpolateComponent_BeamInternal()
	  - like ApplyEffectToBeam(), InterpolateComponent() will update, based on the channel's running effect, internal values to manage stuff like gobo shake, calling again settargetvalue on the interpolations, or setting values without interpolation on the component and/or on the interpolation object.
		  This method could be omitted by the user, it's not mandatory to implement it if you don't need to update stuff at each tick
	- After that, InterpolateComponent() will call updateInterpolation() on each interpolation in the interpolations array

	To "send" values to the light in the level, ApplyEffectToBeam(), updateInterpolations(), SetTargetValue() and InterpolateComponent_BeamInternal(), call SetValueNoInterp()
	- SetValueNoInterp() calls, per each attached beam, SetValueNoInterp_BeamInternal()
	  - SetValueNoInterp_BeamInternal() is overridden by the user and takes in input a value and an interpolation id. This function is responsible of setting the material expression's parameters on the input beam.
	      It takes also an interpolation id to know where this value is coming from, for components that control different parameters and have more interpolations objects

	Many functions take an interpolation id, that you should use to tell/tells you wich feature and on which feature you're setting the values
	*/
	
	/**
	 * Initialize the component. It's the first method being called of the Component's life. 
	 * When implementing this method is MANDATORY you do a supercall to this implementation, so it will
	 * automatically load the geometry name, the attribute index, the DMX channel infromations (address, description, channel tree, etc) contained in the "channels" TArray,
	 * and the min/max/default value per each attribute in the channels contained in the attributesData object.
	 * 
	 * Pay attenction: between the Setup() and the BeginPlay() call the object can be unloaded and reloaded.
	 * Check that every attribute you want to preserved is marked with an UPROPERTY, that they're not objects but structs, or primitives
	 * that every inner attribute of the structs are marked as UPROPERTY as well and that you're not using *at all* pointers to other objects.
	 * Otherwise: you'll end up with data loss!!
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param AttachedGeometryName name of the component attached to that geometry
	 * @param DMXChannels list of phisical dmx channels that have at least one attribute related to that component
	 * @param attributeIndex index of the component, per component. This is useful where, EG, you have multiple wheels, blades, etc (Gobo0, Gobo1, Color0, etc). NOTE: this starts by 0!
	 * @return true if everything went well (this could be used to check whenever we have all of the DMX channels
	*/
	virtual bool Setup(FName AttachedGeometryName, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex);
	
	/**
	 * Called when we're starting the emulation. If you implement this method is MANDATORY you do a supercall to one of the BeginPlay() implementations,
	 * so they can initialize the interpolation objects.
	 * Beside initializing the interpolations, usually this method is used to set the first scalar/vector/textur parameters (like the Wheel textures, or the wheel dimensions) to the objects in the level,
	 * generate the names that will be used at runtime inside InterpolateComponent_BeamInternal.
	 * 
	 * Pay attenction: between the Setup() and the BeginPlay() call the object can be unloaded and reloaded.
	 * Check that every attribute you want to preserved is marked with an UPROPERTY, that they're not objects but structs, or primitives
	 * that every inner attribute of the structs are marked as UPROPERTY as well and that you're not using *at all* pointers to other objects.
	 * Otherwise: you'll end up with data loss!!
	 * 
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	virtual void BeginPlay();

protected:
	
	/**
	 * An attribute group is a set of attributes that are used always together (like wheelSpin and slectSpin) or they are used in the same block of code, so they should share the same default/min/max values.
	 * This function is called in the Setup phase to know which are the attributeGroups of this component, so they can be correctly analyzed to get the right default/min/max values
	 *
	 * This should reflect what happens in the ApplyEffectToBeam(): if you have fallback cases in the switch-case (like in the goboComponent), the attributes of that case should be grouped together
	 * @author Luca Sorace - Clay Paky S.R.L.
	 */
	virtual TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups() { TArray<TSet<ECPGDTFAttributeType>> a; return a; };

	/**
	 * If the RealAcceleration is missing from the GDTF this function is called, so you can return a default value
	 * We know that a RealAcceleration is missing if its GDTF value is equals to 0
	 * 
	 * @param channelData data about the channel we're using to create the interpolation. You can get the min/max values (and the range) or the RealFade from that
	 * @param interpolationId id of the interpolation we're creating. You can use that to return different default values for different interpolations
	 * @return default realAcceleration for the specified interpolationId
	 */
	virtual float getDefaultRealAcceleration(FCPDMXChannelData &channelData, int interpolationId) { return DEFAULT_REAL_ACCELERATION; }
	/**
	 * If the RealFade is missing from the GDTF this function is called, so you can return a default value
	 * We know that a RealFade is missing if its GDTF value is equals to 0
	 *
	 * @param channelData data about the channel we're using to create the interpolation. You can get the min/max values (and the range) or the RealAcceleration from that
	 * @param interpolationId id of the interpolation we're creating. You can use that to return different default values for different interpolations
	 * @return default realAcceleration for the specified interpolationId
	 */
	virtual float getDefaultRealFade(FCPDMXChannelData &channelData, int interpolationId) { return DEFAULT_REAL_FADE; }
	/**
	 * Gets the ratio between the RealAcceleration and the RealFade, so it can be used to calculate a defaultAcceleration if it's missing, but the RealFade is present
	 * The ratio is obtained with the formula `RealAcceleration / RealFade`
	 * 
	 * @param realFade RealFade parameter already present in the GDTF file
	 * @param channelData data about the channel we're using to create the interpolation. You can get the min/max values (and the range) or the RealAcceleration from that
	 * @param interpolationId id of the interpolation we're creating. You can use that to return different default values for different interpolations
	 * @return Ratio between RealAcceleration and RealFade
	 */
	virtual float getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) { return getDefaultRealAcceleration(channelData, interpolationId) / getDefaultRealFade(channelData, interpolationId); }
	/**
	 * Gets the ratio between the RealFade and the RealAcceleration, so it can be used to calculate a defaultFade if it's missing, but the RealAcceleration is present
	 * The ratio is obtained with the formula `(RealFade - 2 * RealAcceleration) / RealAcceleration`
	 *
	 * @param realAcceleration RealAcceleration parameter already present in the GDTF file
	 * @param channelData data about the channel we're using to create the interpolation. You can get the min/max values (and the range) or the RealAcceleration from that
	 * @param interpolationId id of the interpolation we're creating. You can use that to return different default values for different interpolations
	 * @return Ratio between RealFade and the RealAcceleration
	 */
	virtual float getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) { float accel = getDefaultRealAcceleration(channelData, interpolationId); return (getDefaultRealFade(channelData, interpolationId) - 2 * accel) / accel; }
	
	/// Read DMXValue and apply the effect to the Beam
	/**
	 * This is the function that's called each time we receive a DMX packet.
	 * Here you should handle the new DMX value you have just received, updating what's the component is doing and setting new target values for the interpolations
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param DMXValue The original dmx value we have receviced
	 * @param channel The channel where we have received the dmx value
	 * @param DMXBehaviour The DMXfunction and its CHannelSet associated with the current DMXvalue
	 * @param AttributeType Parsed attribute associated with the Channel Function
	 * @param physicalValue Translation of the DMXValue to the phisicalValue of the current Channel Function
	 */
	virtual void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) {};

	/// Called each tick when interpolation is enabled, to calculate the next value 
	/**
	 * This function is called each tick to update the component targets based on the current RunningEffects (EG Gobo shake
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param deltaSeconds The seconds passed since the last time this function was called
	 * @param channel The channel we have to update the targets to
	 */
	virtual void InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel);

	//Applies the values to the phisical light using parameters
	/**
	 * This function should take the input value and applies it to the level's component parameters.
	 * It also takes an interpolationId to differentiate different parameters to apply,
	 * in case there are multiple attributes to control at runtime to get this feature working (EG Blades who have blade A and blade Rot)
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param beam BeamSceneComponent to set the parameters to
	 * @param value value to set to the parameters
	 * @param interpolationId Selects which parameter you should apply for components that needs more runtime parameters
	 */
	virtual void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* beam, float value, int interpolationId) {};

};

#undef DEFAULT_REAL_FADE
#undef DEFAULT_REAL_ACCELERATION