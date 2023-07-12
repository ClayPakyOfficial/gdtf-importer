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
#include "UObject/Object.h"
#include "DMXAttribute.h"
#include "DMXTypes.h"
#include "Library/DMXImport.h"
#include "Library/DMXImportGDTF.h"
#include "CPGDTFDescription.generated.h"

/**
 * \defgroup GDTFDesc GDTF Description
 *  @{
 */

namespace CPGDTFDescription {

	template <typename EnumType> static EnumType GetEnumValueFromString(const FString& String)
	{
		return static_cast<EnumType>(StaticEnum<EnumType>()->GetValueByName(FName(*String)));
	}

	static bool IsDigit(TCHAR Char) {
		return (Char >= '0' && Char <= '9');
	}

	static ECPGDTFAttributeType GetGDTFAttributeTypeValueFromString(const FString& String) {

		// We replace the numbers by a '_'
		FString EditedString = String;
		int nIndex = 0;
		for (int i = 0; i < EditedString.Len(); i++) {
			if (CPGDTFDescription::IsDigit(EditedString[i])) {
				EditedString.InsertAt(i, '_');
				EditedString.InsertAt(i, 'n' + nIndex);
				EditedString.InsertAt(i, '_');
				nIndex--;

				for (int j = i + 3; j < EditedString.Len() && CPGDTFDescription::IsDigit(EditedString[j]); j++) {
					EditedString.RemoveAt(j);
				}
				i = FMath::Min(i + 2, EditedString.Len() -1);
			}
		}
		return static_cast<ECPGDTFAttributeType>(StaticEnum<ECPGDTFAttributeType>()->GetValueByName(FName(*EditedString)));
	}

	/**
	 * Parses a GDTF Matrix
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 24 may 2022
	 *
	 * @param MatrixStr String to parse into a matrix
	 * @returns FMatrix
	 */
	CLAYPAKYGDTFIMPORTER_API FMatrix ParseMatrix(FString MatrixStr);
};

UENUM(BlueprintType)
enum class ECPGDTFDescriptionSnap : uint8
{
	Yes = 1,
	No = 0,
	On = 1,
	Off = 0
};


