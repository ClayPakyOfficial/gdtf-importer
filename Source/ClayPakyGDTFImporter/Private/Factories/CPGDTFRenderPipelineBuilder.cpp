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

//The code here was written using the following tutorial:
// https://isaratech.com/ue4-programmatically-create-a-new-material-and-inner-nodes/

#include "Factories/CPGDTFRenderPipelineBuilder.h"
#include "Factories/CPGDTFBeamHlslGenerator.h"

#define FIND_DESCRIPTION_BASE "__RENDER_PIPELINE_BUILDER"
#define FIND_DESCR_INPUT TEXT("__INPUT"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_CUSTOM TEXT("__CUSTOM"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_NORMAL TEXT("__NORMAL"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_OPACITY TEXT("__OPACITY"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_METALLIC TEXT("__METALLIC"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_BASECOLOR TEXT("__BASECOLOR"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_ROUGHNESS TEXT("__ROUGHNESS"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_CUSTOM_FROST TEXT("__CST_FROST"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_EMISSIVECOLOR TEXT("__EMISSIVECOLOR"  FIND_DESCRIPTION_BASE)
#define FIND_DESCR_WORLDPOSITIONOFFSET TEXT("__WORLDPOSITIONOFFSET"  FIND_DESCRIPTION_BASE)


#define MATERIAL_TYPE_BEAM TEXT("Beam")
#define MATERIAL_TYPE_LENS TEXT("Lens")
#define MATERIAL_TYPE_LIGHT TEXT("Light")

#define BEAM_EDITOR_STARTING_X -768
#define BEAM_EDITOR_STARTING_Y 1200

#define EDITOR_MINI_BLOCK_SIZE 16
#define EDITOR_BLOCK_SIZE 128
#define BEAM_EDITOR_BLOCK_DISTANCE_X EDITOR_BLOCK_SIZE * 2
#define MI_EDITOR_MI_BLOCK_DISTANCE_X EDITOR_BLOCK_SIZE * 6
#define MI_EDITOR_PARAM_BLOCK_DISTANCE_X EDITOR_BLOCK_SIZE * 3
#define MI_EDITOR_PARAM_BLOCK_DISTANCE_Y EDITOR_BLOCK_SIZE * 1

#define MI_INIT_BLOCKS_MOVER MeBlocksMover* meParamsMover = new MeBlocksMover(meModulesMover->getCurrentX() - MI_EDITOR_PARAM_BLOCK_DISTANCE_X, meModulesMover->getCurrentY() + MI_EDITOR_PARAM_BLOCK_DISTANCE_Y, 0, true);

#define EDITOR_TEXTURE_OBJECT_SIZE_Y 192
#define EDITOR_SCALAR_PARAM_SIZE_Y 96
#define EDITOR_CONSTANT_SIZE_Y 80
#define EDITOR_VECTOR4_SIZE_Y 208


/**
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 20 december 2022
 *
 * @param gdtfDescription GDTF description of the fixture we're importing
 * @param selectedMode Index of the current active dmx mode/profile
 * @param components Components attached to this light
 * @param fixturePathOnContentBrowser base path of the fixture in the content brower
 */
CPGDTFRenderPipelineBuilder::CPGDTFRenderPipelineBuilder(UCPGDTFDescription* gdtfDescription, int selectedMode, TArray<UActorComponent*> components, FString fixturePathOnContentBrowser) {
	UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: Instancing"));
	this->mSelectedMode = selectedMode;
	this->mGdtfDescription = gdtfDescription;
	this->mWheels = Cast<UDMXImportGDTFWheels>(gdtfDescription->Wheels)->Wheels;
	this->mSanitizedName = UPackageTools::SanitizePackageName(Cast<UDMXImportGDTFFixtureType>(gdtfDescription->FixtureType)->Name.ToString());

	if (fixturePathOnContentBrowser.EndsWith("/")) fixturePathOnContentBrowser.RemoveAt(fixturePathOnContentBrowser.Len() - 1);
	fixturePathOnContentBrowser.Append("/lightRenderingPipeline/");
	fixturePathOnContentBrowser.Append(FString::FromInt(selectedMode));
	fixturePathOnContentBrowser.Append("/");
	this->mBasePackagePath = fixturePathOnContentBrowser;
	UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: name='%s', pathMaterial='%s'"), *this->mSanitizedName, *this->mBasePackagePath);

	for (FDMXImportGDTFWheel wheel : this->mWheels) {
		FCPGDTFWheelImporter::WheelType wType = FCPGDTFWheelImporter::GetWheelType(this->mGdtfDescription, wheel.Name, selectedMode);
		this->mWheelsNo[wType]++;
	}

	for (int i = 0; i < components.Num(); i++) {
		UCPGDTFShaperFixtureComponent *shaper = Cast<UCPGDTFShaperFixtureComponent>(components[i]);
		if (shaper != nullptr) { shapers.Add(shaper); continue; }
		UCPGDTFIrisFixtureComponent *iris = Cast<UCPGDTFIrisFixtureComponent>(components[i]);
		if (iris) { hasIris = true; continue; }
	}
}

