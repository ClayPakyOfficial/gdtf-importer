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

#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialFunctionInstance.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionTransform.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"

#include "Factories/MaterialFunctionFactoryNew.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Engine/EngineTypes.h"

#include "ClayPakyGDTFImporterLog.h"
#include "CPGDTFDescription.h"
#include "PackageTools.h"
#include "ObjectTools.h"

#include <functional>

#include "Components/CPGDTFBeamSceneComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFShaperFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFIrisFixtureComponent.h"

#include "Factories/Importers/Wheels/CPGDTFWheelImporter.h"

#define __CPGDTFRenderingPipelineBuilder_processME404Error(errorMsg, allOk, keywordName){ \
	allOk = false; \
	errorMsg += TEXT("\nUnable to find BeamRayMarch custom Material Expression. Check if "); \
	errorMsg += keywordName; \
	errorMsg += TEXT(" is in the Desc\n"); \
}

/**
 * Helper Object that will create a custom rendering pipeline cloning and modifying existing material instance
 */
class CLAYPAKYGDTFIMPORTER_API CPGDTFRenderPipelineBuilder
{
private:
	//Tool to automatically place the generated material expression blocks in the material editor
	struct MeBlocksMover {
		private:
			bool movingYAxys = false; //Direction. If true we're moving on the Y axys
			int32 x, y; //x and y coordinates where we should place an ME next time moveMaterialExpression() is called
			int32 defaultOffset; //offset between two different material expressions
		public:
			MeBlocksMover(int32 x, int32 y, int32 defaultOffset, bool movingYAxys) : movingYAxys(movingYAxys), x(x), y(y), defaultOffset(defaultOffset) {}

			void moveMaterialExpression(UMaterialExpression* me, int offset = 0) {
				me->MaterialExpressionEditorX = x;
				me->MaterialExpressionEditorY = y;

				if (offset == 0) offset = defaultOffset;
				if (movingYAxys) y += offset;
					else x += offset;
			}

			void moveMaterialExpressions(TArray <UMaterialExpression*>& mes) {
				for (UMaterialExpression* me : mes)
					moveMaterialExpression(me);
			}

			inline int32 getCurrentX() {
				return x;
			}
			inline int32 getCurrentY() {
				return y;
			}
	};

	//GDTF description of the fixture we're importing
	UCPGDTFDescription* mGdtfDescription;
	//Base path where we're gonna store all the materials & stuff
	FString mBasePackagePath;
	//Sanitized name of the fixture we're importing
	FString mSanitizedName;
	//index of the selected mode/profile inside mGdtfDescription->DMXModes->DMXModes
	int mSelectedMode;

	//Array of wheels of the fixture we're importing
	TArray<FDMXImportGDTFWheel> mWheels;
	//Number of wheels per each WheelType
	int mWheelsNo[FCPGDTFWheelImporter::WheelType::WHEEL_TYPE_SIZE] = {};
	//Shapers that we're importing. Used to obtain info like their orientation and their mode
	TArray<UCPGDTFShaperFixtureComponent *> shapers;
	//If true it means that the fixture we're importing has an iris component
	bool hasIris = false;

	/**
	 * Fully clones a Material Interface and its inner Material from CLAYPAKY_PLUGIN_CONTENT_BASEPATH/MaterialInstances/ to the fixture's Render lightRenderingPipeline folder
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 11 january 2023
	 *
	 * @param materialType String with the name of the material (EG: in the material M_Beam_Master that string should be "Beam")
	 * @param linkOutputFnc function to run to link outputs of the newly created material
	 * @param middleCode function that should edit the newly created material, after it has been fully set up
	 * @return true if everything went well :)
	*/
	bool cloneMaterialInterface(FString materialType, const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& linkOutputFnc, const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& middleCode);

	/**
	 * Builds a new material function containing the rendering pipeline for the fixture we're importing
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 20 december 2022
	*/
	bool buildMaterialInstancePipeline();

