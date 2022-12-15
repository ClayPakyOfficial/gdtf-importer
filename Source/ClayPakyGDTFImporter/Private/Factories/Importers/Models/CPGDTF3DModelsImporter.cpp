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

#include "CPGDTF3DModelsImporter.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "PackageTools.h"
#include <stdexcept>


/**
 * Creates the Importer of GDTF 3D models
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 10 may 2022
 */
FCPGDTF3DModelsImporter::FCPGDTF3DModelsImporter(UPackage* Package, FString GDTFPath, const FXmlFile* XMLFile) {

	this->Package = Package;
	this->GDTFPath = GDTFPath;
	this->XMLFile = XMLFile;

	FString VersionStr = XMLFile->GetRootNode()->GetAttribute("DataVersion");
	TArray<FString> VersionArray;
	VersionStr.ParseIntoArray(VersionArray, TEXT("."), true);
	TArray<FString> MinVersionArray;
	this->MIN_GDTF_VERSION_SUPPORTED.ParseIntoArray(MinVersionArray, TEXT("."), true);

	for (int i = 0; i < MinVersionArray.Num(); i++) {

		if (FCString::Atoi(*VersionArray[i]) < FCString::Atoi(*MinVersionArray[i])) {

			const FText Message = FText::Format(FTextFormat::FromString("GDTF version unsupported for 3D models. Please use GDTF {0} with gltf models. Version provided: {1}"), FText::FromString(this->MIN_GDTF_VERSION_SUPPORTED), FText::FromString(VersionStr));
			UE_LOG_CPGDTFIMPORTER(Error, TEXT("%s"), *Message.ToString());
			FCPGDTFImporterUtils::SendNotification("3D models import failed", Message.ToString(), SNotificationItem::CS_Fail);

			/*
			 * Throws this exception will crash Unreal if you use Unreal 'Live Coding'.
			 * 0 chrash with complete build from Visual Studio.
			 * With incremental patches the editor will crash.
			 */
			throw std::invalid_argument("GDTF Version not supported");
		}
	}
}

/**
 * Import the 3D models from a GDTF file
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 10 may 2022
 *
 * @return True is all the models imports succeded
 */
bool FCPGDTF3DModelsImporter::Import() {

	// If the fixture doesn't have models our work is done
	if (!this->XMLFile->GetRootNode()->GetFirstChildNode()->FindChildNode("Models")) return true;
	// Same if the models node is empty
	if (!this->XMLFile->GetRootNode()->GetFirstChildNode()->FindChildNode("Models")->GetFirstChildNode()) return true;

	// We import the models
	bool bEverythingOK = true;
	const FXmlNode* ModelNode = this->XMLFile->GetRootNode()->GetFirstChildNode()->FindChildNode("Models")->GetFirstChildNode();
	while (ModelNode != nullptr) {

		if (!this->Import3DModel(ModelNode)) {
			UE_LOG_CPGDTFIMPORTER(Error, TEXT("Failed to import model '%s'"), *ModelNode->GetAttribute("Name"));
			FCPGDTFImporterUtils::SendNotification("3D model import failed", FString::Printf(TEXT("Failed to import model '%s'"), *ModelNode->GetAttribute("Name")), SNotificationItem::CS_Fail);
			bEverythingOK = false;
		}
		ModelNode = ModelNode->GetNextNode();
	}

	return bEverythingOK;
}

/**
 * Import a 3D model from a GDTF file.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 10 may 2022
 *
 * @param ModelXML XML Node representing the model
 */
bool FCPGDTF3DModelsImporter::Import3DModel(const FXmlNode* ModelXML) {

	// This model doesn't have a 3D model
	if (ModelXML->GetAttribute("File").Equals("")) return true;

	UObject* model = FCPGDTFImporterUtils::Import3DModel(this->GDTFPath, ModelXML->GetAttribute("Name"), ModelXML->GetAttribute("File"), this->Package->GetName() + TEXT("/models"));
	return model != nullptr;
}