UENUM(/*BlueprintType*/)
enum class ECPGDTFAttributeType : uint16 {
	AnimationSystem_n_,
	AnimationSystem_n_Audio,
	AnimationSystem_n_Macro,
	AnimationSystem_n_Pos,
	AnimationSystem_n_PosAudio,
	AnimationSystem_n_PosRandom,
	AnimationSystem_n_PosRotate,
	AnimationSystem_n_PosShake,
	AnimationSystem_n_Ramp,
	AnimationSystem_n_Random,
	AnimationSystem_n_Shake,
	AnimationSystemReset, ///< ❔
	AnimationWheel_n_,
	AnimationWheel_n_Audio,
	AnimationWheel_n_Macro,
	AnimationWheel_n_Mode, ///< ❔
	AnimationWheel_n_Pos,
	AnimationWheel_n_PosRotate,
	AnimationWheel_n_PosShake,
	AnimationWheel_n_Random,
	AnimationWheel_n_SelectEffects,
	AnimationWheel_n_SelectShake,
	AnimationWheel_n_SelectSpin,
	AnimationWheelShortcutMode, ///< ❔
	BeamEffectIndexRotateMode, ///< ❔
	BeamReset, ///< ❔
	BeamShaper,
	BeamShaperMacro,
	BeamShaperPos,
	BeamShaperPosRotate,
	BlackoutMode, ///< ❔
	Blade_n_A, ///< ✔️
	Blade_n_B, ///< ✔️
	Blade_n_Rot, ///< ✔️
	BladeSoft_n_A, ///< ✔️
	BladeSoft_n_B, ///< ✔️
	Blower_n_, ///< ❔
	ChromaticMode, ///< ❔
	CIE_X, ///< ✔️
	CIE_Y, ///< ✔️
	CIE_Brightness, ///< ✔️
	Color_n_, ///< ✔️
	Color_n_Mode, ///< ❌ Disabled for now
	Color_n_WheelAudio, ///< See ColorWheelComponent header
	Color_n_WheelIndex, ///< ✔️
	Color_n_WheelRandom, ///< ✔️
	Color_n_WheelSpin, ///< ✔️
	ColorAdd_R, ///< ✔️
	ColorAdd_G, ///< ✔️
	ColorAdd_B, ///< ✔️
	ColorAdd_C, ///< ✔️
	ColorAdd_M, ///< ✔️
	ColorAdd_Y, ///< ✔️
	ColorAdd_RY, ///< ✔️
	ColorAdd_GY, ///< ✔️
	ColorAdd_GC, ///< ✔️
	ColorAdd_BC, ///< ✔️
	ColorAdd_BM, ///< ✔️
	ColorAdd_RM, ///< ✔️
	ColorAdd_W,  ///< ✔️
	ColorAdd_WW, ///< ✔️
	ColorAdd_CW, ///< ✔️
	ColorAdd_UV, ///< ✔️
	ColorCalibrationMode, ///< ❔
	ColorConsistency,
	ColorControl,
	ColorEffects_n_,
	ColorMacro_n_, ///< ✔️
	ColorMacro_n_Rate, ///< ❌ Disabled for now
	ColorMixMode, ///< ❔
	ColorMixMSpeed,
	ColorMixReset, ///< ❔
	ColorModelMode, ///< ❔
	ColorRGB_Blue,
	ColorRGB_Cyan,
	ColorRGB_Green,
	ColorRGB_Magenta,
	ColorRGB_Quality,
	ColorRGB_Red,
	ColorRGB_Yellow,
	ColorSettingsReset, ///< ❔
	ColorSub_R, ///< ✔️
	ColorSub_G, ///< ✔️
	ColorSub_B, ///< ✔️
	ColorSub_C, ///< ✔️
	ColorSub_M, ///< ✔️
	ColorSub_Y, ///< ✔️
	ColorUniformity,
	ColorWheelReset, ///< ❔
	ColorWheelSelectMSpeed,
	ColorWheelShortcutMode, ///< ❔
	Control_n_, ///< ❔
	CRIMode, ///< ❔
	CTB, ///< Need to inherit from UCPGDTFColorCorrectionFixtureComponent
	CTBReset, ///< ❔
	CTC, ///< Need to inherit from UCPGDTFColorCorrectionFixtureComponent
	CTCReset, ///< ❔
	CTO, ///< ✔️
	CTOReset, ///< ❔
	CustomColor,
	CyanMode, ///< ❔
	DigitalZoom,
	Dimmer, ///< ✔️
	DimmerCurve, ///< ❔
	DimmerMode, ///< ❔
	DimmerReset, ///< ❔
	DisplayIntensity, ///< ❔
	DMXInput, ///< ❔
	Effects_n_,
	Effects_n_Adjust_m_,
	Effects_n_Fade,
	Effects_n_Pos,
	Effects_n_PosRotate,
	Effects_n_Rate,
	EffectsSync,
	Fan_n_, ///< ❔
	Fan_n_Mode, ///< ❔
	Fans, ///< ❔
	FieldOfView, ///< ❔
	FixtureCalibrationReset, ///< ❔
	FixtureGlobalReset, ///< ❔
	Focus_n_,
	Focus_n_Adjust,
	Focus_n_Distance,
	FocusMode, ///< ❔
	FocusMSpeed,
	FocusReset, ///< ❔
	Fog_n_,
	FollowSpotMode, ///< ❔
	FrameMSpeed,
	FrameReset, ///< ❔
	Frost_n_, ///< ✔️
	Frost_n_MSpeed, ///< ❌ Disabled for now
	Frost_n_PulseClose, ///< ✔️
	Frost_n_PulseOpen, ///< ✔️
	Frost_n_Ramp, ///< ✔️
	Function, ///< ❔
	GlobalMSpeed,
	Gobo_n_, ///< ✔️
	Gobo_n_Pos, ///< ✔️
	Gobo_n_PosRotate, ///< ✔️
	Gobo_n_PosShake, ///< ✔️
	Gobo_n_SelectEffects, ///< ❌ See GoboWheelComponent header
	Gobo_n_SelectShake, ///< ✔️
	Gobo_n_SelectSpin, ///< ✔️
	Gobo_n_WheelAudio, ///< ❌ See GoboWheelComponent header
	Gobo_n_WheelIndex, ///< ✔️
	Gobo_n_WheelMode, ///< ❌ Disabled for now
	Gobo_n_WheelRandom, ///< ✔️
	Gobo_n_WheelShake, ///< ✔️
	Gobo_n_WheelSpin, ///< ✔️
	GoboWheel_n_MSpeed,
	GoboWheelReset, ///< ❔
	GoboWheelShortcutMode, ///< ❔
	Haze_n_,
	HSB_Hue, ///< ✔️
	HSB_Saturation, ///< ✔️
	HSB_Brightness, ///< ✔️
	HSB_Quality,
	InputSource, ///< ❔
	IntensityMSpeed,
	IntensityReset, ///< ❔
	Iris, ///< ✔️
	IrisMode, ///< ❔
	IrisMSpeed,
	IrisPulse, ///< ✔️
	IrisPulseClose, ///< ✔️
	IrisPulseOpen, ///< ✔️
	IrisRandomPulseClose, ///< ❔
	IrisRandomPulseOpen, ///< ❔
	IrisReset, ///< ❔
	IrisStrobe, ///< ❔
	IrisStrobeRandom, ///< ❔
	KeyStone_n_A,
	KeyStone_n_B,
	LampControl, ///< ❔
	LampPowerMode, ///< ❔
	LEDFrequency, ///< ❔
	LEDZoneMode, ///< ❔
	MagentaMode, ///< ❔
	MediaContent_n_, ///< ❔
	MediaFolder_n_, ///< ❔
	ModelContent_n_, ///< ❔
	ModelFolder_n_, ///< ❔
	NoFeature, ///< ✔️ Default GDTF Attribute value. Do nothing
	Pan, ///< ✔️
	PanMode, ///< ❔
	PanReset, ///< ❔
	PanRotate,
	PanTiltMode, ///< ❔
	PixelMode, ///< ❔
	PlayBegin, ///< ❔
	PlayEnd, ///< ❔
	PlayMode, ///< ❔
	PlaySpeed, ///< ❔
	PositionEffect,
	PositionEffectFade,
	PositionEffectRate,
	PositionModes, ///< ❔
	PositionMSpeed,
	PositionReset, ///< ❔
	Prism_n_,
	Prism_n_Macro,
	Prism_n_MSpeed,
	Prism_n_PosRotate,
	Prism_n_SelectSpin,
	ReflectorAdjust,
	Rot_X,
	Rot_Y,
	Rot_Z,
	Scale_X,
	Scale_XYZ,
	Scale_Y,
	Scale_Z,
	ShaperMacros,
	ShaperMacrosSpeed,
	ShaperRot, ///< ✔️
	Shutter_n_, ///< ✔️
	Shutter_n_Strobe, ///< ✔️
	Shutter_n_StrobeEffect,
	Shutter_n_StrobePulse, ///< ✔️
	Shutter_n_StrobePulseClose, ///< ✔️
	Shutter_n_StrobePulseOpen, ///< ✔️
	Shutter_n_StrobeRandom, ///< ✔️
	Shutter_n_StrobeRandomPulse, ///< ✔️
	Shutter_n_StrobeRandomPulseClose, ///< ✔️
	Shutter_n_StrobeRandomPulseOpen, ///< ✔️
	ShutterReset, ///< ❔
	StrobeDuration,
	StrobeFrequency, ///< ✔️
	StrobeMode, ///< ❌ Disabled for now
	StrobeModeEffect, ///< ✔️
	StrobeModePulse, ///< ✔️
	StrobeModePulseClose, ///< ✔️
	StrobeModePulseOpen, ///< ✔️
	StrobeModeRandom, ///< ✔️
	StrobeModeRandomPulse, ///< ✔️
	StrobeModeRandomPulseClose, ///< ✔️
	StrobeModeRandomPulseOpen, ///< ✔️
	StrobeModeShutter, ///< ✔️
	StrobeModeStrobe, ///< ✔️
	StrobeRate,
	Tilt, ///< ✔️
	TiltMode, ///< ❔
	TiltReset, ///< ❔
	TiltRotate,
	Tint, ///< Need to inherit from UCPGDTFColorCorrectionFixtureComponent
	UVStability,
	Video, ///< ❔
	VideoBlendMode, ///< ❔
	VideoBoost_B, ///< ❔
	VideoBoost_G, ///< ❔
	VideoBoost_R, ///< ❔
	VideoBrightness, ///< ❔
	VideoCamera_n_, ///< ❔
	VideoContrast, ///< ❔
	VideoEffect_n_Parameter_m_, ///< ❔
	VideoEffect_n_Type, ///< ❔
	VideoHueShift, ///< ❔
	VideoKeyColor_B, ///< ❔
	VideoKeyColor_G, ///< ❔
	VideoKeyColor_R, ///< ❔
	VideoKeyIntensity, ///< ❔
	VideoKeyTolerance, ///< ❔
	VideoSaturation, ///< ❔
	VideoSoundVolume_n_, ///< ❔
	WavelengthCorrection,
	WhiteCount,
	XYZ_X,
	XYZ_Y,
	XYZ_Z,
	YellowMode, ///< ❔
	Zoom, ///< ✔️
	ZoomMode, ///< ❔
	ZoomModeBeam,
	ZoomModeSpot,
	ZoomMSpeed,
	ZoomReset, ///< ❔
	DIMENSION,
	DefaultValue
};

  /***************************************************/
 /*             Attributes definitions              */
