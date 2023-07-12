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

#include "Factories/CPGDTFFactory.h"
#include "Widgets/CPGDTFImportUI.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Factories/TextureFactory.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Factories/Importers/Description/CPGDTFDescriptionImporter.h"
#include "Factories/Importers/Wheels/CPGDTFWheelImporter.h"
#include "Factories/Importers/Models/CPGDTF3DModelsImporter.h"
#include "Factories/CPGDTFUnzip.h"
#include "CPGDTFDescription.h"
#include "CPGDTFFixtureActor.h"
#include "Library/DMXGDTFAssetImportData.h"
#include "Widgets/SCPGDTFOptionWindow.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FeedbackContext.h"
#include "ObjectTools.h"
#include "Editor.h"
#include "Selection.h"
#include "SubSystems/EditorAssetSubsystem.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"

#define LOCTEXT_NAMESPACE "CPGDTFFactory"
const TCHAR* UCPGDTFFactory::Extension = TEXT("gdtf");

UCPGDTFFactory::UCPGDTFFactory() {

	SupportedClass = nullptr;
	Formats.Add(TEXT("gdtf;General Device Type Format"));

	this->bCreateNew = false;
	this->bText = false;
	this->bEditorImport = true;
	this->bOperationCanceled = false;

	// We ensure that we will be the default GDTF factory
	this->ImportPriority = this->ImportPriority * 10;
}

void UCPGDTFFactory::CleanUp() {

	Super::CleanUp();
	this->bShowOption = true;
}

bool UCPGDTFFactory::ConfigureProperties() {

	Super::ConfigureProperties();
	EnableShowOption();

	return true;
}

void UCPGDTFFactory::PostInitProperties() {

	Super::PostInitProperties();
	ImportUI = NewObject<UCPGDTFImportUI>(this, NAME_None, RF_NoFlags);
}

bool UCPGDTFFactory::DoesSupportClass(UClass* Class) {

	return Class == UCPGDTFDescription::StaticClass();
}

UClass* UCPGDTFFactory::ResolveSupportedClass() {

	return UCPGDTFDescription::StaticClass();
}

FName UCPGDTFFactory::generateActorName(UCPGDTFDescription* XMLDescription) {
	return FName(ObjectTools::SanitizeObjectName("A_" + Cast<UDMXImportGDTFFixtureType>(XMLDescription->FixtureType)->Name.ToString()));
}
FString UCPGDTFFactory::generateModeName(int mode) {
	return TEXT("m") + FString::FromInt(mode) + TEXT("m");
}
FName UCPGDTFFactory::generateActorModeName(UCPGDTFDescription* XMLDescription, int mode) {
	return FName(ObjectTools::SanitizeObjectName(generateActorName(XMLDescription).ToString() + "_" + generateModeName(mode)));
}

/**
 * Import GDTF file in the Content Browser
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 04 may 2022
 *
 * @param InClass       UCPGDTFDescription - Class used to instantiate the object
 * @param InParent      Path of the newly created asset in ContentBrowser
 * @param InName        Name of the newly created asset in ContentBrowser
 * @param Flags
 * @param InFilename    Path of the GDTF file on disk
 * @param Parms
 * @param Warn
 * @param bOutOperationCanceled
 */