CPGDTFRenderPipelineBuilder::~CPGDTFRenderPipelineBuilder() {}

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
bool CPGDTFRenderPipelineBuilder::cloneMaterialInterface(FString materialType, const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& linkOutputFnc, const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& middleCode) {
	FString errorMsg = "";
	bool allOk = true;

	materialType = TEXT("_") + materialType;
	FString mName = getMaterialFilename(materialType, false);
	FString miName = getMaterialInterfaceFilename(materialType, false);

	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: Running cloneMaterialInterface with mName='%s'"), *mName);

	//Create an empty destination material
	UPackage* materialPackage = CreatePackage(*(this->mBasePackagePath + mName));
	auto materialFactory = NewObject<UMaterialFactoryNew>();
	UMaterial* dstMaterial = (UMaterial*)materialFactory->FactoryCreateNew(UMaterial::StaticClass(), materialPackage, *mName, RF_Standalone | RF_Public, NULL, GWarn);
	UMaterialEditorOnlyData* dstMaterialData = dstMaterial->GetEditorOnlyData();

	FAssetRegistryModule::AssetCreated(dstMaterial);
	materialPackage->FullyLoad();
	materialPackage->SetDirtyFlag(true);
	dstMaterial->PreEditChange(NULL);

	//Loads the master/source material
	FString masterMaterialPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	masterMaterialPath = masterMaterialPath + TEXT("MaterialInstances/Master/M") + materialType + TEXT("_Master.M") + materialType + TEXT("_Master");
	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: Material path: '%s'"), *masterMaterialPath);
	UMaterial* srcMaterial = Cast<UMaterial>(FCPGDTFImporterUtils::LoadObjectByPath(masterMaterialPath));

	//Copy the material expression nodes
	TArray<TObjectPtr<UMaterialExpression>> dstExpression;
	TArray<TObjectPtr<UMaterialExpression>> dstExpressionComment;
	TArray<TObjectPtr<UMaterialExpression>> srcExpression = srcMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions;
	TArray<TObjectPtr<UMaterialExpressionComment>> srcExpressionComment = srcMaterial->GetEditorOnlyData()->ExpressionCollection.EditorComments;
	UMaterialExpression::CopyMaterialExpressions(srcExpression, srcExpressionComment, dstMaterial, nullptr, dstExpression, dstExpressionComment);
	//Link the nodes to output
	if(linkOutputFnc) allOk &= linkOutputFnc(dstMaterial, dstMaterialData, dstExpression);
	//Update material propetries
	copyUMaterialDetails(dstMaterial, srcMaterial);

	//Run middle code
	if(middleCode) allOk &= middleCode(dstMaterial, dstMaterialData, dstExpression);

	//We need to run these anyway
	dstMaterial->PostEditChange();
	dstMaterial->MarkPackageDirty();
	dstMaterial->GetOuter()->MarkPackageDirty();
	FGlobalComponentReregisterContext recreateComponents; //Is this really useful?

	if (allOk) {
		//Create a new empty material instance
		UPackage* materialInstancePackage = CreatePackage(*(this->mBasePackagePath + miName));
		UMaterialInstanceConstant* materialInstance = NewObject<UMaterialInstanceConstant>(materialInstancePackage, UMaterialInstanceConstant::StaticClass(), *miName, RF_Standalone | RF_Public);

		FAssetRegistryModule::AssetCreated(materialInstance);
		materialInstancePackage->FullyLoad();
		materialInstancePackage->SetDirtyFlag(true);

		//Set the parent as the material we've just created
		materialInstance->Parent = dstMaterial;

		materialInstance->PreEditChange(NULL);
		materialInstance->PostEditChange();
		materialInstancePackage->MarkPackageDirty();

		FGlobalComponentReregisterContext recreateComponents2; //Is this really useful? pt2

	}
	return allOk;
}

