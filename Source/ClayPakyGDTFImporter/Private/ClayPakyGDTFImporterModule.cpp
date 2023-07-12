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

#include "ClayPakyGDTFImporterModule.h"
#include "CPGDTFDescription.h"
#include "Widgets/FCPGDTFReimportActions.h"
#include "Widgets/UCPGDTFDescriptionThumbnailRenderer.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "ClayPakyGDTFImporterModule"

#include "ClayPakyGDTFImporterLog.h"

void FClayPakyGDTFImporterModule::StartupModule() {

	// Registering of the DMXGDTFDescriptions thumbnails
	UThumbnailManager::Get().RegisterCustomRenderer(UCPGDTFDescription::StaticClass(), UCPGDTFDescriptionThumbnailRenderer::StaticClass());

	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().RegisterAssetTypeActions(MakeShareable(new FCPGDTFActorsReimportActions()));
	AssetToolsModule.Get().RegisterAssetTypeActions(MakeShareable(new FCPGDTFDescriptionsReimportActions()));

	FMath::RandInit(FDateTime::Now().ToUnixTimestamp());
}

void FClayPakyGDTFImporterModule::ShutdownModule() {}

IMPLEMENT_MODULE(FClayPakyGDTFImporterModule, ClayPakyGDTFImporter)

#undef LOCTEXT_NAMESPACE
