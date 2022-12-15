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

#include "CPGDTFWheelImporter.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Utils/CPGDTFColorWizard.h"
#include "XMLFile.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "Rendering/Texture2DResource.h"
#include "Engine/Texture.h"
#include "Libs/FastGaussianBlur/FastGaussianBlur.h"

// We limit the size of each gobo at 256x256 to preserve performances
#define CP_GOBO_SIZE 256

/**
* Creates the Importer of GDTF textures (gobos, prisms, colors wheels ...)
* @author Dorian Gardes - Clay Paky S.P.A.
* @date 06 may 2022
*/
FCPGDTFWheelImporter::FCPGDTFWheelImporter(UPackage* Package, FString GDTFPath, const FXmlFile* XMLFile) {
	
	this->Package = CreatePackage(*(Package->GetName() + "/textures/")); // We store all the textures on a subfolder
	this->GDTFPath = GDTFPath;
	this->XMLFile = XMLFile;
}

/**
 * Import the textures from a GDTF file
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 may 2022
 *
 * @return True is all the wheels slots imports succeded
 */
bool FCPGDTFWheelImporter::Import() {

	// If the fixture doesn't have wheels our work is done
	if (!this->XMLFile->GetRootNode()->GetFirstChildNode()->FindChildNode("Wheels")) return true;
	// Same if the wheels node is empty
	if (!this->XMLFile->GetRootNode()->GetFirstChildNode()->FindChildNode("Wheels")->GetFirstChildNode()) return true;

	// We import the wheels
	bool bEverythingOK = true;
	const FXmlNode* WheelNode = this->XMLFile->GetRootNode()->GetFirstChildNode()->FindChildNode("Wheels")->GetFirstChildNode();
	while (WheelNode != nullptr) {
	
		if (!this->ImportWheel(WheelNode)) bEverythingOK = false;
		WheelNode = WheelNode->GetNextNode();
	}

	return bEverythingOK;
}

/**
 * Link the textures (Gobos, Colors... ) of a fixture to his GDTF Description
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 may 2022
 */
void FCPGDTFWheelImporter::LinkTexturesToGDTFDescription(UCPGDTFDescription* FixtureAsset) {

	TArray<FDMXImportGDTFWheel> wheels = Cast<UDMXImportGDTFWheels>(FixtureAsset->Wheels)->Wheels;

	// For each wheel
	for (int i = 0; i < wheels.Num(); i++) {
	
		FDMXImportGDTFWheel* wheel = &wheels[i];
		FString WheelName = ObjectTools::SanitizeObjectName(wheel->Name.ToString());

		// For each slot
		for (int j=0; j < wheel->Slots.Num(); j++) {

			FDMXImportGDTFWheelSlot* slot = &wheel->Slots[j];
			WheelType Type = this->GetWheelType(FixtureAsset, wheel->Name);
			FString SlotName = ObjectTools::SanitizeObjectName(slot->Name.ToString());

			FString TexturePath = UPackageTools::SanitizePackageName(this->Package->GetName() + TEXT("/") + WheelName + TEXT("/") + SlotName);
			
			// Load of the texture from Content Browser
			UTexture2D* Texture = LoadObject<UTexture2D>(NULL, *TexturePath, nullptr, LOAD_Quiet | LOAD_NoWarn);
			if (!Texture) {

				UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Texture '%s/%s' not found."), *wheel->Name.ToString(), *slot->Name.ToString());

				if (Type == WheelType::Color) continue;
				else {
					UObject* OpenGobo = FCPGDTFImporterUtils::LoadObjectByPath(FString(FCPGDTFImporterUtils::CLAYPAKY_PLUGIN_CONTENT_BASEPATH).Append("MaterialInstances/Textures/T_Circle_01.T_Circle_01")); //Open gobo path
					if (OpenGobo != nullptr) Texture = Cast<UTexture2D>(OpenGobo);
				}
			}

			// Link the texture to the GDTFDescription
			slot->MediaFileName = Texture;
		}
	}

	Cast<UDMXImportGDTFWheels>(FixtureAsset->Wheels)->Wheels = wheels;
}

