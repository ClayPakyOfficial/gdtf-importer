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
#include "Engine/Texture.h"

class UCPGDTFDescriptionImporter;

/**
 * GDTF XML Importer
 */
class FCPGDTFDescriptionImporter {

private:
	/** Folder where the asset will be stored */
	UPackage* Package;

	/** Name of the future imported Asset */
	FName AssetName;

	/** Flags to instantiate objects */
	EObjectFlags Flags;

	/** Path of the GDTF file on disk */
	FString GDTFPath;

	/** XML Description extracted from GDTF archive */
	const FXmlFile* XMLFile;

public:

	/**
	 * Creates the Importer of GDTF Descriptions
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 04 may 2022
	 */
	FCPGDTFDescriptionImporter(UPackage* Package, FName AssetName, EObjectFlags Flags, FString GDTFPath);

	/**
	 * Import the GDTF Description from a GDTF file
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 04 may 2022
	 */
	UCPGDTFDescription* Import();

	/**
	 * Import the GDTF Thumbnail from a GDTF file
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 05 may 2022
	 *
	 * @param XMLDescription XMLFile to find the name of the png file on the GDTF archive
	 */
	UTexture2D* ImportThumbnail(UCPGDTFDescription* XMLDescription);

	/**
	 * Get the XML Description representing the Fixture
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 06 may 2022
	 */
	const FXmlFile* GetXML();

private:

	/**
	 * Parse the XML file and create the GDTF Description
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 04 may 2022
	 */
	UCPGDTFDescription* ParseXML();

	/**
	 * Parse an XML geometry node and all his childrens recursively
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 14 june 2022 
	*/
	UCPGDTFDescriptionGeometryBase* ParseGeometriesNode(UCPGDTFDescription* DescriptionRoot, const FXmlNode* GeometryNode);

	/**
	 * Extract and read description.xml inside GDTF archive.
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 04 may 2022
	 *
	 * @return May be null if it was unable to read the XML file
	 */
	FXmlFile* ExtractXML();

	/**
	 * Depending on the GDTF Spec, some XML Attribute Names might start with or without DMX.
	 *
	 * @param ParentNode		The Parent Node to search in
	 * @param AttributeNameWithDMXTag The Attribute Name with DMX tag. This is the correct form, e.g. 'DMXMode', not 'Mode', 'DMXChannels', not 'Channels'.
	 * @return					Returns a pointer to the child node, or nullptr if the child cannot be found
	 */
	static const FXmlNode* FindChildNodeEvenIfDMXSubstringIsMissing(const FXmlNode& ParentNode, const FString& AttributeNameWithDMXTag);

	/**
	 * Depending on the GDTF Spec, some XML Attribute Names might start with or without DMX.
	 *
	 * @param ParentNode		The Parent Node to search in
	 * @param AttributeNameWithDMXTag The Attribute Name with DMX tag. This is the correct form, e.g. 'DMXMode', not 'Mode', 'DMXChannels', not 'Channels'.
	 * @return					Returns the Attribute as String
	 */
	static FString FindAttributeEvenIfDMXSubstringIsMissing(const FXmlNode& ParentNode, const FString& AttributeNameWithDMXTag);
};