	/**
	 * Builds the light's render pipeline generating Custom Expression's C++ code, cloning and updating the Beam's material instance
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 20 december 2022
	*/
	bool buildBeamPipeline();
	/**
	 * Clones the Lens's material instance and changes the rendering function material to the one we've previously created in buildMaterialInstancePipeline()
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 20 december 2022
	*/
	bool linkLensPipeline(const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& middleCode);
	/**
	 * Clones the Light's material instance and changes the rendering function material to the one we've previously created in buildMaterialInstancePipeline()
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 20 december 2022
	*/
	bool linkSpotlightPipeline(const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& middleCode);

	/**
	 * Searches a Material Expression by the given description and the given type
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 22 december 2022
	 *
	 * @tparam CheckType Type of the material expression we're searching
	 * @param arr Array where it should search
	 * @param descr Part of the description we are searching
	 * @return UMaterialExpression of type CheckType which description contains the input descr
	*/
	template <typename CheckType>
	static CheckType* searchMaterialExpressionByDesc(TArray<TObjectPtr<UMaterialExpression>> arr, FString descr);

	/**
	 * Generates a new MaterialExpression automatically setting its inner metadata
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 18 may 2023
	 *
	 * @tparam MEtype Type of the material expression we're generating
	 * @param material MaterialFunction owner of the newly generated MaterialExpression
	 * @return MEtype Generated material expression of the specified type
	 */
	template <typename MEtype>
	MEtype* generateMaterialExpression(UMaterialFunction* material);
	/**
	 * Generates a new MaterialExpression automatically setting its inner metadata
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 18 may 2023
	 *
	 * @tparam MEtype Type of the material expression we're generating
	 * @param material Material owner of the newly generated MaterialExpression
	 * @return MEtype Generated material expression of the specified type
	 */
	template <typename MEtype>
	MEtype* generateMaterialExpression(UMaterial* material);

	/**
	 * Adds a scalar parameter to the beam ray march's material custom expression, generating the parameter material with the input name, adding the variable input to the material custom expression and linking both together
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param beamMaterial Source beam material
	 * @param paramName Param material name
	 * @param inputName Input variable name
	 * @param meCustom Beam ray march's Material custom Expression
	 * @param mover Optional: If != null, we will automatically position the new parameter
	 * @param defaultValue Optional: The default value of the material param. Default: 0
	 * @return The just added scalar parameter
	*/
	UMaterialExpressionScalarParameter* addScalarInputToBeamPipeline(UMaterial* beamMaterial, FString paramName, FString inputName, UMaterialExpressionCustom* meCustom, MeBlocksMover* mover = nullptr, float defaultValue = 0);
	/**
	 * Adds a texture parameter to the beam ray march's material custom expression, generating the parameter material with the input name, adding the variable input to the material custom expression and linking both together
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 24 february 2023
	 *
	 * @param beamMaterial Source beam material
	 * @param paramName Param material name
	 * @param inputName Input variable name
	 * @param meCustom Beam ray march's Material custom Expression
	 * @param defaultTexture The default texture of the material param
	 * @param mover Optional: If != null, we will automatically position the new parameter
	 * @param defaultSamplerType Optional: The default sampler type of the texture. Default: Linear Color
	 * @return The just added texture parameter
	*/
	UMaterialExpressionTextureObjectParameter* addTextureInputToBeamPipeline(UMaterial* beamMaterial, FString paramName, FString inputName, UMaterialExpressionCustom* meCustom, UTexture2D* defaultTexture, MeBlocksMover* mover = nullptr, EMaterialSamplerType defaultSamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor);
	/**
	 * Adds a scalar parameter to the material instance pipeline, generating the parameter material with the input name
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 24 february 2023
	 *
	 * @param beamMaterial Source material
	 * @param paramName Param material name
	 * @param mover Optional: If != null, we will automatically position the new parameter
	 * @param defaultValue Optional: The default value of the material param. Default: 0
	 * @return The just added scalar parameter
	*/
	UMaterialExpressionScalarParameter* addScalarParameterToMiPipeline(UMaterialFunction* beamMaterial, FString paramName, MeBlocksMover* mover = nullptr, float defaultValue = 0);
	/**
	 * Adds a texture parameter to the material instance pipeline, generating the parameter material with the input name
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 24 february 2023
	 *
	 * @param beamMaterial Source material
	 * @param paramName Param material name
	 * @param defaultTexture The default texture of the material param
	 * @param mover Optional: If != null, we will automatically position the new parameter
	 * @param defaultSamplerType Optional: The default sampler type of the texture. Default: Linear Color
	 * @return The just added scalar parameter
	*/
	UMaterialExpressionTextureObjectParameter* addTextureParameterToMiPipeline(UMaterialFunction* beamMaterial, FString paramName, UTexture2D* defaultTexture, MeBlocksMover* mover = nullptr, EMaterialSamplerType defaultSamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor);


