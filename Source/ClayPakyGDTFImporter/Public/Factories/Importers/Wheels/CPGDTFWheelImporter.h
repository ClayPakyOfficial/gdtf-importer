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
#include "XMLFile.h"
#include "Engine/Texture.h"
#include "CPGDTFDescription.h"

class UCPGDTFWheelImporter;

/**
 * GDTF Wheels Importer
 */
class FCPGDTFWheelImporter {

private:

	/** Folder where the asset will be stored */
	UPackage* Package;

	/** Path of the GDTF file on disk */
	FString GDTFPath;

	/** XML Description extracted from GDTF archive */
	const FXmlFile* XMLFile;

public:

	/**
	 * Creates the Importer of GDTF textures (gobos, prisms, colors wheels ...)
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 06 may 2022
	 */
	FCPGDTFWheelImporter(UPackage* Package, FString GDTFPath, const FXmlFile* XMLFile);

	/**
	 * Import the textures from a GDTF file
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 06 may 2022
	 *
	 * @return True is all the wheels slots imports succeded
	 */
	bool Import();

	/**
	 * Link the textures (Gobos, Colors... ) of a fixture to his GDTF Description
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 06 may 2022
	 */
	void LinkTexturesToGDTFDescription(UCPGDTFDescription* FixtureAsset);

	/**
	 * Automaticaly generate the Wheels disks textures
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 06 july 2022
	 *
	 * @param FixtureDescription GDTFDescription of the Fixture
	 * @param FixturePackagePath Path of the fixture folder on Content Browser
	 */
	static void CreateGDTFWheelsTextures(UCPGDTFDescription* FixtureDescription, FString FixturePackagePath);

	/**
	 * Create an Array of Colors for the construction of the Color Wheel disk
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 06 july 2022
	 *
	 * @param Wheel Description of the wheel constructed
	 * @return The Array of Color for a specific wheel
	 */
	static TArray<FLinearColor> GenerateColorArray(FDMXImportGDTFWheel Wheel);

	//DO NOT assign manual values to this enum
	enum WheelType {
		Color,
		Gobo,
		Prism,
		Animation,
		Effects,
		WHEEL_TYPE_SIZE
	};

private: 
	//Must reflect the one specified in WheelType
	static constexpr const char* const WheelTypeAsString[WheelType::WHEEL_TYPE_SIZE] = {
		"Color",
		"Gobo",
		"Prism",
		"Animation",
		"Effects"
	};

public:
	/**
	 * Gets the wheel's name as a String
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	 *
	 * @param wheelType Type of the Wheel
	 */
	static inline FString wheelTypeToString(FCPGDTFWheelImporter::WheelType wheelType) {
		FString ret(FCPGDTFWheelImporter::WheelTypeAsString[wheelType]);
		return ret;
	}

	/**
	 * Find the Wheel type with GDTFDescription in the specified dmx mode
	 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
	 * @date 06 july 2022
	 *
	 * @param FixtureDescription GDTF Description of a fixture
	 * @param Name Name of the Wheel
	 * @param mode Index of the selected fixture's mode
	 * @return Type of the Wheel
	 */
	static WheelType GetWheelType(UCPGDTFDescription* FixtureDescription, FName Name, int mode);

	/**
	 * Find the Wheel type with GDTFDescription
	 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
	 * @date 06 july 2022
	 *
	 * @param FixtureDescription GDTF Description of a fixture
	 * @param Name Name of the Wheel
	 * @param mode Index of the selected fixture's mode
	 * @return Type of the Wheel
	 */
	static WheelType GetWheelType(UCPGDTFDescription* FixtureDescription, FName Name);

private:

	/**
	 * Import the GDTF textures related to a specific wheel if applicable
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 06 may 2022
	 *
	 * @param WheelXML XML Node representing the wheel
	 */
	bool ImportWheel(const FXmlNode* WheelXML);

	/**
	 * Create a texture for a given array of SubTextures
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 12 july 2022
	 *
	 * @param SavePath		Path of the texture on the content browser
	 * @param TexturesArray Array of textures used to build the wheel disk
	 */
	static void CreateGoboWheelTexture_Internal(FString SavePath, TArray<UTexture2D*> TexturesArray);

	/**
	 * Frost the Gobo Wheel Texture
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 14 july 2022
	 *
	 * @param PixelsArray Pixels to frost
	 * @param SizeX Size of the Texture
	 * @param SizeY Size of the Texture
	 * @param Strenght Strenght of the frost
	 */
	static void FrostWheelTexture_Internal(uint8* PixelsArray, int SizeX, int SizeY, float Strenght);

	/**
	 * Create a texture for a given array of Colors
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 06 july 2022
	 *
	 * @param SavePath		Path of the texture on the content browser
	 * @param ColorsArray Array of colors used to build the wheel disk
	 */
	static void CreateColorWheelTexture_Internal(FString SavePath, TArray<FLinearColor> ColorsArray);

	/**
	 * Create a texture for a given array of Pixels
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 12 july 2022
	 *
	 * @param PixelsArray Pixels of the future texture
	 * @param SavePath Path in the ContentBrowser
	 * @param SizeX Size of the texture
	 * @param SizeY Size of the texture
	 * @param bIsFrosted
	 */
	static void SaveWheelToTexture_Internal(uint8* PixelsArray, FString SavePath, int SizeX, int SizeY, bool bIsFrosted = false);
};