/**
 * Import the GDTF textures related to a specific wheel if applicable
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 may 2022
 *
 * @param WheelXML XML Node representing the wheel
 */
bool FCPGDTFWheelImporter::ImportWheel(const FXmlNode* WheelXML) {

	FString WheelName = WheelXML->GetAttribute("Name");
	bool bEverythingOK = true; // Flag to not stop the import but alert for an error

	const FXmlNode* WheelSlot = WheelXML->GetFirstChildNode();
	while (WheelSlot != nullptr) {

		// If the slot doesn't have a texture associated we continue
		if (WheelSlot->GetAttribute("MediaFileName").Equals("")) {
			
			WheelSlot = WheelSlot->GetNextNode();
			continue;
		}
		
		UTexture2D* texture = FCPGDTFImporterUtils::ImportPNG(this->GDTFPath, WheelSlot->GetAttribute("Name"), TEXT("wheels/") + WheelSlot->GetAttribute("MediaFileName"), this->Package->GetName() + TEXT("/") + WheelName);
		if (texture == nullptr) {
			bEverythingOK = false;
		} else {
			texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
			texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			texture->SRGB = false;
			texture->MaxTextureSize = CP_GOBO_SIZE;
			texture->UpdateResource();
			texture->GetPackage()->SetDirtyFlag(true);
		}

		WheelSlot = WheelSlot->GetNextNode();
	}

	return bEverythingOK;
}

/**
 * Automaticaly generate the Wheels disks textures
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 july 2022
 *
 * @param FixtureDescription GDTFDescription of the Fixture
 * @param FixturePackagePath Path of the fixture folder on Content Browser
*/
void FCPGDTFWheelImporter::CreateGDTFWheelsTextures(UCPGDTFDescription* FixtureDescription, FString FixturePackagePath) {

	if (FixturePackagePath[FixturePackagePath.Len()-1] == '/') FixturePackagePath.RemoveAt(FixturePackagePath.Len()-1);
	FixturePackagePath.Append("/textures/");
	for (FDMXImportGDTFWheel Wheel : Cast<UDMXImportGDTFWheels>(FixtureDescription->Wheels)->Wheels) {

		WheelType Type = FCPGDTFWheelImporter::GetWheelType(FixtureDescription, Wheel.Name);
		FString SavePath = FixturePackagePath + ObjectTools::SanitizeObjectName(Wheel.Name.ToString());
		if (Type == WheelType::Gobo) {
			
			TArray<UTexture2D*> Textures;
			for (FDMXImportGDTFWheelSlot Slot : Wheel.Slots) {
				if (Slot.MediaFileName != nullptr) Textures.Add(Slot.MediaFileName);
			}
			FCPGDTFWheelImporter::CreateGoboWheelTexture_Internal(SavePath, Textures);
		} else if (Type == WheelType::Color) {
			
			TArray<FLinearColor> Colors = FCPGDTFWheelImporter::GenerateColorArray(Wheel);
			FCPGDTFWheelImporter::CreateColorWheelTexture_Internal(SavePath, Colors);
		}
	}

}

/**
 * Find the Wheel type with GDTFDescription
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 july 2022
 *
 * @param FixtureDescription GDTF Description of a fixture
 * @param Name Name of the Wheel
 * @return Type of the Wheel
*/
FCPGDTFWheelImporter::WheelType FCPGDTFWheelImporter::GetWheelType(UCPGDTFDescription* FixtureDescription, FName Name) {

	for (FDMXImportGDTFDMXMode DMXMode : FixtureDescription->GetDMXModes()->DMXModes) {
		for (FDMXImportGDTFDMXChannel Channel : DMXMode.DMXChannels) {
			for (FDMXImportGDTFLogicalChannel LogicalChannel : Channel.LogicalChannels) {
				for (FDMXImportGDTFChannelFunction ChannelFunction : LogicalChannel.ChannelFunctions) {
					// If we found our Wheel
					if (ChannelFunction.Wheel.Name.ToString().Equals(Name.ToString())) {
						FString AttributeName = LogicalChannel.Attribute.Name.ToString();
						if (AttributeName.StartsWith("Color")) return WheelType::Color;
						else if (AttributeName.StartsWith("Gobo")) return WheelType::Gobo;
						else if (AttributeName.StartsWith("Prism")) return WheelType::Prism;
						else if (AttributeName.StartsWith("Animation")) return WheelType::Animation;
						else if (AttributeName.StartsWith("Effects")) return WheelType::Effects;
					}
				}
			}
		}
	}
	// Only if the wheel is not used by DMXChannels
	return WheelType::Effects;
}

