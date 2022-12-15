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
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/CPGDTFImportUI.h"
#include "CPGDTFFactory.generated.h"

/// Factory to import GDTF files
UCLASS()
class UCPGDTFFactory : public UFactory , public FReimportHandler {

	GENERATED_BODY()

public:
	UCPGDTFFactory();

	/**  Set import batch **/
	void EnableShowOption() { bShowOption = true; }

	//~ Begin UObject Interface
	virtual void CleanUp() override;
	virtual bool ConfigureProperties() override;
	virtual void PostInitProperties() override;
	//~ End UObject Interface

	//~ Begin UFactory Interface
	virtual bool DoesSupportClass(UClass * Class) override;
	virtual UClass* ResolveSupportedClass() override;

	/**
	 * Import GDTF file in the Content Browser
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 04 may 2022
	 *
	 * @param InClass       UCPGDTFDescription - Class used to instantiate the object
	 * @param InParent      Path of the newly created asset in ContentBrowser
	 * @param InName        Name of the newly created asset in ContentBrowser
	 * @param Flags
	 * @param InFilename    Path of the GDTF file on disk
	 * @param Parms
	 * @param Warn
	 * @param bOutOperationCanceled
	 */
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& InFilename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled);
	virtual bool FactoryCanImport(const FString& Filename) override;
	//~ End UFactory Interface

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface

public:
	static const TCHAR* Extension;

private:

	/** If False the option window is not opened. */
	bool bShowOption;

	/** True if the import operation was canceled. */
	bool bOperationCanceled;

	UPROPERTY(transient)
	UCPGDTFImportUI* ImportUI;
};