/***************************************************/

/// Extension of FDMXImportGDTFAttribute to add the enum Attribute value
USTRUCT(BlueprintType)
struct FCPGDTFDescriptionAttribute : public FDMXImportGDTFAttribute {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		ECPGDTFAttributeType Type = ECPGDTFAttributeType::NoFeature;
};


  /***************************************************/
 /*             Geometries definitions              */
/***************************************************/

UENUM(BlueprintType)
enum class ECPGDTFDescriptionGeometryBeamType : uint8
{
	Wash,
	Spot,
	None,
	Rectangle,
	PC,
	Fresnel,
	Glow
 
};

UENUM(BlueprintType)
enum class ECPGDTFDescriptionGeometryType : uint8 {
	Geometry,
	Axis,
	FilterBeam,
	FilterColor,
	FilterGobo,
	FilterShaper,
	Beam,
	//MediaServerLayer, // All commented members are defined in GDTF Spec but not supported for now here
	//MediaServerCamera,
	//MediaServerMaster,
	//Display,
	GeometryReference,
			/// TODO \todo ADD LASER SUPPORT
			//Laser,
	//WiringObject,
	//Inventory,
	//Structure,
	//Support,
	//Magnet,
	Other // Not in GDTF Spec but used for Unreal subobjects in FActorGeometryTree
};