/**
 * Builds the light's render pipeline generating Custom Expression's C++ code, cloning and updating the Beam's material instance
 * 
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 20 december 2022
*/
bool CPGDTFRenderPipelineBuilder::buildBeamPipeline() {
	return cloneMaterialInterface(MATERIAL_TYPE_BEAM,

		[](UMaterial* dstMaterial, UMaterialEditorOnlyData* dstMaterialData, TArray<TObjectPtr<UMaterialExpression>> dstExpression) {
			bool allOk = true;
			//Link the nodes to output
			UMaterialExpressionTransform* meWorldPositionOffset = searchMaterialExpressionByDesc<UMaterialExpressionTransform>(dstExpression, FIND_DESCR_WORLDPOSITIONOFFSET);
			UMaterialExpressionMultiply* meEmissiveColor = searchMaterialExpressionByDesc<UMaterialExpressionMultiply>(dstExpression, FIND_DESCR_EMISSIVECOLOR);
			if (meWorldPositionOffset) {
				dstMaterialData->WorldPositionOffset.Connect(0, meWorldPositionOffset);
			} else { allOk = false; UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: buildBeamPipeline > FIND_DESCR_WORLDPOSITIONOFFSET not found")); }
			if (meEmissiveColor) {
				dstMaterialData->EmissiveColor.Connect(0, meEmissiveColor);
			} else { allOk = false;  UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: buildBeamPipeline > FIND_DESCR_EMISSIVECOLOR not found")); }

			return allOk;
		},

		[&](UMaterial* dstMaterial, UMaterialEditorOnlyData* dstMaterialData, TArray<TObjectPtr<UMaterialExpression>> dstExpression) {
			UMaterialExpressionCustom* meCustom = searchMaterialExpressionByDesc<UMaterialExpressionCustom>(dstExpression, FIND_DESCR_CUSTOM);
			if (meCustom) {
				MeBlocksMover* meBlocksMover = new MeBlocksMover(BEAM_EDITOR_STARTING_X, BEAM_EDITOR_STARTING_Y, 0, true);

				//Update the code of the custom block
				//Generate custom hsls code with the correct features pipelined
				CPGDTFBeamHlslGenerator beamHlslGenerator = CPGDTFBeamHlslGenerator();
				beamHlslGenerator.setWheels(this->mWheelsNo);
				beamHlslGenerator.setShapers(this->shapers);
				beamHlslGenerator.setIris(this->hasIris);
				FString finalCode = beamHlslGenerator.generateCode();
				meCustom->Code = finalCode;

				/*************************************
				*               GENERAL              *
				*************************************/

				UMaterialExpressionScalarParameter* scalarFrost = addScalarInputToBeamPipeline(dstMaterial, getFrostParamName(), getInputFrostName(), meCustom, meBlocksMover);

				if (this->hasIris)
					addScalarInputToBeamPipeline(dstMaterial, getIrisParamName(), getInputIrisName(), meCustom, meBlocksMover);

				/*************************************
				*               WHEELS               *
				*************************************/

				FString defaultTexturePath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
				defaultTexturePath += TEXT("MaterialInstances/Textures/T_Circle_01.T_Circle_01");
				UTexture2D* defaultTexture = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(defaultTexturePath));
				//Add parameters and inputs to the custom block
				for (int i = 0; i < FCPGDTFWheelImporter::WheelType::WHEEL_TYPE_SIZE; i++) {
					FCPGDTFWheelImporter::WheelType wheelType = (FCPGDTFWheelImporter::WheelType) i;

					for (int j = 0; j < this->mWheelsNo[wheelType]; j++) {

						if (wheelType == FCPGDTFWheelImporter::WheelType::Animation || wheelType == FCPGDTFWheelImporter::WheelType::Prism || wheelType == FCPGDTFWheelImporter::WheelType::Effects) continue;

						//Add gobo/prism rotation parameter
						if ((wheelType == FCPGDTFWheelImporter::WheelType::Gobo && false /*TODO: check if the current gobo wheel has rotation*/) || wheelType == FCPGDTFWheelImporter::WheelType::Prism) {
							addScalarInputToBeamPipeline(dstMaterial, getRotationParamName(wheelType, j), getInputRotationName(wheelType, j), meCustom, meBlocksMover);
						}

						//Add gobo/color/animation number and index parameters
						if (wheelType == FCPGDTFWheelImporter::WheelType::Color || wheelType == FCPGDTFWheelImporter::WheelType::Gobo || wheelType == FCPGDTFWheelImporter::WheelType::Animation) { //TODO Implement a real animation wheel
							if(wheelType != FCPGDTFWheelImporter::WheelType::Animation)
								addScalarInputToBeamPipeline(dstMaterial, getNumSlotsParamName(wheelType, j), getInputNumSlotsName(wheelType, j), meCustom, meBlocksMover, 1);
							addScalarInputToBeamPipeline(dstMaterial, getIndexParamName(wheelType, j), getInputIndexName(wheelType, j), meCustom, meBlocksMover);
						}

						//Add texture parameter
						bool frosted = wheelType == FCPGDTFWheelImporter::WheelType::Gobo || wheelType == FCPGDTFWheelImporter::WheelType::Animation;
						addTextureInputToBeamPipeline(dstMaterial, getDiskParamName(wheelType, frosted, j), getInputTextureName(wheelType, frosted, j), meCustom, defaultTexture, meBlocksMover);
					}
				}
				
				/*************************************
				*               SHAPER               *
				*************************************/
				FString mfShaperConvertBladePositionPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
				mfShaperConvertBladePositionPath += TEXT("MaterialInstances/MaterialFunctions/shaper/MF_Shaper_ConvertBladePosition.MF_Shaper_ConvertBladePosition");
				UMaterialFunctionInterface* mfShaperConvertBladePosition = Cast<UMaterialFunctionInterface>(FCPGDTFImporterUtils::LoadObjectByPath(mfShaperConvertBladePositionPath));
				FString mfShaperConvertRotPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
				mfShaperConvertRotPath += TEXT("MaterialInstances/MaterialFunctions/shaper/MF_Shaper_ConvertRot.MF_Shaper_ConvertRot");
				UMaterialFunctionInterface* mfShaperConvertRot = Cast<UMaterialFunctionInterface>(FCPGDTFImporterUtils::LoadObjectByPath(mfShaperConvertRotPath));

				const std::function<void(FString, FString, bool)> addABRotParameterToBeamPipeline = [&](FString paramName, FString variableName, bool isRot) {
					UMaterialExpressionScalarParameter* param = generateMaterialExpression<UMaterialExpressionScalarParameter>(dstMaterial);
					param->ParameterName = FName(*paramName);
					param->Group = FName(TEXT("Runtime Parameters"));
					param->SortPriority = 5;
					param->DefaultValue = 0;
					dstMaterial->GetEditorOnlyData()->ExpressionCollection.AddExpression(param);

					UMaterialExpressionMaterialFunctionCall* mfShaperConvertBladePositionCall = generateMaterialExpression<UMaterialExpressionMaterialFunctionCall>(dstMaterial);
					dstMaterial->GetEditorOnlyData()->ExpressionCollection.AddExpression(mfShaperConvertBladePositionCall);
					if (isRot) {
						mfShaperConvertBladePositionCall->SetMaterialFunction(mfShaperConvertRot);
					} else {
						mfShaperConvertBladePositionCall->SetMaterialFunction(mfShaperConvertBladePosition);
						mfShaperConvertBladePositionCall->FunctionInputs[1].Input.Connect(0, scalarFrost); //Frost
					}
					mfShaperConvertBladePositionCall->FunctionInputs[0].Input.Connect(0, param); //Value

					mfShaperConvertBladePositionCall->MaterialExpressionEditorX = meBlocksMover->getCurrentX() + BEAM_EDITOR_BLOCK_DISTANCE_X;
					mfShaperConvertBladePositionCall->MaterialExpressionEditorY = meBlocksMover->getCurrentY() + EDITOR_MINI_BLOCK_SIZE;
					meBlocksMover->moveMaterialExpression(param, EDITOR_SCALAR_PARAM_SIZE_Y);

					FCustomInput input;
					input.InputName = FName(*variableName);
					input.Input.Connect(0, mfShaperConvertBladePositionCall);
					meCustom->Inputs.Add(input);
				};
				for (UCPGDTFShaperFixtureComponent *shaper : shapers) {
					int orientation = shaper->getOrientation();

					addABRotParameterToBeamPipeline(getBladeABRotParamName(true, orientation), getInputBladeABRotName(true, orientation), false);
					addABRotParameterToBeamPipeline(getBladeABRotParamName(false, orientation), getInputBladeABRotName(false, orientation), !shaper->isInAbMode());
					addScalarInputToBeamPipeline(dstMaterial, getBladeOrientationParamName(orientation), getInputBladeOrientationName(orientation), meCustom, meBlocksMover);
				}

				delete meBlocksMover;
				return true;
			} else {
				UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: buildBeamPipeline > FIND_DESCR_CUSTOM not found\n"));
				return false;
			}
		}
	);
}



/**
 * Builds a new material function containing the rendering pipeline for the fixture we're importing
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 20 december 2022
*/
bool CPGDTFRenderPipelineBuilder::buildMaterialInstancePipeline() {
	FString errorMsg = "";
	bool allOk = true;

	FString name = this->mSanitizedName + TEXT("_RenderPipeline");
	FString mfName = TEXT("MF_") + name;
	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: Running buildMaterialInstancePipeline with mfName='%s' and basePath='%s'"), *mfName, *this->mBasePackagePath);

	MeBlocksMover* meModulesMover = new MeBlocksMover(0, 0, MI_EDITOR_MI_BLOCK_DISTANCE_X, false);
	
	//Creates an empty destination material function
	UPackage* materialPackage = CreatePackage(*(this->mBasePackagePath + mfName));
	auto materialFactory = NewObject<UMaterialFunctionFactoryNew>();
	UMaterialFunction* dstMaterial = (UMaterialFunction*)materialFactory->FactoryCreateNew(UMaterialFunction::StaticClass(), materialPackage, *mfName, RF_Standalone | RF_Public, NULL, GWarn);
	UMaterialFunctionEditorOnlyData* dstMaterialData = dstMaterial->GetEditorOnlyData();

	FAssetRegistryModule::AssetCreated(dstMaterial);
	materialPackage->FullyLoad();
	materialPackage->SetDirtyFlag(true);
	dstMaterial->PreEditChange(NULL);

	//Loads the material function for the various components:
	//Iris
	FString mfDmxIrisPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	mfDmxIrisPath += TEXT("MaterialInstances/MaterialFunctions/MF_Iris.MF_Iris");
	UMaterialFunctionInterface* mfIris = Cast<UMaterialFunctionInterface>(FCPGDTFImporterUtils::LoadObjectByPath(mfDmxIrisPath));
	//Gobo/Color/Animation wheel
	FString mfDmxWheelPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	mfDmxWheelPath += TEXT("MaterialInstances/MaterialFunctions/MF_DMXWheel.MF_DMXWheel");
	UMaterialFunctionInterface* mfWheel = Cast<UMaterialFunctionInterface>(FCPGDTFImporterUtils::LoadObjectByPath(mfDmxWheelPath));
	//Shaper A+B
	FString mfShaperABPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	mfShaperABPath += TEXT("MaterialInstances/MaterialFunctions/shaper/MF_Shaper_SampleBladeAB.MF_Shaper_SampleBladeAB");
	UMaterialFunctionInterface* mfShaperAB = Cast<UMaterialFunctionInterface>(FCPGDTFImporterUtils::LoadObjectByPath(mfShaperABPath));
	//Shaper A+Rot
	FString mfShaperARotPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	mfShaperARotPath += TEXT("MaterialInstances/MaterialFunctions/shaper/MF_Shaper_SampleBladeARot.MF_Shaper_SampleBladeARot");
	UMaterialFunctionInterface* mfShaperARot = Cast<UMaterialFunctionInterface>(FCPGDTFImporterUtils::LoadObjectByPath(mfShaperARotPath));

	//Creates a new output node
	UMaterialExpressionFunctionOutput* output = generateMaterialExpression<UMaterialExpressionFunctionOutput>(dstMaterial);
	output->bRealtimePreview = false;
	output->ConditionallyGenerateId(false); //Needed! Not having this will cause the output node not being connected after an unreal engine reload!
	dstMaterialData->ExpressionCollection.AddExpression(output);

	//Creates the "starting" node of the pipeline, that's gonna be the input dmx color
	UMaterialExpressionVectorParameter* input = generateMaterialExpression<UMaterialExpressionVectorParameter>(dstMaterial);
	input->ParameterName = FName(TEXT("DMX Color"));
	input->DefaultValue = { 1, 1, 1, 0 };
	input->Group = FName(TEXT("Runtime Parameters"));
	input->Function = dstMaterial;

	UMaterialExpression* previousParameter = input;
	dstMaterialData->ExpressionCollection.AddExpression(previousParameter);

	//Generates the input frost parameter material expression
	UMaterialExpressionScalarParameter* frostParam = addScalarParameterToMiPipeline(dstMaterial, getFrostParamName());

	//Moves the starting parameter in the material editor
	frostParam->MaterialExpressionEditorX = meModulesMover->getCurrentX() - MI_EDITOR_PARAM_BLOCK_DISTANCE_X;
	frostParam->MaterialExpressionEditorY = meModulesMover->getCurrentY() + EDITOR_MINI_BLOCK_SIZE * 2;
	meModulesMover->moveMaterialExpression(previousParameter);

	FString defaultTexturePath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	defaultTexturePath += TEXT("MaterialInstances/Textures/T_Circle_01.T_Circle_01");
	UTexture2D* defaultTexture = Cast<UTexture2D>(FCPGDTFImporterUtils::LoadObjectByPath(defaultTexturePath));


	//Adds the correct module to the pipeline per each component
	/*************************************
	*               GENERAL              *
	*************************************/
	if (this->hasIris) {
		MI_INIT_BLOCKS_MOVER
		UMaterialExpressionMaterialFunctionCall* currentNode = generateMaterialExpression<UMaterialExpressionMaterialFunctionCall>(dstMaterial);
		dstMaterialData->ExpressionCollection.AddExpression(currentNode);
		currentNode->SetMaterialFunction(mfIris);

		UMaterialExpressionScalarParameter* irisVal = addScalarParameterToMiPipeline(dstMaterial, getIrisParamName(), meParamsMover);

		currentNode->FunctionInputs[1].Input.Connect(0, previousParameter);	//Previous value
		currentNode->FunctionInputs[2].Input.Connect(0, frostParam);		//Frost
		currentNode->FunctionInputs[3].Input.Connect(0, irisVal);			//IrisVal

		meModulesMover->moveMaterialExpression(currentNode);
		previousParameter = currentNode;
		delete meParamsMover;
	}

	/*************************************
	*               WHEELS               *
	*************************************/
	for (int i = 0; i < FCPGDTFWheelImporter::WheelType::WHEEL_TYPE_SIZE; i++) {
		FCPGDTFWheelImporter::WheelType wheelType = (FCPGDTFWheelImporter::WheelType)i;

		for (int j = 0; j < this->mWheelsNo[wheelType]; j++) {
			MI_INIT_BLOCKS_MOVER
			switch (wheelType) {
				case FCPGDTFWheelImporter::WheelType::Color:
				case FCPGDTFWheelImporter::WheelType::Gobo:
				case FCPGDTFWheelImporter::WheelType::Animation:
				{
					if (wheelType == FCPGDTFWheelImporter::WheelType::Animation) break; //TODO Implement a real animation wheel
					//Creates a new material function call node
					UMaterialExpressionMaterialFunctionCall* currentNode = generateMaterialExpression<UMaterialExpressionMaterialFunctionCall>(dstMaterial);
					dstMaterialData->ExpressionCollection.AddExpression(currentNode);
					currentNode->SetMaterialFunction(mfWheel); //Set the material function's function     (function [function]), function function function. Function? Function function! Functioooon :( Fu.... FUNCTION!! :DDD


					//Generates the input parameters material expression
					//Texture disk
					UMaterialExpressionTextureObjectParameter* texParam = addTextureParameterToMiPipeline(dstMaterial, getDiskParamName(wheelType, false, j), defaultTexture, meParamsMover);
					//Texture disk Frosted
					UMaterialExpressionTextureObjectParameter* texFrostedParam = addTextureParameterToMiPipeline(dstMaterial, getDiskParamName(wheelType, true, j), defaultTexture, meParamsMover);
					//Num slots
					UMaterialExpression* numSlotParam;
					if (wheelType == FCPGDTFWheelImporter::WheelType::Animation) {
						UMaterialExpressionConstant* numSlot = generateMaterialExpression<UMaterialExpressionConstant>(dstMaterial);
						numSlot->R = CPGDTFRenderPipelineBuilder::ANIMATION_WHEEL_NSLOT;
						dstMaterialData->ExpressionCollection.AddExpression(numSlot);
						numSlotParam = numSlot;
						meParamsMover->moveMaterialExpression(numSlot, EDITOR_CONSTANT_SIZE_Y);
					} else {
						numSlotParam = addScalarParameterToMiPipeline(dstMaterial, getNumSlotsParamName(wheelType, j), meParamsMover, 1);
					}
					//Wheel index
					UMaterialExpressionScalarParameter* wheelIndexParam = addScalarParameterToMiPipeline(dstMaterial, getIndexParamName(wheelType, j), meParamsMover);

					//Links the inputs to the material function
					currentNode->FunctionInputs[1].Input.Connect(0, previousParameter);	//Previous value
					currentNode->FunctionInputs[2].Input.Connect(0, frostParam);		//Frost
					currentNode->FunctionInputs[3].Input.Connect(0, texParam);			//Texture disk
					currentNode->FunctionInputs[4].Input.Connect(0, texFrostedParam);	//Texture disk frosted
					currentNode->FunctionInputs[5].Input.Connect(0, numSlotParam);		//Num slots
					currentNode->FunctionInputs[6].Input.Connect(0, wheelIndexParam);	//Wheel index

					meModulesMover->moveMaterialExpression(currentNode);
					previousParameter = currentNode;
					break;
				}
				case FCPGDTFWheelImporter::WheelType::Prism:
				case FCPGDTFWheelImporter::WheelType::Effects:
					break;
			}
			delete meParamsMover;
		}
	}
	
	/*************************************
	*               SHAPER               *
	*************************************/
	for (UCPGDTFShaperFixtureComponent* shaper : shapers) {
		MI_INIT_BLOCKS_MOVER
		int orientation = shaper->getOrientation();
		UMaterialExpressionMaterialFunctionCall* currentNode = generateMaterialExpression<UMaterialExpressionMaterialFunctionCall>(dstMaterial);
		dstMaterialData->ExpressionCollection.AddExpression(currentNode);

		if (shaper->isInAbMode()) currentNode->SetMaterialFunction(mfShaperAB);
			else currentNode->SetMaterialFunction(mfShaperARot);

		UMaterialExpressionScalarParameter* aParam = addScalarParameterToMiPipeline(dstMaterial, getBladeABRotParamName(true, orientation), meParamsMover);
		UMaterialExpressionScalarParameter* bRotParam = addScalarParameterToMiPipeline(dstMaterial, getBladeABRotParamName(false, orientation), meParamsMover);
		UMaterialExpressionScalarParameter* orientationParam = addScalarParameterToMiPipeline(dstMaterial, getBladeOrientationParamName(orientation), meParamsMover);

		currentNode->FunctionInputs[1].Input.Connect(0, previousParameter);	//Previous value
		currentNode->FunctionInputs[2].Input.Connect(0, frostParam);		//Frost
		currentNode->FunctionInputs[3].Input.Connect(0, aParam);			//A
		currentNode->FunctionInputs[4].Input.Connect(0, bRotParam);			//B / Rot
		currentNode->FunctionInputs[5].Input.Connect(0, orientationParam);	//Orientation

		meModulesMover->moveMaterialExpression(currentNode);
		previousParameter = currentNode;
		delete meParamsMover;
	}

	//Link the last module we've added to the material function's output
	output->A.Connect(0, previousParameter);
	meModulesMover->moveMaterialExpression(output);

	dstMaterial->UpdateInputOutputTypes();

	dstMaterial->PostEditChange();
	FGlobalComponentReregisterContext recreateComponents; //Is this really useful?

	delete meModulesMover;
	return true;
}

/**
 * Clones the Lens's material instance and changes the rendering function material to the one we've previously created in buildMaterialInstancePipeline()
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 20 december 2022
*/
bool CPGDTFRenderPipelineBuilder::linkLensPipeline(const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& middleCode) {
	return cloneMaterialInterface(MATERIAL_TYPE_LENS,
		[](UMaterial* dstMaterial, UMaterialEditorOnlyData* dstMaterialData, TArray<TObjectPtr<UMaterialExpression>> dstExpression) {
			bool allOk = true;
			//Link the nodes to output
			UMaterialExpressionTextureSampleParameter2D* meBaseColor = searchMaterialExpressionByDesc<UMaterialExpressionTextureSampleParameter2D>(dstExpression, FIND_DESCR_BASECOLOR);
			UMaterialExpressionOneMinus* meMetallic = searchMaterialExpressionByDesc<UMaterialExpressionOneMinus>(dstExpression, FIND_DESCR_METALLIC);
			UMaterialExpressionAdd* meRoughness = searchMaterialExpressionByDesc<UMaterialExpressionAdd>(dstExpression, FIND_DESCR_ROUGHNESS);
			UMaterialExpressionAdd* meEmissiveColor = searchMaterialExpressionByDesc<UMaterialExpressionAdd>(dstExpression, FIND_DESCR_EMISSIVECOLOR);
			UMaterialExpressionConstant* meOpacity = searchMaterialExpressionByDesc<UMaterialExpressionConstant>(dstExpression, FIND_DESCR_OPACITY);
			UMaterialExpressionTextureSampleParameter2D* meNormal = searchMaterialExpressionByDesc<UMaterialExpressionTextureSampleParameter2D>(dstExpression, FIND_DESCR_NORMAL);

			if (meBaseColor) {
				dstMaterialData->BaseColor.Connect(0, meBaseColor);
			} else { allOk = false; UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: linkLensPipeline > FIND_DESCR_BASECOLOR not found\n")); }
			if (meMetallic) {
				dstMaterialData->Metallic.Connect(0, meMetallic);
			} else { allOk = false; UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: linkLensPipeline > FIND_DESCR_METALLIC not found\n")); }
			if (meRoughness) {
				dstMaterialData->Roughness.Connect(0, meRoughness);
			} else { allOk = false; UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: linkLensPipeline > FIND_DESCR_ROUGHNESS not found\n")); }
			if (meEmissiveColor) {
				dstMaterialData->EmissiveColor.Connect(0, meEmissiveColor);
			} else { allOk = false; UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: linkLensPipeline > FIND_DESCR_EMISSIVECOLOR not found\n")); }
			if (meOpacity) {
				dstMaterialData->Opacity.Connect(0, meOpacity);
			} else { allOk = false; UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: linkLensPipeline > FIND_DESCR_BASECOLOR not found\n")); }
			if (meNormal) {
				dstMaterialData->Normal.Connect(0, meNormal);
			} else { allOk = false; UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: linkLensPipeline > FIND_DESCR_BASECOLOR not found\n")); }


			return allOk;
		},
		middleCode
	);
}
/**
 * Clones the Light's material instance and changes the rendering function material to the one we've previously created in buildMaterialInstancePipeline()
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 20 december 2022
*/
bool CPGDTFRenderPipelineBuilder::linkSpotlightPipeline(const std::function<bool(UMaterial*, UMaterialEditorOnlyData*, TArray<TObjectPtr<UMaterialExpression>>)>& middleCode) {
	return cloneMaterialInterface(MATERIAL_TYPE_LIGHT,
		[](UMaterial* dstMaterial, UMaterialEditorOnlyData* dstMaterialData, TArray<TObjectPtr<UMaterialExpression>> dstExpression) {
			//Link the nodes to output
			UMaterialExpressionMultiply* meEmissiveColor = searchMaterialExpressionByDesc<UMaterialExpressionMultiply>(dstExpression, FIND_DESCR_EMISSIVECOLOR);
			if (meEmissiveColor) {
				dstMaterialData->EmissiveColor.Connect(0, meEmissiveColor);
				return true;
			} else UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: linkSpotlightPipeline > FIND_DESCR_EMISSIVECOLOR not found\n"));
			return false;
		},
		middleCode
	);
}

/**
 * Creates custom material instances that will be in charge of creating this fixture's pipeline to render colors/gobos/animations/anything regarding to the "texture" of the light
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 20 december 2022
 *
 * @return True if everything OK.
*/
bool CPGDTFRenderPipelineBuilder::buildLightRenderPipeline() {
	auto middleCode = [&](UMaterial* dstMaterial, UMaterialEditorOnlyData* dstMaterialData, TArray<TObjectPtr<UMaterialExpression>> dstExpression) {
		UMaterialExpressionMultiply* meInput = searchMaterialExpressionByDesc<UMaterialExpressionMultiply>(dstExpression, FIND_DESCR_INPUT);
		if (meInput) {
			FString name = this->mSanitizedName + TEXT("_RenderPipeline");
			FString mfName = TEXT("MF_") + name;

			//FString masterMaterialPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
			//masterMaterialPath = masterMaterialPath + TEXT("MaterialInstances/MaterialFunctions/MF_DMXDimmer.MF_DMXDimmer");

			//Loads the master/source material
			UMaterialFunctionInterface* materialFunction = Cast<UMaterialFunctionInterface>(FCPGDTFImporterUtils::LoadObjectByPath(*(this->mBasePackagePath + mfName + TEXT(".") + mfName)));

			//Adds material function 
			UMaterialExpressionMaterialFunctionCall* mfCall = generateMaterialExpression<UMaterialExpressionMaterialFunctionCall>(dstMaterial);
			mfCall->Material = dstMaterial;
			mfCall->SetMaterialFunction(materialFunction);
			dstMaterialData->ExpressionCollection.AddExpression(mfCall);

			//Links it to the first multiply operation
			meInput->A.Connect(0, mfCall);

			return true;
		} else UE_LOG_CPGDTFIMPORTER(Warning, TEXT("CPGDTFRenderPipelineBuilder: buildLightRenderPipeline > FIND_DESCR_INPUT not found\n"));
		return false;
	};

	bool ret = this->buildBeamPipeline() && this->buildMaterialInstancePipeline()  && this->linkLensPipeline(middleCode) && this->linkSpotlightPipeline(middleCode);
	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: buildLightRenderPipeline > Exiting with result: %d\n"), ret);
	return ret;
}

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
CheckType* CPGDTFRenderPipelineBuilder::searchMaterialExpressionByDesc(TArray<TObjectPtr<UMaterialExpression>> arr, FString descr) {
	for (TObjectPtr<UMaterialExpression> mePtr : arr) {
		UMaterialExpression* me = mePtr.Get();
		if (me->Desc.Contains(descr)) {
			CheckType* casted = Cast<CheckType>(me);
			if (casted) return casted;
		}
	}
	return nullptr;
}

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
MEtype* CPGDTFRenderPipelineBuilder::generateMaterialExpression(UMaterialFunction* material) {
	MEtype* obj = NewObject<MEtype>(material);
	obj->Function = material;
	obj->UpdateMaterialExpressionGuid(false, true);
	return obj;
}
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
MEtype* CPGDTFRenderPipelineBuilder::generateMaterialExpression(UMaterial* material) {
	MEtype* obj = NewObject<MEtype>(material);
	obj->Material = material;
	obj->UpdateMaterialExpressionGuid(false, true);
	return obj;
}

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
*/
UMaterialExpressionScalarParameter* CPGDTFRenderPipelineBuilder::addScalarInputToBeamPipeline(UMaterial* beamMaterial, FString paramName, FString inputName, UMaterialExpressionCustom* meCustom, MeBlocksMover* mover, float defaultValue) {
	UMaterialExpressionScalarParameter* param = generateMaterialExpression<UMaterialExpressionScalarParameter>(beamMaterial);
	param->ParameterName = FName(*paramName);
	param->Group = FName(TEXT("Runtime Parameters"));
	param->SortPriority = 5;
	param->DefaultValue = defaultValue;
	beamMaterial->GetEditorOnlyData()->ExpressionCollection.AddExpression(param);

	FCustomInput input;
	input.InputName = FName(*inputName);
	input.Input.Connect(0, param);
	meCustom->Inputs.Add(input);

	if(mover) mover->moveMaterialExpression(param, EDITOR_SCALAR_PARAM_SIZE_Y);
	return param;
}
/**
	 * Adds a texture parameter to the beam ray march's material custom expression, generating the parameter material with the input name, adding the variable input to the material custom expression and linking both together
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
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
UMaterialExpressionTextureObjectParameter* CPGDTFRenderPipelineBuilder::addTextureInputToBeamPipeline(UMaterial* beamMaterial, FString paramName, FString inputName, UMaterialExpressionCustom* meCustom, UTexture2D* defaultTexture, MeBlocksMover* mover, EMaterialSamplerType defaultSamplerType) {
	UMaterialExpressionTextureObjectParameter* texParam = generateMaterialExpression<UMaterialExpressionTextureObjectParameter>(beamMaterial);
	texParam->ParameterName = FName(*paramName);
	texParam->Group = FName(TEXT("Runtime Parameters"));
	texParam->SortPriority = 35;
	texParam->Texture = defaultTexture;
	texParam->SamplerType = defaultSamplerType;
	beamMaterial->GetEditorOnlyData()->ExpressionCollection.AddExpression(texParam);

	FCustomInput input; // = new FCustomInput;
	input.InputName = FName(*inputName);
	input.Input.Connect(0, texParam);
	meCustom->Inputs.Add(input);

	if (mover) mover->moveMaterialExpression(texParam, EDITOR_TEXTURE_OBJECT_SIZE_Y);
	return texParam;
}
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
UMaterialExpressionScalarParameter* CPGDTFRenderPipelineBuilder::addScalarParameterToMiPipeline(UMaterialFunction* beamMaterial, FString paramName, MeBlocksMover* mover, float defaultValue) {
	UMaterialExpressionScalarParameter* param = generateMaterialExpression<UMaterialExpressionScalarParameter>(beamMaterial);
	param->ParameterName = FName(*paramName);
	param->Group = FName(TEXT("Runtime Parameters"));
	param->SortPriority = 5;
	param->DefaultValue = defaultValue;
	beamMaterial->GetEditorOnlyData()->ExpressionCollection.AddExpression(param);

	if (mover) mover->moveMaterialExpression(param, EDITOR_SCALAR_PARAM_SIZE_Y);
	return param;
}
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
UMaterialExpressionTextureObjectParameter* CPGDTFRenderPipelineBuilder::addTextureParameterToMiPipeline(UMaterialFunction* beamMaterial, FString paramName, UTexture2D* defaultTexture, MeBlocksMover* mover, EMaterialSamplerType defaultSamplerType) {
	UMaterialExpressionTextureObjectParameter* texParam = generateMaterialExpression<UMaterialExpressionTextureObjectParameter>(beamMaterial);
	texParam->ParameterName = FName(*paramName);
	texParam->Group = FName(TEXT("Runtime Parameters"));
	texParam->SortPriority = 35;
	texParam->Texture = defaultTexture;
	texParam->SamplerType = defaultSamplerType;
	beamMaterial->GetEditorOnlyData()->ExpressionCollection.AddExpression(texParam);

	if (mover) mover->moveMaterialExpression(texParam, EDITOR_TEXTURE_OBJECT_SIZE_Y);
	return texParam;
}


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
FString CPGDTFRenderPipelineBuilder::getFileName(FString prepend, FString materialType, FString append, bool addPath) {
	FString _materialType = materialType;
	if (!_materialType.StartsWith(TEXT("_"))) _materialType = TEXT("_") + _materialType;
	FString name = prepend + this->mSanitizedName + TEXT("_LRP_") + _materialType + append;
	if (addPath) name = this->mBasePackagePath + name + TEXT(".") + name; //TODO test this
	return name;
}
/**
 * Generates the filename of a Material in the fixture's personal render pipeline builder folder
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 13 january 2023
 *
 * @param materialType Beam|Lens|Spotlight
 * @param addPath if true the full path to the fixture's personal render pipeline builder folder is prepended
 * @return The generated path/file name of the Material
*/
FString CPGDTFRenderPipelineBuilder::getMaterialFilename(FString materialType, bool addPath) {
	return getFileName(TEXT("M_"), materialType, TEXT("_MASTER"), addPath);
}
/**
 * Generates the filename of a Material Interface in the fixture's personal render pipeline builder folder
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 13 january 2023
 *
 * @param materialType Beam|Lens|Spotlight
 * @param addPath if true the full path to the fixture's personal render pipeline builder folder is prepended
 * @return The generated path/file name of the Material Interface
*/
FString CPGDTFRenderPipelineBuilder::getMaterialInterfaceFilename(FString materialType, bool addPath) {
	return getFileName(TEXT("MI_"), materialType, TEXT(""), addPath);
}


/**
 * Copies every detail from one UMaterial to another
 * Everything is copied except: shading model, aset user data, physical material map, previewMesh and asset import data
 * SetShadingModel is copied by checking which shading models are supported by the srcMaterial, since apparently there's no way to get the current shadingModel from a material, so if the srcMaterial supports more shading models this parameter may be bad
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 22 december 2022
 *
 * @param dstMaterial Destination material
 * @param srcMaterial Source material
*/
void CPGDTFRenderPipelineBuilder::copyUMaterialDetails(UMaterial* dstMaterial, UMaterial* srcMaterial) {
	//this is pure smelly shit. But I couldn't find a better way to copy it :(
	dstMaterial->MaterialDomain = srcMaterial->MaterialDomain;
	dstMaterial->BlendMode = srcMaterial->BlendMode;
	dstMaterial->SetShadingModel(srcMaterial->GetShadingModels().GetFirstShadingModel());
	dstMaterial->TwoSided = srcMaterial->TwoSided;
	dstMaterial->bUseMaterialAttributes = srcMaterial->bUseMaterialAttributes;
	dstMaterial->bCastRayTracedShadows = srcMaterial->bCastRayTracedShadows;
	dstMaterial->SubsurfaceProfile = srcMaterial->SubsurfaceProfile;
	dstMaterial->MaterialDecalResponse = srcMaterial->MaterialDecalResponse;
	dstMaterial->bCastDynamicShadowAsMasked = srcMaterial->bCastDynamicShadowAsMasked;
	dstMaterial->OpacityMaskClipValue = srcMaterial->OpacityMaskClipValue;
	dstMaterial->DitheredLODTransition = srcMaterial->DitheredLODTransition;
	dstMaterial->DitherOpacityMask = srcMaterial->DitherOpacityMask;
	dstMaterial->bAllowNegativeEmissiveColor = srcMaterial->bAllowNegativeEmissiveColor;
	dstMaterial->NumCustomizedUVs = srcMaterial->NumCustomizedUVs;
	dstMaterial->bGenerateSphericalParticleNormals = srcMaterial->bGenerateSphericalParticleNormals;
	dstMaterial->bTangentSpaceNormal = srcMaterial->bTangentSpaceNormal;
	dstMaterial->bUseEmissiveForDynamicAreaLighting = srcMaterial->bUseEmissiveForDynamicAreaLighting;
	dstMaterial->bFullyRough = srcMaterial->bFullyRough;
	dstMaterial->bNormalCurvatureToRoughness = srcMaterial->bNormalCurvatureToRoughness;
	dstMaterial->Wireframe = srcMaterial->Wireframe;
	dstMaterial->ShadingRate = srcMaterial->ShadingRate;
	dstMaterial->bIsSky = srcMaterial->bIsSky;
	dstMaterial->StrataBlendMode = srcMaterial->StrataBlendMode;
	dstMaterial->bScreenSpaceReflections = srcMaterial->bScreenSpaceReflections;
	dstMaterial->bContactShadows = srcMaterial->bContactShadows;
	dstMaterial->TranslucencyLightingMode = srcMaterial->TranslucencyLightingMode;
	dstMaterial->TranslucencyDirectionalLightingIntensity = srcMaterial->TranslucencyDirectionalLightingIntensity;
	dstMaterial->bUseTranslucencyVertexFog = srcMaterial->bUseTranslucencyVertexFog;
	dstMaterial->bApplyCloudFogging = srcMaterial->bApplyCloudFogging;
	dstMaterial->bComputeFogPerPixel = srcMaterial->bComputeFogPerPixel;
	dstMaterial->bOutputTranslucentVelocity = srcMaterial->bOutputTranslucentVelocity;
	dstMaterial->bEnableResponsiveAA = srcMaterial->bEnableResponsiveAA;
	dstMaterial->TranslucencyPass = srcMaterial->TranslucencyPass;
	dstMaterial->bEnableMobileSeparateTranslucency = srcMaterial->bEnableMobileSeparateTranslucency;
	dstMaterial->bDisableDepthTest = srcMaterial->bDisableDepthTest;
	dstMaterial->bWriteOnlyAlpha = srcMaterial->bWriteOnlyAlpha;
	dstMaterial->AllowTranslucentCustomDepthWrites = srcMaterial->AllowTranslucentCustomDepthWrites;
	dstMaterial->TranslucentShadowDensityScale = srcMaterial->TranslucentShadowDensityScale;
	dstMaterial->TranslucentSelfShadowDensityScale = srcMaterial->TranslucentSelfShadowDensityScale;
	dstMaterial->TranslucentSelfShadowSecondDensityScale = srcMaterial->TranslucentSelfShadowSecondDensityScale;
	dstMaterial->TranslucentSelfShadowSecondOpacity = srcMaterial->TranslucentSelfShadowSecondOpacity;
	dstMaterial->TranslucentBackscatteringExponent = srcMaterial->TranslucentBackscatteringExponent;
	dstMaterial->TranslucentMultipleScatteringExtinction = srcMaterial->TranslucentMultipleScatteringExtinction;
	dstMaterial->TranslucentShadowStartOffset = srcMaterial->TranslucentShadowStartOffset;
	dstMaterial->bUsedWithSkeletalMesh = srcMaterial->bUsedWithSkeletalMesh;
	dstMaterial->bUsedWithEditorCompositing = srcMaterial->bUsedWithEditorCompositing;
	dstMaterial->bUsedWithParticleSprites = srcMaterial->bUsedWithParticleSprites;
	dstMaterial->bUsedWithBeamTrails = srcMaterial->bUsedWithBeamTrails;
	dstMaterial->bUsedWithMeshParticles = srcMaterial->bUsedWithMeshParticles;
	dstMaterial->bUsedWithNiagaraSprites = srcMaterial->bUsedWithNiagaraSprites;
	dstMaterial->bUsedWithNiagaraRibbons = srcMaterial->bUsedWithNiagaraRibbons;
	dstMaterial->bUsedWithNiagaraMeshParticles = srcMaterial->bUsedWithNiagaraMeshParticles;
	dstMaterial->bUsedWithGeometryCache = srcMaterial->bUsedWithGeometryCache;
	dstMaterial->bUsedWithStaticLighting = srcMaterial->bUsedWithStaticLighting;
	dstMaterial->bUsedWithMorphTargets = srcMaterial->bUsedWithMorphTargets;
	dstMaterial->bUsedWithSplineMeshes = srcMaterial->bUsedWithSplineMeshes;
	dstMaterial->bUsedWithInstancedStaticMeshes = srcMaterial->bUsedWithInstancedStaticMeshes;
	dstMaterial->bUsedWithGeometryCollections = srcMaterial->bUsedWithGeometryCollections;
	dstMaterial->bUsedWithClothing = srcMaterial->bUsedWithClothing;
	dstMaterial->bUsedWithWater = srcMaterial->bUsedWithWater;
	dstMaterial->bUsedWithHairStrands = srcMaterial->bUsedWithHairStrands;
	dstMaterial->bUsedWithLidarPointCloud = srcMaterial->bUsedWithLidarPointCloud;
	dstMaterial->bUsedWithVirtualHeightfieldMesh = srcMaterial->bUsedWithVirtualHeightfieldMesh;
	dstMaterial->bUsedWithNanite = srcMaterial->bUsedWithNanite;
	dstMaterial->bAutomaticallySetUsageInEditor = srcMaterial->bAutomaticallySetUsageInEditor;
	dstMaterial->FloatPrecisionMode = srcMaterial->FloatPrecisionMode;
	dstMaterial->bUseLightmapDirectionality = srcMaterial->bUseLightmapDirectionality;
	dstMaterial->bMobileEnableHighQualityBRDF = srcMaterial->bMobileEnableHighQualityBRDF;
	dstMaterial->bUseAlphaToCoverage = srcMaterial->bUseAlphaToCoverage;
	dstMaterial->bForwardRenderUsePreintegratedGFForSimpleIBL = srcMaterial->bForwardRenderUsePreintegratedGFForSimpleIBL;
	dstMaterial->bMobileEnableHighQualityBRDF = srcMaterial->bMobileEnableHighQualityBRDF;
	dstMaterial->bForwardBlendsSkyLightCubemaps = srcMaterial->bForwardBlendsSkyLightCubemaps;
	dstMaterial->bUsePlanarForwardReflections = srcMaterial->bUsePlanarForwardReflections;
	dstMaterial->BlendableLocation = srcMaterial->BlendableLocation;
	dstMaterial->BlendableOutputAlpha = srcMaterial->BlendableOutputAlpha;
	dstMaterial->BlendablePriority = srcMaterial->BlendablePriority;
	dstMaterial->bIsBlendable = srcMaterial->bIsBlendable;
	dstMaterial->bEnableStencilTest = srcMaterial->bEnableStencilTest;
	dstMaterial->StencilCompare = srcMaterial->StencilCompare;
	dstMaterial->StencilRefValue = srcMaterial->StencilRefValue;
	dstMaterial->RefractionMode = srcMaterial->RefractionMode;
	dstMaterial->RefractionDepthBias = srcMaterial->RefractionDepthBias;
	dstMaterial->SetDiffuseBoost(srcMaterial->GetDiffuseBoost());
	dstMaterial->SetExportResolutionScale(srcMaterial->GetExportResolutionScale());
	dstMaterial->SetCastShadowAsMasked(srcMaterial->GetCastShadowAsMasked());
}

#undef FIND_DESCR_WORLDPOSITIONOFFSET
#undef FIND_DESCR_EMISSIVECOLOR
#undef FIND_DESCR_CUSTOM_FROST
#undef FIND_DESCRIPTION_BASE
#undef FIND_DESCR_ROUGHNESS
#undef FIND_DESCR_BASECOLOR
#undef FIND_DESCR_METALLIC
#undef FIND_DESCR_OPACITY
#undef FIND_DESCR_CUSTOM 
#undef FIND_DESCR_NORMAL
#undef FIND_DESCR_INPUT

#undef MATERIAL_TYPE_BEAM
#undef MATERIAL_TYPE_LENS
#undef MATERIAL_TYPE_LIGHT

#undef EDITOR_MINI_BLOCK_SIZE
#undef EDITOR_BLOCK_SIZE
#undef BEAM_EDITOR_BLOCK_DISTANCE_X
#undef MI_EDITOR_MI_BLOCK_DISTANCE_X
#undef MI_EDITOR_PARAM_BLOCK_DISTANCE_X
#undef MI_EDITOR_PARAM_BLOCK_DISTANCE_Y

#undef MI_INIT_BLOCKS_MOVER

#undef EDITOR_TEXTURE_OBJECT_SIZE_Y
#undef EDITOR_SCALAR_PARAM_SIZE_Y
#undef EDITOR_CONSTANT_SIZE_Y
#undef EDITOR_VECTOR4_SIZE_Y