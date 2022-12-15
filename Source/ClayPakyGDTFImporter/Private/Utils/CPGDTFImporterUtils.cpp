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


#include "CPGDTFImporterUtils.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Factories/CPGDTFUnzip.h"

#include "PackageTools.h"
#include "Factories/TextureFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "GLTFImportOptions.h"
#include "Framework/Notifications/NotificationManager.h"

#include <iostream>
#include <fstream>

/**
 * Import a PNG from a GDTF file
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 may 2022
 *
 * @param GDTFPath  Path of the GDTF file on disk
 * @param AssetName Name of the imported Asset
 * @param FileName  Name of the png on GDTF archive
 * @param PathOnContentBrowser   Path of the imported Texture2D on the ContentBrowser
 */
UTexture2D* FCPGDTFImporterUtils::ImportPNG(FString GDTFPath, FString AssetName, FString FileName, FString PathOnContentBrowser) {

	// Check if alls args are provided
	if (GDTFPath.IsEmpty() || AssetName.IsEmpty() || FileName.IsEmpty() || PathOnContentBrowser.IsEmpty()) return nullptr;


	UPackage* AssetPackage;
	UTexture2D* Asset = Cast<UTexture2D>(FCPGDTFImporterUtils::IsAssetExisting(AssetName, PathOnContentBrowser));
	
	// If Asset found in Content Browser (probably a re-import)
	if (Asset != nullptr) AssetPackage = Asset->GetPackage();
	// Creation of the Package to store the Asset
	else AssetPackage = FCPGDTFImporterUtils::PreparePackageOnSubFolder(AssetName, PathOnContentBrowser);
	
	// Load of the file from disk
	std::tuple<void*, int> bufferTuple = UCPGDTFUnzip::ExtractFileFromGDTFArchive(GDTFPath, FileName + ".png");
	if (!std::get<0>(bufferTuple)) { // Check if read problems
		FString LongFileName = FileName + ".png";
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Error opening '%s' on '%s'"), *LongFileName, *GDTFPath);
		return nullptr;
	}

	// Creation of the Asset
	UTextureFactory* Factory = NewObject<UTextureFactory>();
	Factory->SuppressImportOverwriteDialog(true); // Remove overwrite warning for re-import
	Factory->AddToRoot(); // Prevent Garbage Collection
	const uint8* bufferptr = (uint8*)std::get<0>(bufferTuple);
	const uint8* endbufferptr = (bufferptr + std::get<1>(bufferTuple));
	FString CleanAssetName = ObjectTools::SanitizeObjectName(AssetName);
	
	Asset = (UTexture2D*)Factory->FactoryCreateBinary(UTexture2D::StaticClass(), AssetPackage, *CleanAssetName, RF_Standalone | RF_Public, NULL, TEXT("PNG"), bufferptr, endbufferptr, GWarn);
	free(std::get<0>(bufferTuple)); // Clear of the buffer memory

	// Notify the Asset Registery from the creation of the Asset
	FAssetRegistryModule::AssetCreated(Asset);
	Factory->RemoveFromRoot();
	AssetPackage->SetDirtyFlag(true);
	return Asset;
}


/**
 * Import a 3D model from a GDTF file
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 10 may 2022
 *
 * @param GDTFPath  Path of the GDTF file on disk
 * @param AssetName Name of the imported Asset
 * @param FileName  Name of the model on GDTF archive
 * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
 */