/**
 * Base object to regroup all properties in common <br>
 * To make it editable on GDTFDescription UI we should use structs instead of objects (except for UCPGDTFDescriptionGeometries)
 */
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryBase : public UObject {

	GENERATED_BODY()

public:
	UCPGDTFDescriptionGeometryBase() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::Other; // Defined in subobjects
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
	FName Model;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
	FMatrix Position;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		ECPGDTFDescriptionGeometryType Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		TArray<UCPGDTFDescriptionGeometryBase*> Childrens;
};

/// Implementation of Geometry GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometry : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

	UCPGDTFDescriptionGeometry() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::Geometry;
	}
};

/// Implementation of GeometryAxis GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryAxis : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

	UCPGDTFDescriptionGeometryAxis() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::Axis;
	}
};

/// Implementation of GeometryFilterBeam GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryFilterBeam : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

	UCPGDTFDescriptionGeometryFilterBeam() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::FilterBeam;
	}
};

/// Implementation of GeometryFilterColor GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryFilterColor : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

	UCPGDTFDescriptionGeometryFilterColor() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::FilterColor;
	}
};

/// Implementation of GeometryFilterGobo GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryFilterGobo : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

	UCPGDTFDescriptionGeometryFilterGobo() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::FilterGobo;
	}
};

