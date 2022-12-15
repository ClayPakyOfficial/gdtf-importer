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
#include "AssetTypeCategories.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/// Base class of the Plugin.<br> Used by Unreal to start and stop it.
class CLAYPAKYGDTFIMPORTER_API FClayPakyGDTFImporterModule : public IModuleInterface
{

public:
	
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static inline FClayPakyGDTFImporterModule& Get() { return FModuleManager::LoadModuleChecked< FClayPakyGDTFImporterModule >("ClayPakyGDTFImporter"); }

	static inline bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("ClayPakyGDTFImporter"); }
	
	static inline EAssetTypeCategories::Type GetAssetCategory() { return ClayPakyGDTFImporterAssetCategory; }

	// The ClayPakyGDTFImporterModule name
	static inline const FName ModuleName = FName("ClayPakyGDTFImporter");

	// The ClayPakyGDTFImporter asset category
	static inline EAssetTypeCategories::Type ClayPakyGDTFImporterAssetCategory;
	
};