UObject* FCPGDTFImporterUtils::Import3DModel(FString GDTFPath, FString AssetName, FString FileName, FString PathOnContentBrowser) {

	// Check if alls args are provided
	if (GDTFPath.IsEmpty() || AssetName.IsEmpty() || FileName.IsEmpty() || PathOnContentBrowser.IsEmpty()) return nullptr;

	UPackage* AssetPackage;
	UObject* Asset = FCPGDTFImporterUtils::IsAssetExisting(AssetName, PathOnContentBrowser);
	
	// If Asset found in Content Browser (probably a re-import)
	if (Asset != nullptr) AssetPackage = Asset->GetPackage();
	// Creation of the Package to store the Asset
	else AssetPackage = FCPGDTFImporterUtils::PreparePackage(AssetName, PathOnContentBrowser);

	FString CleanAssetName = ObjectTools::SanitizeObjectName(AssetName);
	FString TempFilePath = FPaths::ProjectIntermediateDir() + CleanAssetName + TEXT(".glb");
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	// If the file exist we delete it
	if (FileManager.FileExists(*TempFilePath)) FileManager.DeleteFile(*TempFilePath);

	// Load of the model from archive
	FString InternalArchivePath = TEXT("models/gltf/") + FileName + TEXT(".glb");
	std::tuple<void*, int> bufferTuple = UCPGDTFUnzip::ExtractFileFromGDTFArchive(GDTFPath, InternalArchivePath);
	if (!std::get<0>(bufferTuple)) { // Check if read problems
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Error opening '%s' on '%s'"), *InternalArchivePath, *GDTFPath);
		return nullptr;
	}

	// Write the model on temp file
	std::fstream TempFileStream;
	TempFileStream.open(*TempFilePath, std::ios::out | std::ios::binary);
	if (!TempFileStream) UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Unable to write temp '%s' on disk"), *FileName);
	TempFileStream.write((char*)std::get<0>(bufferTuple), std::get<1>(bufferTuple));
	TempFileStream.flush();
	TempFileStream.close();
	free(std::get<0>(bufferTuple)); // Clear of the buffer memory

	// Creation of the settings to automate the import and avoid a popup windows for each model
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	UGLTFImportOptions* ImportOptions = NewObject<UGLTFImportOptions>();
	ImportOptions->bGenerateLightmapUVs = false; // Default value
	ImportOptions->ImportScale = 0.1f;
	UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
	ImportTask->bAutomated = true;
	ImportTask->Options = ImportOptions;
	ImportTask->DestinationPath = AssetPackage->GetFName().ToString();
	ImportTask->DestinationName = CleanAssetName;
	ImportTask->Filename = TempFilePath;

	// Import model
	AssetToolsModule.Get().ImportAssetTasks( { ImportTask } );

	// If import success, load model
	if (!ImportTask->ImportedObjectPaths.IsEmpty()) { 
		
		TArray<UStaticMesh*> LoadedMeshes = FCPGDTFImporterUtils::LoadMeshesInFolder(AssetPackage->GetFName().ToString() + "/" + CleanAssetName);
		if (!LoadedMeshes.IsEmpty()) Asset = LoadedMeshes[0];
	}
	
	FileManager.DeleteFile(*TempFilePath);
	return Asset;
}


/**
 * Try to find the Asset in the Content Browser
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 10 may 2022
 *
 * @param AssetName Name of the imported Asset
 * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
 */
UObject* FCPGDTFImporterUtils::IsAssetExisting(FString AssetName, FString PathOnContentBrowser) {

	FString CleanAssetName = ObjectTools::SanitizeObjectName(AssetName);
	FString BasePackageName = UPackageTools::SanitizePackageName(PathOnContentBrowser + TEXT("/") + CleanAssetName);

	FString ObjectPath = BasePackageName + TEXT(".") + CleanAssetName;
	UObject* existingAsset = LoadObject<UObject>(NULL, *ObjectPath, nullptr, LOAD_Quiet | LOAD_NoWarn);
	if (existingAsset) return existingAsset;
	else return nullptr;
}

/**
 * Create the package on a subfolder for an imported asset
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 10 may 2022
 *
 * @param AssetName Name of the imported Asset
 * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
 */
UPackage* FCPGDTFImporterUtils::PreparePackageOnSubFolder(FString AssetName, FString PathOnContentBrowser) {

	FString CleanAssetName = ObjectTools::SanitizeObjectName(AssetName);
	FString BasePackageName = UPackageTools::SanitizePackageName(PathOnContentBrowser + TEXT("/") + CleanAssetName);

	// Creation of the Asset Package
	const FString Suffix(TEXT(""));
	FString FinalPackageName;
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, Suffix, FinalPackageName, CleanAssetName);
	return CreatePackage(*FinalPackageName);
}

/**
 * Create the package for an imported asset
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 10 may 2022
 *
 * @param AssetName Name of the imported Asset
 * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
 */
UPackage* FCPGDTFImporterUtils::PreparePackage(FString AssetName, FString PathOnContentBrowser) {

	FString CleanAssetName = ObjectTools::SanitizeObjectName(AssetName);
	FString BasePackageName = UPackageTools::SanitizePackageName(PathOnContentBrowser);

	// Creation of the Asset Package
	const FString Suffix(TEXT(""));
	FString FinalPackageName;
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, Suffix, FinalPackageName, CleanAssetName);
	return CreatePackage(*FinalPackageName);
}

