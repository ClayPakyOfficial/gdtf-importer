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
#include "XMLFile.h"
#include "CPGDTFDescription.h"

class UCPGDTF3DModelsImporter;

/**
 * GDTF 3D models Importer
 */
class FCPGDTF3DModelsImporter {

private:

	/** Folder where the asset will be stored */
	UPackage* Package;

	/** Path of the GDTF file on disk */
	FString GDTFPath;

	/** XML Description extracted from GDTF archive */
	const FXmlFile* XMLFile;

	/** Minimal version of GDTF supported by 3D models importer */
	FString MIN_GDTF_VERSION_SUPPORTED = "1.2";

public:

	/**
	 * Creates the Importer of GDTF 3D models
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 10 may 2022
	 */
	FCPGDTF3DModelsImporter(UPackage* Package, FString GDTFPath, const FXmlFile* XMLFile);

	/**
	 * Import the 3D models from a GDTF file
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 10 may 2022
	 *
	 * @return True is all the models imports succeded
	 */
	bool Import();

private:

	/**
	 * Import a 3D model from a GDTF file.
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 10 may 2022
	 *
	 * @param ModelXML XML Node representing the model
	 */
	bool Import3DModel(const FXmlNode* ModelXML);
};
