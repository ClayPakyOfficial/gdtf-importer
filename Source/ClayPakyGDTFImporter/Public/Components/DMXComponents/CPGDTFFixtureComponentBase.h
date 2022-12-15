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

#pragma once

#include "CoreMinimal.h"
#include "ClayPakyGDTFImporterLog.h"
#include "CPGDTFDescription.h"
#include "CPGDTFFixtureActor.h"
#include "CPGDTFInterpolation.h"
#include "CPGDTFFixtureComponentBase.generated.h"

/// Struct used to describe the basic DMXChannels properties
USTRUCT(BlueprintType)
struct FCPDMXChannelData
{
	GENERATED_BODY()

	FCPDMXChannelData() {
		this->Address = -1;
		this->MinValue = 0.0f;
		this->MaxValue = 1.0f;
		this->DefaultValue = 0.0f;
	}

	FCPDMXChannelData(FDMXImportGDTFDMXChannel Channel) {
		
		if (Channel.Offset.Num() > 0) this->Address = FMath::Max(1, FMath::Min(512, Channel.Offset[0]));
		else this->Address = -1;
		this->MinValue = Channel.LogicalChannels[0].ChannelFunctions[0].PhysicalFrom;
		FDMXImportGDTFLogicalChannel LastLogicalChannel = Channel.LogicalChannels[Channel.LogicalChannels.Num() - 1];
		FDMXImportGDTFChannelFunction LastChannelFunction = LastLogicalChannel.ChannelFunctions[LastLogicalChannel.ChannelFunctions.Num() - 1];
		this->MaxValue = LastChannelFunction.PhysicalTo;
		this->DefaultValue = 0.0f;
	}

	UPROPERTY(EditAnywhere, Category = "DMX Channel", meta=(ClampMin="0", ClampMax="512"))
	int32 Address = -1;

	UPROPERTY(BlueprintReadOnly, Category = "DMX Channel")
	float MinValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DMX Channel")
	float MaxValue = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DMX Channel")
	float DefaultValue = 0.0f;

	bool IsAddressValid() { return this->Address != -1; }
};

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
class CLAYPAKYGDTFIMPORTER_API UCPGDTFFixtureComponentBase : public UActorComponent
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
		FName AttachedGeometryName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
		USceneComponent* AttachedGeometry;

	/// Enable the PushDMXRawValues() method if we need no normalized values
	UPROPERTY()
		bool bIsRawDMXEnabled;

public:
	UCPGDTFFixtureComponentBase();

	///virtual void BeginPlay() { };
	virtual void OnConstruction() {}; // Initializes the component on spawn on a world

	void Setup(FName AttachedGeometryName);

	/// Value changes smaller than this threshold are ignored 
	UPROPERTY(EditAnywhere, Category = "DMX Parameters")
		float SkipThreshold = 0.003f;

	/// If attached to a DMX Fixture Actor, returns the parent fixture actor. 
	UFUNCTION(BlueprintCallable, Category = "DMX")
		class ACPGDTFFixtureActor* GetParentFixtureActor();

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/// Pushes DMX Values to the Component. Expects normalized values in the range of 0.0f - 1.0f
	UFUNCTION(BlueprintCallable, Category = "DMX Component")
		virtual void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap);

	UFUNCTION(BlueprintCallable, Category = "DMX Component")
		virtual void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap) {};

	  /*******************************************/
	 /*          Interpolation Related          */
	/*******************************************/
	
	/// If used within a DMX Fixture Actor or Fixture Matrix Actor, the plugin interpolates towards the last set value. 
	UPROPERTY(EditAnywhere, Category = "DMX Parameters")
		bool bUseInterpolation = true;

	/// The scale of the interpolation speed. Faster when > 1, slower when < 1 
	UPROPERTY(EditAnywhere, Category = "DMX Parameters")
		float InterpolationScale = 1.0f;

	/// Called each tick when interpolation is enabled, to calculate the next value 
	UFUNCTION(BlueprintCallable, Category = "DMX")
		virtual void InterpolateComponent(float DeltaSeconds) {};

	/// True if the target value is reached and no interpolation is required
	UFUNCTION(BlueprintCallable, Category = "DMX")
		virtual bool IsDMXInterpolationDone() const { return true; };

	/// Applies the speed scale property
	UFUNCTION(BlueprintCallable, Category = "DMX")
		virtual void ApplySpeedScale() {};
};
