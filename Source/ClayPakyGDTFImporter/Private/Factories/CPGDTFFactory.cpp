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

/**
 * Import GDTF file in the Content Browser
 * @author Dorian Gardes - Clay Paky S.P.A.
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

	// Check if the file exists on disk
	if (!IFileManager::Get().FileExists(*InFilename)) {

		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Failed to load file '%s'"), *InFilename);
		FCPGDTFImporterUtils::SendNotification("Import failed", FString::Printf(TEXT("Failed to load file '%s'"), *InFilename), SNotificationItem::CS_Fail);
		return nullptr;
	}

	ParseParms(Parms);
	CA_ASSUME(InParent);
	const TCHAR* Type = *FPaths::GetExtension(InFilename); // = "gdtf"

	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Type);

	// Check if GDTF was already imported
	if (InParent != nullptr && FCPGDTFImporterUtils::IsAssetExisting(InName.ToString(), InParent->GetName()) != nullptr) {

		this->bShowOption = false;
		ImportUI->bImportXML = true;
		ImportUI->bImportModels = true;
		ImportUI->bImportTextures = true;
	}

	// Creation of the XML Importer object
	FCPGDTFDescriptionImporter XMLImporter = FCPGDTFDescriptionImporter(InParent->GetPackage(), InName, Flags, InFilename);

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

				FName BluePrintName = FName(ObjectTools::SanitizeObjectName("A_" + Cast<UDMXImportGDTFFixtureType>(XMLDescription->FixtureType)->Name.ToString()));
				UBlueprint* Blueprint = Cast<UBlueprint>(FCPGDTFImporterUtils::IsAssetExisting(BluePrintName.ToString(), InParent->GetName()));

				if (Blueprint == nullptr) { // If import

					// Creation of the ready to use Actor
					ACPGDTFFixtureActor* Actor = NewObject<ACPGDTFFixtureActor>();
					Actor->PreConstruct(XMLDescription, InParent->GetName());

					UPackage* BluePrintPackage = FCPGDTFImporterUtils::PreparePackage(BluePrintName.ToString(), InParent->GetName() + "/" + BluePrintName.ToString());
					// Convert Actor to a Blueprint
					FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
					Params.bOpenBlueprint = false;
					Params.bReplaceActor = false;
					Blueprint = FKismetEditorUtilities::CreateBlueprintFromActor(BluePrintName, BluePrintPackage, Actor, Params);

					if (Blueprint != nullptr) {

						// Notify the asset registry
						FAssetRegistryModule::AssetCreated(Blueprint);
						UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
						EditorAssetSubsystem->SaveDirectory(InParent->GetPackage()->GetName(), true, true);
						FCPGDTFImporterUtils::SendNotification("Import success", FString::Printf(TEXT("Successfully imported '%s'"), *InFilename), SNotificationItem::CS_Success);
					} else {
						UE_LOG_CPGDTFIMPORTER(Error, TEXT("Unable to create %s Actor"), *BluePrintName.ToString());
						FCPGDTFImporterUtils::SendNotification("Import failed", FString::Printf(TEXT("Unable to create %s Actor"), *InFilename), SNotificationItem::CS_Fail);
					}
				} else { // Reimport


					// First method trying to replace the existing BluePrint (not working well)
					/*FKismetEditorUtilities::ReplaceBlueprint(ExistingBlueprint, NewBlueprint);
					ExistingBlueprint->GetPackage()->SetDirtyFlag(true);
					NewBlueprint->ConditionalBeginDestroy();
					NewBlueprint->MarkAsGarbage();
					FAssetRegistryModule::AssetDeleted(ExistingBlueprint);*/


					// Second method editing the BluePrint (not working better)
					/*ACPGDTFFixtureActor* BlueprintValues = Blueprint->GeneratedClass.Get()->GetDefaultObject<ACPGDTFFixtureActor>();
					// Add Clear DMXComponents
					// Add Clear geometry tree
					BlueprintValues->PreConstruct(XMLDescription, InParent->GetName());
					// Compile the changes
					FCompilerResultsLog LogResults; // We don't really care of the logs here.
					FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &LogResults);*/
					
					//Blueprint->MarkPackageDirty();
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
		//UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
		//EditorAssetSubsystem->SaveDirectory(InParent->GetPackage()->GetName(), true, true);
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