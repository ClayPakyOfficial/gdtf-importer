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
#include "InputCoreTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"

class SButton;
class UCPGDTFImportUI;
class IDetailsView;

/// Class describing the import popup
class SCPGDTFOptionWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS( SCPGDTFOptionWindow )
    : _ImportUI(nullptr)
    , _WidgetWindow()
    , _FullPath()
    , _MaxWindowHeight(0.0f)
    , _MaxWindowWidth(0.0f)
    {}

    SLATE_ARGUMENT( UCPGDTFImportUI*, ImportUI )
    SLATE_ARGUMENT( TSharedPtr<SWindow>, WidgetWindow )
    SLATE_ARGUMENT( FText, FullPath )
    SLATE_ARGUMENT( float, MaxWindowHeight)
    SLATE_ARGUMENT(float, MaxWindowWidth)
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs);
    virtual bool SupportsKeyboardFocus() const override { return true; }

    static void GetImportOptions(UCPGDTFImportUI* ImportUI, const FString& FullPath, bool& OutOperationCanceled, bool& bOutImportAll);

    FReply OnImport()
    {
        bShouldImport = true;
        if ( WidgetWindow.IsValid() )
        {
            WidgetWindow.Pin()->RequestDestroyWindow();
        }
        return FReply::Handled();
    }

    FReply OnImportAll()
    {
        bShouldImportAll = true;
        return OnImport();
    }

    FReply OnCancel()
    {
        bShouldImport = false;
        bShouldImportAll = false;
        if ( WidgetWindow.IsValid() )
        {
            WidgetWindow.Pin()->RequestDestroyWindow();
        }
        return FReply::Handled();
    }

    virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override
    {
        if( InKeyEvent.GetKey() == EKeys::Escape )
        {
            return OnCancel();
        }

        return FReply::Unhandled();
    }

    bool ShouldImport() const
    {
        return bShouldImport;
    }

    bool ShouldImportAll() const
    {
        return bShouldImportAll;
    }

    SCPGDTFOptionWindow()
        : ImportUI(nullptr)
        , bShouldImport(false)
        , bShouldImportAll(false)
    {}

private:

    FReply OnResetToDefaultClick() const;
    FText GetImportTypeDisplayText() const;


private:
    TWeakObjectPtr<UCPGDTFImportUI> ImportUI;
    TSharedPtr<IDetailsView> DetailsView;
    TWeakPtr< SWindow > WidgetWindow;
    TSharedPtr< SButton > ImportButton;
    bool			bShouldImport;
    bool			bShouldImportAll;
};