/**
 * Load all meshes from a folder on ContentBrowser.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 27 may 2022
 * 
 * @param ContentBrowserFolderPath Path to look for meshes
 * @return Array of UStaticMesh loaded from folder
*/
TArray<UStaticMesh*> FCPGDTFImporterUtils::LoadMeshesInFolder(FString ContentBrowserFolderPath) {

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(FName(ContentBrowserFolderPath));
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	// If no asset found
	if (AssetData.Num() < 1) return {};

	TArray<UStaticMesh*> Meshes;
	for (FAssetData data : AssetData) {
		Meshes.Add(Cast<UStaticMesh>(AssetData[0].GetAsset()));
	}

	return Meshes;
}

/**
 * Load a generic GDTF mesh.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 01 june 2022
 *
 * @param Type Type of the mesh to load
 * @return Mesh or nullptr if EDMXImportGDTFPrimitiveType::Undefined provided
*/
UStaticMesh* FCPGDTFImporterUtils::LoadGDTFGenericMesh(ECPGDTFDescriptionModelsPrimitiveType Type) {

	if (Type == ECPGDTFDescriptionModelsPrimitiveType::Undefined) return nullptr;

	// Construction of the AssetPath
	FString AssetName = StaticEnum<ECPGDTFDescriptionModelsPrimitiveType>()->GetNameStringByValue((uint8)Type);
	FString AssetPath = FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH;
	AssetPath += "GenericMeshes/" + AssetName + "." + AssetName;

	// Load of the Asset
	return Cast<UStaticMesh>(FCPGDTFImporterUtils::LoadObjectByPath(AssetPath));
}

/**
 * Load an Asset stored in Content Browser.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 08 june 2022
 *
 * @param Path Path of the Asset
 * @return Asset if found. nullptr else.
*/
UObject* FCPGDTFImporterUtils::LoadObjectByPath(FString Path) {

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	return AssetRegistryModule.Get().GetAssetByObjectPath(FName(Path)).GetAsset();
}

/**
 * Convert a GDTF "Position" to an Unreal FRotator.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 24 may 2022
 *
 * @param InMatrix      Matrix to convert
 * @param OutRotator    Rotator who represent the relative rotation
 */
void FCPGDTFImporterUtils::MatrixToRotator(FMatrix InMatrix, FRotator* OutRotator) {

	/*	Rotation Matrix is 3x3 on the top left corner

	Rotation matrix, consist of 3*3 floats. Stored as row-major matrix, i.e. each row of the matrix is stored as a 3-component vector.
	Mathematical definition of the matrix is column-major, i.e. the matrix rotation is stored in the three columns. Metric system, right-handed Cartesian coordinates XYZ:
	*/

	/*********************************************************************
	 *          TODO \todo CHECK THE UNREAL ROTATION CALCULATION         *
	 * https://en.wikipedia.org/wiki/Rotation_matrix#In_three_dimensions *
	**********************************************************************/
	//UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Matrix in input %s"), *InMatrix.ToString());
	//UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Rotator Test '%s'"), *InMatrix.Rotator().ToString());

	* OutRotator = InMatrix.Rotator(); // If the Unreal default calculation is correct this function can be deleted
}

double FCPGDTFImporterUtils::NearestPowerOfTwo(double N) {

	int log = FMath::Log2(N);
	double under = FMath::Pow(2.0, log);
	double over = FMath::Pow(2.0, log + 1.0);

	if (under == 0) return over;
	else return (N - under) < (over - N) ? under : over;
}

/**
 * Send notification to the UI
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 07 september 2022
 *
 * @param Title Title of the notification
 * @param Message Message to show
 * @param State Success or failure
 * @param Duration Default value 3s
 * @param FadeInDuration Default value 0.5s
 * @param FadeOutDuration Default value 0.5s
 */
void FCPGDTFImporterUtils::SendNotification(FString Title, FString Message, SNotificationItem::ECompletionState State, float Duration, float FadeInDuration, float FadeOutDuration) {

	FNotificationInfo Info = FNotificationInfo(FText::FromString(Title));
	//Info.Text = FText::FromString(Title);
	Info.SubText = FText::FromString(Message);
	Info.ExpireDuration = Duration;
	Info.FadeInDuration = FadeInDuration;
	Info.FadeOutDuration = FadeOutDuration;
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;

	TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	if (Notification.IsValid()) {
		Notification->SetCompletionState(State);
	}
}