	/**
	 * Copies every detail from one UMaterial to another
	 * Everything is copied except: shading model, aset user data, physical material map, previewMesh and asset import data
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 22 december 2022
	 *
	 * @param dstMaterial Destination material
	 * @param srcMaterial Source material
	 * @param shadingModel Since I wasn't able to find a way to get the shadingModel from srcMaterial, you have to manually specify what shadingModel should dstMaterial have
	*/
	void copyUMaterialDetails(UMaterial* dstMaterial, UMaterial* srcMaterial);

public:
	/**
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 20 december 2022
	 *
	 * @param gdtfDescription GDTF description of the fixture we're importing
	 * @param selectedMode Index of the current active dmx mode/profile
	 * @param components Components attached to this light
	 * @param fixturePathOnContentBrowser base path of the fixture in the content brower
	*/
	CPGDTFRenderPipelineBuilder(UCPGDTFDescription* gdtfDescription, int selectedMode, TArray<UActorComponent*> components, FString fixturePathOnContentBrowser);
	~CPGDTFRenderPipelineBuilder();

	/**
	 * Creates custom material instances that will be in charge of creating this fixture's pipeline to render colors/gobos/animations/anything regarding to the "texture" of the light
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 20 december 2022
	 *
	 * @return True if everything OK.
	*/
	bool buildLightRenderPipeline();

	/**
	 * Generates the filename in the fixture's personal render pipeline builder folder
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 13 january 2023
	 *
	 * @param prepend prepend a string to the filename
	 * @param materialType Beam|Lens|Spotlight
	 * @param append append a string to the filename
	 * @param addPath if true the full path to the fixture's personal render pipeline builder folder is prepended
	 * @return The generated path/file name
	*/
	FString getFileName(FString prepend, FString materialType, FString append, bool addPath);
	/**
	 * Generates the filename of a Material in the fixture's personal render pipeline builder folder
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 13 january 2023
	 *
	 * @param materialType Beam|Lens|Spotlight
	 * @param addPath if true the full path to the fixture's personal render pipeline builder folder is prepended
	 * @return The generated path/file name of the Material
	*/
	FString getMaterialFilename(FString materialType, bool addPath);
	/**
	 * Generates the filename of a Material Interface in the fixture's personal render pipeline builder folder
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 13 january 2023
	 *
	 * @param materialType Beam|Lens|Spotlight
	 * @param addPath if true the full path to the fixture's personal render pipeline builder folder is prepended
	 * @return The generated path/file name of the Material Interface
	*/
	FString getMaterialInterfaceFilename(FString materialType, bool addPath);

	/*
		██████   █████  ██████   █████  ███    ███ ███████ 
		██   ██ ██   ██ ██   ██ ██   ██ ████  ████ ██      
		██████  ███████ ██████  ███████ ██ ████ ██ ███████ 
		██      ██   ██ ██   ██ ██   ██ ██  ██  ██      ██ 
		██      ██   ██ ██   ██ ██   ██ ██      ██ ███████
	*/