/**
 * Create an Array of Colors for the construction of the Color Wheel disk
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 july 2022
 *
 * @param Wheel Description of the wheel constructed
 * @return The Array of Color for a specific wheel
*/
TArray<FLinearColor> FCPGDTFWheelImporter::GenerateColorArray(FDMXImportGDTFWheel Wheel) {
	
	TArray<FLinearColor> ColorsArray;

	for (FDMXImportGDTFWheelSlot Slot : Wheel.Slots) {
		if (Slot.MediaFileName == nullptr) ColorsArray.Add(FCPColorWizard::ColorCIEToRGB(Slot.Color));
		else { /// TODO \todo Test this part
			// If a color texture was given we read the color of the center pixel
			FLinearColor PixelColor = FLinearColor::White;
			int32 SizeX = Slot.MediaFileName->GetSizeX();
			int32 SizeY = Slot.MediaFileName->GetSizeY();
			uint32 CenterPixelIndex = ((SizeY / 2) * SizeX) + (SizeX / 2);
			Slot.MediaFileName->GetPixelFormat();

			FByteBulkData BulkData = Slot.MediaFileName->GetPlatformData()->Mips[0].BulkData;
			FColor* RawImageData = static_cast<FColor*>(BulkData.Lock(LOCK_READ_ONLY));
			PixelColor = FLinearColor(RawImageData[CenterPixelIndex]);
			BulkData.Unlock();
			ColorsArray.Add(PixelColor);
		}
	}
	return ColorsArray;
}

