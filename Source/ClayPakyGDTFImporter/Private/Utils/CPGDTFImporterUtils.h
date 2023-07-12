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
#include "CPGDTFDescription.h"
#include "Library/DMXImportGDTF.h"
#include "Widgets/Notifications/SNotificationList.h"

/**
 * GDTF XML Importer Utils
 */
class FCPGDTFImporterUtils {

public:

    /// Equals to "/ClayPakyGDTFImporter/"
    static constexpr const TCHAR* CLAYPAKY_PLUGIN_CONTENT_BASEPATH = TEXT("/ClayPakyGDTFImporter/");

    /**
     * Import a PNG from a GDTF file
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 06 may 2022
     *
     * @param GDTFPath  Path of the GDTF file on disk
     * @param AssetName Name of the imported Asset
     * @param FileName  Name of the png on GDTF archive
     * @param PathOnContentBrowser   Path of the imported Texture2D on the ContentBrowser
     */
	static UTexture2D* FCPGDTFImporterUtils::ImportPNG(FString GDTFPath, FString AssetName, FString FileName, FString PathOnContentBrowser);

    /**
     * Import a 3D model from a GDTF file
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 10 may 2022
     *
     * @param GDTFPath  Path of the GDTF file on disk
     * @param AssetName Name of the imported Asset
     * @param FileName  Name of the model on GDTF archive
     * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
     */
    static UObject* FCPGDTFImporterUtils::Import3DModel(FString GDTFPath, FString AssetName, FString FileName, FString PathOnContentBrowser);

    /**
     * Try to find the Asset in the Content Browser
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 10 may 2022
     *
     * @param AssetName Name of the imported Asset
     * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
     */
    static UObject* FCPGDTFImporterUtils::IsAssetExisting(FString AssetName, FString PathOnContentBrowser);

    /**
     * Create the package on a subfolder for an imported asset
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 10 may 2022
     *
     * @param AssetName Name of the imported Asset
     * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
     */
    static UPackage* FCPGDTFImporterUtils::PreparePackageOnSubFolder(FString AssetName, FString PathOnContentBrowser);

    /**
     * Create the package for an imported asset
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 10 may 2022
     *
     * @param AssetName Name of the imported Asset
     * @param PathOnContentBrowser   Path of the imported model on the ContentBrowser
     */
    static UPackage* FCPGDTFImporterUtils::PreparePackage(FString AssetName, FString PathOnContentBrowser);


    /**
     * Load all meshes from a folder on ContentBrowser.
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 27 may 2022
     *
     * @param ContentBrowserFolderPath Path to look for meshes
     * @return Array of UStaticMesh loaded from folder
    */
    static TArray<UStaticMesh*> LoadMeshesInFolder(FString ContentBrowserFolderPath);

    /**
     * Load a generic GDTF mesh.
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 01 june 2022
     *
     * @param Type Type of the mesh to load
     * @return Mesh or nullptr if EDMXImportGDTFPrimitiveType::Undefined provided
    */
    static UStaticMesh* LoadGDTFGenericMesh(ECPGDTFDescriptionModelsPrimitiveType Type);

    /**
     * Load an Asset stored in Content Browser.
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 08 june 2022
     *
     * @param Path Path of the Asset
     * @return Asset if found. nullptr else.
    */
    static UObject* LoadObjectByPath(FString Path);

    /**
     * Convert a GDTF "Position" to an Unreal FRotator.
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 24 may 2022
     *
     * @param InMatrix      Matrix to convert
     * @param OutRotator    Rotator who represent the relative rotation
     */
    static void MatrixToRotator(FMatrix InMatrix, FRotator* OutRotator);

    /**
     * Really needs documentation ??
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 08 july 2022
    */
    static double NearestPowerOfTwo(double N);

    /**
     * Send notification to the UI
     * @author Dorian Gardes - Clay Paky S.R.L.
     * @date 07 september 2022
     * 
     * @param Title Title of the notification
     * @param Message Message to show
     * @param State Success or failure
     * @param Duration Default value 3s
     * @param FadeInDuration Default value 0.5s
     * @param FadeOutDuration Default value 0.5s
     */
    static void SendNotification(FString Title, FString Message, SNotificationItem::ECompletionState State, float Duration = 5.0f, float FadeInDuration = 0.5f, float FadeOutDuration = 0.5f);
};

