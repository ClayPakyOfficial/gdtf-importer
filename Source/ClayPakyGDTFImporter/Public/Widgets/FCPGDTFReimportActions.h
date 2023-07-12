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
#include "CPGDTFFixtureActor.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"

/// Add re-import buttons in context menu for GDTF descriptions
class FCPGDTFReimportActions : public FAssetTypeActions_Base {

	//~ Begin FAssetTypeActions_Base Interface
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual FColor GetTypeColor() const override { return FColor(252, 195, 0); } // Clay Paky Yellow
	virtual uint32 GetCategories() override { return EAssetTypeCategories::UI; }
	//~ End FAssetTypeActions_Base Interface
};

class FCPGDTFActorsReimportActions : public FCPGDTFReimportActions {

	virtual UClass* GetSupportedClass() const override;
	virtual FText GetName() const override { return FText::FromString("BluePrint based on ACPGDTFFixtureActor class"); }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
};

class FCPGDTFDescriptionsReimportActions : public FCPGDTFReimportActions {

	virtual UClass* GetSupportedClass() const override;
	virtual FText GetName() const override { return FText::FromString("Clay Paky GDTF Description"); }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
};