/**
 * Create a texture for a given array of SubTextures
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 12 july 2022
 *
 * @param SavePath		Path of the texture on the content browser
 * @param TexturesArray Array of textures used to build the wheel disk
*/
void FCPGDTFWheelImporter::CreateGoboWheelTexture_Internal(FString SavePath, TArray<UTexture2D*> TexturesArray) {

	int16 TextureSizeX = CP_GOBO_SIZE * TexturesArray.Num();
	
	// Generation of the Texture pixels
	uint8* Pixels = new uint8[TextureSizeX * CP_GOBO_SIZE * 4];

	// Creation of a full black texture
	for (int i = 0; i < TextureSizeX * CP_GOBO_SIZE * 4; i += 4) {
		Pixels[i  ] = 0;
		Pixels[i+1] = 0;
		Pixels[i+2] = 0;
		Pixels[i+3] = 255; // Alpha
	}
	// Loop on gobos
	for (int GoboIndex = 0; GoboIndex < TexturesArray.Num(); GoboIndex++) {

		FTexture2DMipMap GoboMip = TexturesArray[GoboIndex]->GetPlatformData()->Mips[0];
		if (GoboMip.SizeX != CP_GOBO_SIZE) {
			UE_LOG_CPGDTFIMPORTER(Warning, TEXT("We do not support gobos textures other than CP_GOBO_SIZE x CP_GOBO_SIZE px. Please provide CP_GOBO_SIZE x CP_GOBO_SIZE px or more for %s texture on GDTF."), *TexturesArray[GoboIndex]->GetName());
			continue;
		}

		int GoboX = 0, GoboY = 0;
		uint8* GoboPixels = (uint8*)GoboMip.BulkData.Lock(LOCK_READ_ONLY);

		if (GoboPixels == nullptr) {
			UE_LOG_CPGDTFIMPORTER(Error, TEXT("Unable to read %s Gobo pixels."), *TexturesArray[GoboIndex]->GetName());
			GoboMip.BulkData.Unlock();
			continue;
		}

		// Loop on columns
		for (int32 PixelsX = CP_GOBO_SIZE * GoboIndex; PixelsX < (CP_GOBO_SIZE * (GoboIndex + 1)); PixelsX++) {
			GoboY = 0;
			// Loop on rows
			for (int32 PixelsY = 0; PixelsY < CP_GOBO_SIZE; PixelsY++) {
				int32 curPixelIndex = ((PixelsY * TextureSizeX) + PixelsX);
				int32 curGoboPixelIndex = ((GoboY * CP_GOBO_SIZE) + GoboX);
				Pixels[4 * curPixelIndex + 2] = GoboPixels[4 * curGoboPixelIndex + 2]; // Red;
				Pixels[4 * curPixelIndex + 1] = GoboPixels[4 * curGoboPixelIndex + 1];// Green;
				Pixels[4 * curPixelIndex    ] = GoboPixels[4 * curGoboPixelIndex];   // Blue;
				Pixels[4 * curPixelIndex + 3] = 255; // Alpha;
				GoboY++;
			}
			GoboX++;
		}
		GoboMip.BulkData.Unlock();
	}

	FCPGDTFWheelImporter::SaveWheelToTexture_Internal(Pixels, SavePath, TextureSizeX, CP_GOBO_SIZE, false);

	// Creation of the frosted version because frost operation is very consuming
	FCPGDTFWheelImporter::FrostGoboWheelTexture_Internal(Pixels, TextureSizeX, CP_GOBO_SIZE, 4); /// TODO \todo Is a frost strenght value available in GDTF ??. LS: Linear frost not implemented
	FCPGDTFWheelImporter::SaveWheelToTexture_Internal(Pixels, SavePath, TextureSizeX, CP_GOBO_SIZE, true);
	delete[] Pixels;
}

/**
 * Frost the Gobo Wheel Texture
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 14 july 2022
 * 
 * @param PixelsArray Pixels to frost
 * @param SizeX Size of the Texture
 * @param SizeY Size of the Texture
 * @param Strenght Strenght of the frost
*/
void FCPGDTFWheelImporter::FrostGoboWheelTexture_Internal(uint8* PixelsArray, int SizeX, int SizeY, float Strenght) {

	// We need to copy the input pixels to a smaller one because the Gaussian Blur algorithm only support RGB and we have RGBA
	int* InPixelsBlur  = new int[SizeX * SizeY * 3];
	int* OutPixelsBlur = new int[SizeX * SizeY * 3];

	// Copy into In array
	for (int PixelX = 0; PixelX < SizeX; PixelX++) {
		for (int PixelY = 0; PixelY < SizeY; PixelY++) {
			int PixelArrayAddr = ((PixelY * SizeX) + PixelX);
			InPixelsBlur[PixelArrayAddr * 3    ] = PixelsArray[PixelArrayAddr * 4];
			InPixelsBlur[PixelArrayAddr * 3 + 1] = PixelsArray[PixelArrayAddr * 4 + 1];
			InPixelsBlur[PixelArrayAddr * 3 + 2] = PixelsArray[PixelArrayAddr * 4 + 2];
		}
	}

	fast_gaussian_blur_rgb(InPixelsBlur, OutPixelsBlur, SizeX, SizeY, 3, Strenght);
	delete[] InPixelsBlur;

	//Copy the out array into PixelsArray
	for (int PixelX = 0; PixelX < SizeX; PixelX++) {
		for (int PixelY = 0; PixelY < SizeY; PixelY++) {
			int PixelArrayAddr = ((PixelY * SizeX) + PixelX);
			PixelsArray[PixelArrayAddr * 4] = OutPixelsBlur[PixelArrayAddr * 3];
			PixelsArray[PixelArrayAddr * 4 + 1] = OutPixelsBlur[PixelArrayAddr * 3 + 1];
			PixelsArray[PixelArrayAddr * 4 + 2] = OutPixelsBlur[PixelArrayAddr * 3 + 2];
		}
	}
	delete[] OutPixelsBlur;
}