/// Implementation of GeometryFilterShaper GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryFilterShaper : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

	UCPGDTFDescriptionGeometryFilterShaper() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::FilterShaper;
	}
};

/// Implementation of GeometryBeam GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryBeam : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

public:
	UCPGDTFDescriptionGeometryBeam() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::Beam;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		EDMXImportGDTFLampType LampType = EDMXImportGDTFLampType::Discharge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float PowerConsumption = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float LuminousFlux = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float ColorTemperature = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float BeamAngle = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float FieldAngle = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float ThrowRatio = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float RectangleRatio = 1.7777f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float BeamRadius = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		ECPGDTFDescriptionGeometryBeamType BeamType = ECPGDTFDescriptionGeometryBeamType::Wash; // Default value

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		uint8 ColorRenderingIndex = 100;

	//TODO ? Missing EmitterSpectrum compared with GDTF Spec
};

/// Implementation of GeometryReference GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometryReference : public UCPGDTFDescriptionGeometryBase {

	GENERATED_BODY()

public:
	UCPGDTFDescriptionGeometryReference() {
		this->Position = FMatrix::Identity;
		this->Type = ECPGDTFDescriptionGeometryType::GeometryReference;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		FName Geometry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		TArray<FDMXImportGDTFBreak> Breaks;
};

/// Implementation of Geometries GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionGeometries : public UDMXImportGeometries {
	
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		TArray<UCPGDTFDescriptionGeometryBase*> Geometries;


	/**
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 15 june 2022
	 *
	 * @param Type
	 * @return Childrens
	*/
	UFUNCTION(BlueprintCallable)
		TArray<UCPGDTFDescriptionGeometryBase*> GetChildrensOfType(ECPGDTFDescriptionGeometryType Type);

	/**
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 15 june 2022
	 *
	 * @param CurrentNode
	 * @param Type
	 * @return Childrens of specified node
	*/
	UFUNCTION(BlueprintCallable)
		static TArray<UCPGDTFDescriptionGeometryBase*> GetChildrensOfType_STATIC(UCPGDTFDescriptionGeometryBase* CurrentNode, ECPGDTFDescriptionGeometryType Type);

	/**
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 15 june 2022
	 *
	 * @param Type
	 * @return Childrens Names
	*/
	UFUNCTION(BlueprintCallable)
		TArray<FName> GetChildrensNamesOfType(ECPGDTFDescriptionGeometryType Type);

	/**
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 15 june 2022
	 *
	 * @param CurrentNode
	 * @param Type
	 * @return Childrens Names
	*/
	UFUNCTION(BlueprintCallable)
		static TArray<FName> GetChildrensNamesOfType_STATIC(UCPGDTFDescriptionGeometryBase* CurrentNode, ECPGDTFDescriptionGeometryType Type);
};

  /***************************************************/
 /*               Models definitions                */
/***************************************************/

/// Generics 3D models
UENUM(BlueprintType)
enum class ECPGDTFDescriptionModelsPrimitiveType : uint8 {

	Undefined = 0,
	Cube = 1,
	Cylinder = 2,
	Sphere = 3,
	Base = 4,
	Yoke = 5,
	Head = 6,
	Scanner = 7,
	Conventional = 8,
	Pigtail = 9,
	Base1_1 = 10,
	Scanner1_1 = 11,
	Conventional1_1 = 12
};