UObject* UCPGDTFFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& InFilename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) {
	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: UCPGDTFFactory::FactoryCreateFile CALLED - start"));


	// Check if the file exists on disk
	if (!IFileManager::Get().FileExists(*InFilename)) {

		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Failed to load file '%s'"), *InFilename);
		FCPGDTFImporterUtils::SendNotification("Import failed", FString::Printf(TEXT("Failed to load file '%s'"), *InFilename), SNotificationItem::CS_Fail);
		return nullptr;
	}

	ParseParms(Parms);
	CA_ASSUME(InParent);
	const TCHAR* Type = *FPaths::GetExtension(InFilename); // = "gdtf"

	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: UCPGDTFFactory::FactoryCreateFile CALLED - broadcast"));
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Type);

	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: UCPGDTFFactory::FactoryCreateFile CALLED - check if exist"));
	// Check if GDTF was already imported
	if (InParent != nullptr && FCPGDTFImporterUtils::IsAssetExisting(InName.ToString(), InParent->GetName()) != nullptr) {

		this->bShowOption = false;
		ImportUI->bImportXML = true;
		ImportUI->bImportModels = true;
		ImportUI->bImportTextures = true;
	}

	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: UCPGDTFFactory::FactoryCreateFile CALLED - creating xlr"));
	// Creation of the XML Importer object
	FCPGDTFDescriptionImporter XMLImporter = FCPGDTFDescriptionImporter(InParent->GetPackage(), InName, Flags, InFilename);

	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: UCPGDTFFactory::FactoryCreateFile CALLED - opening ui"));
	// Sould we open the UI ?
	bool bImportAll = false;
	if (this->bShowOption && !IsAutomatedImport()) SCPGDTFOptionWindow::GetImportOptions(ImportUI, InParent->GetPathName(), this->bOperationCanceled, bImportAll);
	bOutOperationCanceled = this->bOperationCanceled;
	// If the user chose to import all, we don't show the dialog again and use the same settings for each object until importing another set of files
	this->bShowOption = !bImportAll;

	if (this->bOperationCanceled) return nullptr;

	if (!ImportUI->bImportXML && !ImportUI->bImportModels && !ImportUI->bImportTextures) {
		Warn->Log(ELogVerbosity::Error, TEXT("Nothing to Import"));
		bOutOperationCanceled = true;
		FCPGDTFImporterUtils::SendNotification("Import cancelled", "Nothing to import", SNotificationItem::CS_Fail);
		return nullptr;
	}

	UE_LOG_CPGDTFIMPORTER(Display, TEXT("CPGDTFRenderPipelineBuilder: UCPGDTFFactory::FactoryCreateFile CALLED - start import"));
	/**
	 * BEGINNING OF GDTF IMPORT
	 */

	 // Begining of the XMLDescription import
	 // Creation of the object we need it even if we don't import it in the ContentBrowser
	UCPGDTFDescription* XMLDescription = XMLImporter.Import(); // Reading of the XML file
	if (XMLDescription == nullptr) {
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		FCPGDTFImporterUtils::SendNotification("Import failed", "Error on GDTF description import", SNotificationItem::CS_Fail);
		return nullptr;
	}

	// Add the possibility to reimport
	XMLDescription->GetGDTFAssetImportData()->SetSourceFile(InFilename);

	if (ImportUI->bImportXML) { // Import XML to ContentBrowser

		// Thumbnail import
		UTexture2D* Thumbnail = XMLImporter.ImportThumbnail(XMLDescription);
		if (Thumbnail) {
			Cast<UDMXImportGDTFFixtureType>(XMLDescription->FixtureType)->Thumbnail = Thumbnail;
			//this->AdditionalImportedObjects.Add(Thumbnail);
		}
	}

	if (ImportUI->bImportTextures) { // Import textures Wheels to ContentBrowser

		FCPGDTFWheelImporter WheelsImporter = FCPGDTFWheelImporter(InParent->GetPackage(), InFilename, XMLImporter.GetXML());
		if (!WheelsImporter.Import()) { // If something happened on wheels import we notify the user

			const FText Message = FText::Format(LOCTEXT("ImportFailed_Generic", "Error on '{0}' GDTF textures import.\nPlease see Output Log for details."), FText::FromString(InFilename));
			FCPGDTFImporterUtils::SendNotification("Import failed", Message.ToString(), SNotificationItem::CS_Fail);

		} else if (ImportUI->bImportXML) {

			WheelsImporter.LinkTexturesToGDTFDescription(XMLDescription);
			WheelsImporter.CreateGDTFWheelsTextures(XMLDescription, InParent->GetName());
		}
	}

	if (ImportUI->bImportModels) { // Import 3D models to Content Browser
		try {

			FCPGDTF3DModelsImporter ModelsImporter = FCPGDTF3DModelsImporter(InParent->GetPackage(), InFilename, XMLImporter.GetXML());
			if (!ModelsImporter.Import()) { // If something appened on wheels import we notify the user

				const FText Message = FText::Format(LOCTEXT("ImportFailed_Generic", "Error on '{0}' GDTF models import.\nPlease see Output Log for details."), FText::FromString(InFilename));
				FCPGDTFImporterUtils::SendNotification("Import failed", Message.ToString(), SNotificationItem::CS_Fail);

				// If 3D models import went well and a complete import was requested we create the actor
			} else if (ImportUI->bImportXML && ImportUI->bImportTextures && ImportUI->bImportModels) {
				struct ReimportBP { UBlueprint* existingBp, *newBp;  ReimportBP(UBlueprint* existingBlueprint, UBlueprint* newBlueprint) : existingBp(existingBlueprint), newBp(newBlueprint) {} };
				TArray<UBlueprint*> newBluePrints;
				TArray<ReimportBP> reimportBlueprints;
				UBlueprint* mode0Bp = nullptr;
				for (int mode = 0; mode < XMLDescription->GetDMXModes()->DMXModes.Num(); mode++) {

					FString modeName = generateModeName(mode);
					FName BluePrintName = generateActorModeName(XMLDescription, mode);
					FString BluePrintPath = InParent->GetName() + "/Actors/";
					UBlueprint* ExistingBlueprint = Cast<UBlueprint>(FCPGDTFImporterUtils::IsAssetExisting(BluePrintName.ToString(), BluePrintPath));

					// Creation of the ready to use Actor
					ACPGDTFFixtureActor* Actor = NewObject<ACPGDTFFixtureActor>();
					Actor->FixturePathInContentBrowser = InParent->GetName();
					Actor->ActorsPathInContentBrowser = BluePrintPath;
					//Actor->CreateRenderingPipelines(XMLDescription);
					Actor->CurrentModeName = generateModeName(mode);
					Actor->CurrentModeIndex = mode;
					Actor->PreConstruct(XMLDescription);

					UPackage* BluePrintPackage = FCPGDTFImporterUtils::PreparePackage(BluePrintName.ToString(), BluePrintPath + BluePrintName.ToString());

					// Convert Actor to a Blueprint
					FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
					Params.bOpenBlueprint = false;
					Params.bReplaceActor = false;
					Params.bDeferCompilation = true;
					UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprintFromActor(BluePrintName, BluePrintPackage, Actor, Params);
					if (mode0Bp == nullptr)
						mode0Bp = NewBlueprint;

					if (NewBlueprint != nullptr) {
						if (ExistingBlueprint == nullptr) { // If import
							UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Importing '%s' blueprint"), *BluePrintName.ToString());
							newBluePrints.Add(NewBlueprint);
						} else { // Reimport
							UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Reimporting '%s' blueprint"), *BluePrintName.ToString());
							reimportBlueprints.Add(ReimportBP(ExistingBlueprint, NewBlueprint));
						}
					} else {
						UE_LOG_CPGDTFIMPORTER(Error, TEXT("Unable to create %s Actor"), *BluePrintName.ToString());
						FCPGDTFImporterUtils::SendNotification("Import failed", FString::Printf(TEXT("Unable to create %s Actor"), *InFilename), SNotificationItem::CS_Fail);
					}
				}
				//We must skip garbage collection in the following phases and postpone it to the end, otherwise InParent will be deleted and we will segfault
				//New blueprints
				for (UBlueprint* bp : newBluePrints) {
					FKismetEditorUtilities::CompileBlueprint(bp, EBlueprintCompileOptions::SkipGarbageCollection);
					FAssetRegistryModule::AssetCreated(bp); // Notify the asset registry
				}
				//Reimport
				if (reimportBlueprints.Num() > 0) {
					GEditor->GetSelectedObjects()->DeselectAll();
					GEditor->GetSelectedActors()->DeselectAll();
					GEditor->GetSelectedComponents()->DeselectAll(); //Prevents segfault if we replace a blueprint to a selected object
				}
				for (ReimportBP bp : reimportBlueprints) {
					FKismetEditorUtilities::CompileBlueprint(bp.newBp, EBlueprintCompileOptions::SkipGarbageCollection);
					FKismetEditorUtilities::ReplaceBlueprint(bp.existingBp, bp.newBp); //Replace all existing instance of the old blueprint with the new, temporary, one
					ObjectTools::DeleteAssets({ bp.newBp }, false); //Delete the new, temporary, blueprint
				}
				//If we have re/imported any blueprint
				if (newBluePrints.Num() > 0 || reimportBlueprints.Num() > 0) {
					UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
					if (mode0Bp) //Create a "base" blueprint
						EditorAssetSubsystem->DuplicateLoadedAsset(mode0Bp, InParent->GetName() + "/" + generateActorName(XMLDescription).ToString());
					CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS); //Finally collect garbage
					EditorAssetSubsystem->SaveDirectory(InParent->GetPackage()->GetName(), true, true);
					FCPGDTFImporterUtils::SendNotification("Import success", FString::Printf(TEXT("Successfully imported '%s'"), *InFilename), SNotificationItem::CS_Success);
				}
			}


		} catch (const std::invalid_argument& e) {

			FString errorMessage = e.what();

			// If this is not a GDTF version error we let propagate the error
			if (!errorMessage.Equals("GDTF Version not supported")) throw e;
		}
	}

	if (ImportUI->bImportXML) {
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, XMLDescription);
		return XMLDescription;
	} else {
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}
}