struct FCPDMXImportGDTFChannelFunction : public FDMXImportGDTFChannelFunction {
    FDMXImportGDTFChannelSet* getChannelSetByName(FString csName) {
        for (int i = 0; i < ChannelSets.Num(); i++)
            if (ChannelSets[i].Name.Equals(*csName))
                return &(ChannelSets[i]);
        return nullptr;
    }
};

struct FCPDMXImportGDTFLogicalChannel : public FDMXImportGDTFLogicalChannel {
    FDMXImportGDTFChannelFunction* getChannelFunctionByName(FString name) {
        TArray<FString> sp;
        name.ParseIntoArray(sp, TEXT("."), false);
        sp.RemoveAt(0); //name starts from the current element, remove it
        return getChannelFunctionByName(sp);
    }
    FDMXImportGDTFChannelFunction* getChannelFunctionByName(TArray<FString> sp) {
        FString cfName = sp[0]; //sp doesn't include the current element
        for (int i = 0; i < ChannelFunctions.Num(); i++)
            if (ChannelFunctions[i].Name.IsEqual(*cfName))
                return &(ChannelFunctions[i]);
        return nullptr;
    }

    FDMXImportGDTFChannelSet* getChannelSetByName(FString name) {
        TArray<FString> sp;
        name.ParseIntoArray(sp, TEXT("."), false);
        sp.RemoveAt(0); //name starts from the current element, remove it
        return getChannelSetByName(sp);
    }
    FDMXImportGDTFChannelSet* getChannelSetByName(TArray<FString> sp) {
        FCPDMXImportGDTFChannelFunction *cf = static_cast<FCPDMXImportGDTFChannelFunction*>(getChannelFunctionByName(sp)); //Since we don't have any new member in this structs, we can (theoretically) static_cast without undefined behaviours. Btw, we can't just use Cast since it doesn't support structs
        if (cf == nullptr) return nullptr;
        return cf->getChannelSetByName(sp[1]);
    }
};

struct FCPDMXImportGDTFDMXChannel : public FDMXImportGDTFDMXChannel {
    FDMXImportGDTFLogicalChannel* getLogicalChannelByName(FString name) {
        TArray<FString> sp;
        name.ParseIntoArray(sp, TEXT("."), false);
        sp.RemoveAt(0); //name starts from the current element, remove it
        return getLogicalChannelByName(sp);
    }
    FDMXImportGDTFLogicalChannel* getLogicalChannelByName(TArray<FString> sp) {
        FString logicalName = sp[0]; //sp doesn't include the current element
        for (int i = 0; i < LogicalChannels.Num(); i++)
            if (LogicalChannels[i].Attribute.Name.IsEqual(*logicalName))
                return &(LogicalChannels[i]);
        return nullptr;
    }

    FDMXImportGDTFChannelFunction* getChannelFunctionByName(FString name) {
        TArray<FString> sp;
        name.ParseIntoArray(sp, TEXT("."), false);
        sp.RemoveAt(0); //name starts from the current element, remove it
        return getChannelFunctionByName(sp);
    }
    FDMXImportGDTFChannelFunction* getChannelFunctionByName(TArray<FString> sp) {
        FCPDMXImportGDTFLogicalChannel* logical = static_cast<FCPDMXImportGDTFLogicalChannel*>(getLogicalChannelByName(sp));
        if (logical == nullptr) return nullptr;
        sp.RemoveAt(0);
        return logical->getChannelFunctionByName(sp);
    }

    FDMXImportGDTFChannelSet* getChannelSetByName(FString name) {
        TArray<FString> sp;
        name.ParseIntoArray(sp, TEXT("."), false);
        sp.RemoveAt(0); //name starts from the current element, remove it
        return getChannelSetByName(sp);
    }
    FDMXImportGDTFChannelSet* getChannelSetByName(TArray<FString> sp) {
        FCPDMXImportGDTFLogicalChannel* logical = static_cast<FCPDMXImportGDTFLogicalChannel*>(getLogicalChannelByName(sp));
        if (logical == nullptr) return nullptr;
        sp.RemoveAt(0);
        FCPDMXImportGDTFChannelFunction* cf = static_cast<FCPDMXImportGDTFChannelFunction*>(logical->getChannelFunctionByName(sp));
        if (cf == nullptr) return nullptr;
        return cf->getChannelSetByName(sp[1]);
    }
};
    