	/**************************************
	*               GENERAL               *
	**************************************/
	/**
	 * Generates the Scalar parameter for the frost
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 23 february 2023
	 *
	 * @return The generated frost's name
	*/
	static inline FString getFrostParamName() {
		return TEXT("DMX Frost");
	}
	/**
	 * Generates the Scalar parameter for the iris
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 08 june 2023
	 *
	 * @return The generated iris's name
	*/
	static inline FString getIrisParamName() {
		return TEXT("DMX Iris");
	}
	/*************************************
	*               SHAPER               *
	*************************************/
	/**
	 * Generates the Scalar parameter for the blade orientation
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @return The generated blade orientation's name
	*/
	static inline FString getBladeOrientationParamName() {
		return TEXT("DMX Blade Orientation");
	}
	/**
	 * Generates the Scalar parameter for the blade insertion/swivelling
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @param isAParam true if we're moving the A point, false if we're controlling the B point or the swivelling
	 * @return The generated blade values's name
	*/
	static inline FString getBladeABRotParamName(bool isAParam) {
		FString ret = TEXT("DMX Blade ");
		if (isAParam) ret += TEXT("A");
		else ret += TEXT("B/Rot");
		return ret;
	}
	/*************************************
	*               WHEELS               *
	*************************************/
	/**
	 * Generates the Scalar parameter texture's name based on the wheel type and if the wheel is frosted or not
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param frosted true if the wheel is frosted
	 * @return The generated wheel texture's name
	*/
	static inline FString getDiskParamName(FCPGDTFWheelImporter::WheelType wheelType, bool frosted) {
		FString ret = TEXT("DMX ") + FCPGDTFWheelImporter::wheelTypeToString(wheelType) + TEXT(" Disk");
		if (frosted) ret += TEXT(" Frosted"); //This should be a temporary hack, we're gonna try rendering the frost inside the pipeline
		return ret;
	}
	/**
	 * Generates the Scalar parameter wheel rotation's name based on the wheel type
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @return The generated wheel rotation's name
	*/
	static inline FString getRotationParamName(FCPGDTFWheelImporter::WheelType wheelType) {
		return TEXT("DMX ") + FCPGDTFWheelImporter::wheelTypeToString(wheelType) + TEXT(" Wheel Rotation");
	}
	/**
	 * Generates the Scalar parameter numSlots's name based on the wheel type
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @return The generated wheel numSlots' name
	*/
	static inline FString getNumSlotsParamName(FCPGDTFWheelImporter::WheelType wheelType) {
		return TEXT("DMX ") + FCPGDTFWheelImporter::wheelTypeToString(wheelType) + TEXT(" Wheel Num Mask");
	}
	/**
	 * Generates the Scalar parameter wheel index's name based on the wheel type
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @return The generated wheel index's name
	*/
	static inline FString getIndexParamName(FCPGDTFWheelImporter::WheelType wheelType) {
		return TEXT("DMX ") + FCPGDTFWheelImporter::wheelTypeToString(wheelType) + TEXT(" Wheel Index");
	}


	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ████████████████    Automatic id    ████████████████
	// ████████████████    Automatic id    ████████████████
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

	/*************************************
	*               SHAPER               *
	*************************************/
	/**
	 * Generates the Scalar parameter for the blade orientation based on the blade number/orientation
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @param bladeNo the number of the blade (equals to its orientation + 1)
	 * @return The generated blade orientation's name
	*/
	static inline FString getBladeOrientationParamName(int bladeNo) {
		return getBladeOrientationParamName() + CPGDTFRenderPipelineBuilder::getParamNameFromId(bladeNo);
	}
	/**
	 * Generates the Scalar parameter for the blade insertion/swivelling based on the blade number/orientation
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @param isAParam true if we're moving the A point, false if we're controlling the B point or the swivelling
	 * @param bladeNo the number of the blade (equals to its orientation + 1)
	 * @return The generated blade values's name
	*/
	static inline FString getBladeABRotParamName(bool isAParam, int bladeNo) {
		return getBladeABRotParamName(isAParam) + CPGDTFRenderPipelineBuilder::getParamNameFromId(bladeNo);
	}
	/*************************************
	*               WHEELS               *
	*************************************/
	/**
	 * Generates the Scalar parameter texture's name based on the wheel type and if the wheel is frosted or not and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param frosted true if the wheel is frosted
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel texture's name
	*/
	static inline FString getDiskParamName(FCPGDTFWheelImporter::WheelType wheelType, bool frosted, int wheelNo) {
		FString ret = getDiskParamName(wheelType, frosted) + CPGDTFRenderPipelineBuilder::getParamNameFromId(wheelNo);
		return ret;
	}
	/**
	 * Generates the Scalar parameter wheel rotation's name based on the wheel type and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel rotation's name
	*/
	static inline FString getRotationParamName(FCPGDTFWheelImporter::WheelType wheelType, int wheelNo) {
		FString ret = getRotationParamName(wheelType) + CPGDTFRenderPipelineBuilder::getParamNameFromId(wheelNo);
		return ret;
	}
	/**
	 * Generates the Scalar parameter numSlots's name based on the wheel type and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel numSlots' name
	*/
	static inline FString getNumSlotsParamName(FCPGDTFWheelImporter::WheelType wheelType, int wheelNo) {
		FString ret = getNumSlotsParamName(wheelType) + CPGDTFRenderPipelineBuilder::getParamNameFromId(wheelNo);
		return ret;
	}
	/**
	 * Generates the Scalar parameter wheel index's name based on the wheel type and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel index's name
	*/
	static inline FString getIndexParamName(FCPGDTFWheelImporter::WheelType wheelType, int wheelNo) {
		FString ret = getIndexParamName(wheelType) + CPGDTFRenderPipelineBuilder::getParamNameFromId(wheelNo);
		return ret;
	}