bool UCPGDTFFactory::FactoryCanImport(const FString& Filename) {

	return FPaths::GetExtension(Filename) == UCPGDTFFactory::Extension;
}

bool UCPGDTFFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames) {

	UCPGDTFDescription* GDTFReimport = Cast<UCPGDTFDescription>(Obj);
	if (GDTFReimport) {

		OutFilenames.Add(GDTFReimport->GetGDTFAssetImportData()->GetFilePathAndName());
		return true;
	} else return false;
}

void UCPGDTFFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) {

	UCPGDTFDescription* GDTFReimport = Cast<UCPGDTFDescription>(Obj);
	if (GDTFReimport && ensure(NewReimportPaths.Num() == 1)) {
		GDTFReimport->GetGDTFAssetImportData()->SetSourceFile(NewReimportPaths[0]);
	}
}

EReimportResult::Type UCPGDTFFactory::Reimport(UObject* InObject) {

	UCPGDTFDescription* GDTFReimport = Cast<UCPGDTFDescription>(InObject);

	if (!GDTFReimport || !GDTFReimport->GetGDTFAssetImportData()) return EReimportResult::Failed;

	const FString SourceFilename = GDTFReimport->GetGDTFAssetImportData()->GetFilePathAndName();
	if (SourceFilename.IsEmpty() || !FPaths::FileExists(SourceFilename)) {
		FCPGDTFImporterUtils::SendNotification("Reimport failed", FString::Printf(TEXT("Unable to find GDTF source file: '%s'"), *SourceFilename), SNotificationItem::CS_Fail);
		return EReimportResult::Failed;
	}

	FString ParentFolder = InObject->GetOuter()->GetPackage()->GetName();
	int32 Index = 0;
	ParentFolder.FindLastChar('/', Index);
	ParentFolder = ParentFolder.Left(Index);

	this->bAutomatedReimport = true;
	bool OutCanceled = false;
	if (ImportObject(InObject->GetClass(), CreatePackage(*ParentFolder), *InObject->GetName(), RF_Public | RF_Standalone, SourceFilename, nullptr, OutCanceled)) {
		return EReimportResult::Succeeded;
	}

	return OutCanceled ? EReimportResult::Cancelled : EReimportResult::Failed;
}

int32 UCPGDTFFactory::GetPriority() const {
	return ImportPriority;
}

#undef LOCTEXT_NAMESPACE