/// Implementation of Model GDTF node
USTRUCT(BlueprintType)
struct FCPGDTFDescriptionModel {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float Length = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float Width = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		float Height = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		ECPGDTFDescriptionModelsPrimitiveType PrimitiveType = ECPGDTFDescriptionModelsPrimitiveType::Undefined;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		FName File;
};

/// Implementation of Models GDTF node
UCLASS(BlueprintType, Blueprintable)
class UCPGDTFDescriptionModels : public UDMXImportModels {

	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		TArray<FCPGDTFDescriptionModel> Models;
};

  /***************************************************/
 /*                  DMX definition                 */
/***************************************************/

/// Implementation of ChannelFunction GDTF node
USTRUCT(BlueprintType)
struct FCPGDTFDescriptionChannelFunction : public FDMXImportGDTFChannelFunction {

	GENERATED_BODY()

	FCPGDTFDescriptionChannelFunction() {}

	FCPGDTFDescriptionChannelFunction(FDMXImportGDTFChannelFunction PreviousChannelFunction, FDMXImportGDTFDMXValue _DMXTo) {
		this->Name = PreviousChannelFunction.Name;
		this->Attribute = PreviousChannelFunction.Attribute;
		this->OriginalAttribute = PreviousChannelFunction.OriginalAttribute;
		this->DMXFrom = PreviousChannelFunction.DMXFrom;
		this->DMXValue = PreviousChannelFunction.DMXValue;
		this->PhysicalFrom = PreviousChannelFunction.PhysicalFrom;
		this->PhysicalTo = PreviousChannelFunction.PhysicalTo;
		this->RealFade = PreviousChannelFunction.RealFade;
		this->Wheel = PreviousChannelFunction.Wheel;
		this->Emitter = PreviousChannelFunction.Emitter;
		this->Filter = PreviousChannelFunction.Filter;
		this->DMXInvert = PreviousChannelFunction.DMXInvert;
		this->ModeMaster = PreviousChannelFunction.ModeMaster;
		this->ModeFrom = PreviousChannelFunction.ModeFrom;
		this->ModeTo = PreviousChannelFunction.ModeTo;
		this->ChannelSets = PreviousChannelFunction.ChannelSets;
		this->DMXTo = _DMXTo;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		FDMXImportGDTFDMXValue DMXTo;
};

/// Implementation of ChannelSet GDTF node
USTRUCT(BlueprintType)
struct FCPGDTFDescriptionChannelSet: public FDMXImportGDTFChannelSet {
	
	GENERATED_BODY()

	FCPGDTFDescriptionChannelSet() {}

	FCPGDTFDescriptionChannelSet(FDMXImportGDTFChannelSet PreviousChannelSet, FDMXImportGDTFDMXValue _DMXTo) {
		this->Name = PreviousChannelSet.Name;
		this->DMXFrom = PreviousChannelSet.DMXFrom;
		this->PhysicalFrom = PreviousChannelSet.PhysicalFrom;
		this->PhysicalTo = PreviousChannelSet.PhysicalTo;
		this->WheelSlotIndex = PreviousChannelSet.WheelSlotIndex;
		this->DMXTo = _DMXTo;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX");
		FDMXImportGDTFDMXValue DMXTo;
};

  /***************************************************/
 /*              Global obj definition              */
/***************************************************/

/// Top level Object containing the complete GDTF description of a fixture
UCLASS(BlueprintType, Blueprintable)
class CLAYPAKYGDTFIMPORTER_API UCPGDTFDescription : public UDMXImportGDTF {

	GENERATED_BODY()

public:
	template <typename TType> TType* CreateNewObject() {
		FName NewName = MakeUniqueObjectName(this, TType::StaticClass());
		return NewObject<TType>(this, TType::StaticClass(), NewName, RF_Public);
	}

	FString GetFixtureSavePath();
};

/// @}