	/*
		██    ██  █████  ██████  ██  █████  ██████  ██      ███████ ███████ 
		██    ██ ██   ██ ██   ██ ██ ██   ██ ██   ██ ██      ██      ██      
		██    ██ ███████ ██████  ██ ███████ ██████  ██      █████   ███████ 
		 ██  ██  ██   ██ ██   ██ ██ ██   ██ ██   ██ ██      ██           ██ 
		  ████   ██   ██ ██   ██ ██ ██   ██ ██████  ███████ ███████ ███████ 
	*/

	/**************************************
	*               GENERAL               *
	**************************************/
	/**
	 * Generates the frost variable name/custom material expression input name
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 23 february 2023
	 *
	 * @return The generated frost's variable name
	*/
	static inline FString getInputFrostName() {
		return TEXT("frost");
	}
	/**
	 * Generates the iris variable name/custom material expression input name
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 08 june 2023
	 *
	 * @return The generated iris's variable name
	*/
	static inline FString getInputIrisName() {
		return TEXT("irisVal");
	}
	/*************************************
	*               SHAPER               *
	*************************************/
	/**
	 * Generates the blade orientation variable name/custom material expression input name
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @return The generated blade orientation's variable name
	*/
	static inline FString getInputBladeOrientationName() {
		return TEXT("bladeOrientationRads");
	}
	/**
	 * Generates the blade insertion/swivelling variable name/custom material expression input name
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @param isAParam true if we're moving the A point, false if we're controlling the B point or the swivelling
	 * @return The generated blade value's variable name
	*/
	static inline FString getInputBladeABRotName(bool isAParam) {
		FString ret = TEXT("blade");
		if (isAParam) ret += TEXT("A");
		else ret += TEXT("BRot");
		return ret;
	}
	/*************************************
	*               WHEELS               *
	*************************************/
	/**
	 * Generates the texture variable name/custom material expression input name based on the wheel type and if the wheel is frosted or not
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param frosted true if the wheel is frosted
	 * @return The generated wheel texture variable name
	*/
	static inline FString getInputTextureName(FCPGDTFWheelImporter::WheelType wheelType, bool frosted) {
		FString ret = TEXT("TXTp") + FCPGDTFWheelImporter::wheelTypeToString(wheelType);
		if (frosted) ret += TEXT("_Frosted"); //This should be a temporary hack, we're gonna try rendering the frost inside the pipeline
		return ret;
	}
	/**
	 * Generates the wheel rotation variable/custom material expression input name name based on the wheel type
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @return The generated wheel rotation name
	*/
	static inline FString getInputRotationName(FCPGDTFWheelImporter::WheelType wheelType) {
		return FCPGDTFWheelImporter::wheelTypeToString(wheelType) + TEXT("RotationIndex");
	}
	/**
	 * Generates the wheel numSlots variable/custom material expression input name name based on the wheel type
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @return The generated wheel numSlots name
	*/
	static inline FString getInputNumSlotsName(FCPGDTFWheelImporter::WheelType wheelType) {
		return TEXT("Num") + FCPGDTFWheelImporter::wheelTypeToString(wheelType);
	}
	/**
	 * Generates the wheel index variable/custom material expression input name name based on the wheel type
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @return The generated wheel index name
	*/
	static inline FString getInputIndexName(FCPGDTFWheelImporter::WheelType wheelType) {
		return FCPGDTFWheelImporter::wheelTypeToString(wheelType) + TEXT("Index");
	}


	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ████████████████    Automatic id    ████████████████
	// ████████████████    Automatic id    ████████████████
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