/**
 * Create a texture for a given array of Colors
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 06 july 2022
 *
 * @param SavePath		Path of the texture on the content browser
 * @param ColorsArray Array of colors used to build the wheel disk
 */
void FCPGDTFWheelImporter::CreateColorWheelTexture_Internal(FString SavePath, TArray<FLinearColor> ColorsArray) {

	#define CP_COLOR_SIZE 16

	int X = ColorsArray.Num() * CP_COLOR_SIZE;
	
	// Generation of the Texture pixels
	uint8* Pixels = new uint8[X * CP_COLOR_SIZE * 4];

	// Loop on colors
	for (int ColorIndex = 0; ColorIndex < ColorsArray.Num(); ColorIndex++) {
		// Loop on columns
		for (int32 x = CP_COLOR_SIZE * ColorIndex; x < CP_COLOR_SIZE * (ColorIndex + 1); x++) {
			// Loop on rows
			for (int y = 0; y < CP_COLOR_SIZE; y++) {
				int32 curPixelIndex = ((y * X) + x);
				Pixels[4 * curPixelIndex + 2] = ColorsArray[ColorIndex].R * 255;   // Red;
				Pixels[4 * curPixelIndex + 1] = ColorsArray[ColorIndex].G * 255;  // Green;
				Pixels[4 * curPixelIndex    ] = ColorsArray[ColorIndex].B * 255; // Blue;
				Pixels[4 * curPixelIndex + 3] = 255; // Alpha;
			}
		}
	}

	FCPGDTFWheelImporter::SaveWheelToTexture_Internal(Pixels, SavePath, X, CP_COLOR_SIZE);
	delete[] Pixels;
	#undef CP_COLOR_SIZE
}

/**
 * Create a texture for a given array of Pixels
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 12 july 2022
 * 
 * @param PixelsArray Pixels of the future texture
 * @param SavePath Path in the ContentBrowser
 * @param SizeX Size of the texture
 * @param SizeY Size of the texture
 * @param bIsFrosted 
*/
void FCPGDTFWheelImporter::SaveWheelToTexture_Internal(uint8* PixelsArray, FString SavePath, int SizeX, int SizeY, bool bIsFrosted) {

	// Generate the save package
	if (SavePath[SavePath.Len()-1] == '/') SavePath.RemoveAt(SavePath.Len()-1);
	int Index;
	SavePath.FindLastChar('/', Index);
	FString AssetName = FString("Wheel_").Append(SavePath.Mid(Index + 1));
	if (bIsFrosted) AssetName.Append("_Frosted");
	SavePath = SavePath.Append("/" + AssetName);
	UE_LOG_CPGDTFIMPORTER(Error, TEXT("Saving wheel texture to: '%s'"), *SavePath);
	UPackage* Package = CreatePackage(*SavePath);
	
	// Creation of the UTexture2D object
	UTexture2D* Texture = NewObject<UTexture2D>(Package, *AssetName, RF_Public | RF_Standalone);
	Texture->SetPlatformData(new FTexturePlatformData());	// Then we initialize the PlatformData
	Texture->GetPlatformData()->SizeX = SizeX;
	Texture->GetPlatformData()->SizeY = SizeY;
	Texture->GetPlatformData()->SetNumSlices(1);
	Texture->GetPlatformData()->PixelFormat = EPixelFormat::PF_B8G8R8A8;

	
	Texture->Source.Init(SizeX, SizeY, 1, 1, ETextureSourceFormat::TSF_BGRA8, PixelsArray);

	Texture->LODBias = 0;
	Texture->AdjustMinAlpha = 1.0;
	Texture->CompressionNoAlpha = true;
	Texture->SRGB = true;
	Texture->bGlobalForceMipLevelsToBeResident = true;
	Texture->UpdateResource();
	
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Texture);
}

#undef CP_GOBO_SIZE