	/*************************************
	*               SHAPER               *
	*************************************/
	/**
	 * Generates the blade orientation variable name/custom material expression input name based on the blade number/orientation
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @param bladeNo the number of the blade (equals to its orientation + 1)
	 * @return The generated blade orientation's variable name
	*/
	static inline FString getInputBladeOrientationName(int bladeNo) {
		return getInputBladeOrientationName() + CPGDTFRenderPipelineBuilder::getInputNameFromId(bladeNo);
	}
	/**
	 * Generates the blade insertion/swivelling variable name/custom material expression input name
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 30 january 2023
	 *
	 * @param isAParam true if we're moving the A point, false if we're controlling the B point or the swivelling
	 * @return The generated blade value's variable name
	*/
	static inline FString getInputBladeABRotName(bool isAParam, int bladeNo) {
		return getInputBladeABRotName(isAParam) + CPGDTFRenderPipelineBuilder::getInputNameFromId(bladeNo);
	}
	/*************************************
	*               WHEELS               *
	*************************************/
	/**
	 * Generates the texture variable name/custom material expression input name based on the wheel type, if the wheel is frosted or not and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param frosted true if the wheel is frosted
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel texture variable name
	*/
	static inline FString getInputTextureName(FCPGDTFWheelImporter::WheelType wheelType, bool frosted, int wheelNo) {
		return getInputTextureName(wheelType, frosted) + CPGDTFRenderPipelineBuilder::getInputNameFromId(wheelNo);
	}
	/**
	 * Generates the wheel rotation variable/custom material expression input name name based on the wheel type and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel rotation name
	*/
	static inline FString getInputRotationName(FCPGDTFWheelImporter::WheelType wheelType, int wheelNo) {
		return getInputRotationName(wheelType) + CPGDTFRenderPipelineBuilder::getInputNameFromId(wheelNo);
	}
	/**
	 * Generates the wheel numSlots variable/custom material expression input name name based on the wheel type and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel numSlots name
	*/
	static inline FString getInputNumSlotsName(FCPGDTFWheelImporter::WheelType wheelType, int wheelNo) {
		return getInputNumSlotsName(wheelType) + CPGDTFRenderPipelineBuilder::getInputNameFromId(wheelNo);
	}
	/**
	 * Generates the wheel index variable/custom material expression input name name based on the wheel type and the wheel number
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Wheel type
	 * @param wheelNo number of wheel per wheelType (EG: Gobo0, Gobo1, Color0, etc)
	 * @return The generated wheel index name
	*/
	static inline FString getInputIndexName(FCPGDTFWheelImporter::WheelType wheelType, int wheelNo) {
		return getInputIndexName(wheelType) + CPGDTFRenderPipelineBuilder::getInputNameFromId(wheelNo);
	}

	/*
		██    ██ ████████ ██ ██      ██ ████████ ██ ███████ ███████ 
		██    ██    ██    ██ ██      ██    ██    ██ ██      ██      
		██    ██    ██    ██ ██      ██    ██    ██ █████   ███████ 
		██    ██    ██    ██ ██      ██    ██    ██ ██           ██ 
		 ██████     ██    ██ ███████ ██    ██    ██ ███████ ███████ 
	*/

	/**************************************
	*               GENERAL               *
	**************************************/

	/**
	 * Generates the string to append to a scalar parameter name containing the attribute number (EG: Gobo 0, Gobo 1, Shaper 0, etc)
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param attributeNo number of attribute per type
	 * @return The generated string
	*/
	static inline FString getParamNameFromId(int attributeNo) {
		return TEXT(" #") + FString::FromInt(attributeNo);
	}
	/**
	 * Generates the string to append to a variable name/custom material expression input name containing the attribute number (EG: Gobo 0, Gobo 1, Shaper 0, etc)
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param attributeNo number of attribute per type
	 * @return The generated string
	*/
	static inline FString getInputNameFromId(int attributeNo) {
		return TEXT("__") + FString::FromInt(attributeNo);
	}

	//Horizontal dimension of the animation wheel. This is an hack to use the same function of the gobo/color wheel to sample the animation wheel
	static const int ANIMATION_WHEEL_NSLOT = 16;
};
