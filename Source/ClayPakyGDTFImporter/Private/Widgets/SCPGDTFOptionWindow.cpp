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

#include "Widgets/SCPGDTFOptionWindow.h"
#include "Widgets/CPGDTFImportUI.h"

#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include <Editor/MainFrame/Private/MainFrameModule.h>

#define LOCTEXT_NAMESPACE "SCPGDTFOptionWindow"

void SCPGDTFOptionWindow::Construct(const FArguments& InArgs)
{
    check (InArgs._ImportUI);

    ImportUI = InArgs._ImportUI;
    WidgetWindow = InArgs._WidgetWindow;

    TSharedPtr<SBox> ImportTypeDisplay;
    TSharedPtr<SHorizontalBox> HeaderButtons;
    TSharedPtr<SBox> InspectorBox;

    this->ChildSlot
    [
        SNew(SBox)
            .MaxDesiredHeight(InArgs._MaxWindowHeight)
            .MaxDesiredWidth(InArgs._MaxWindowWidth)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2)
            [
                SAssignNew(ImportTypeDisplay, SBox)
            ]
            +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2)
            [
                SNew(SBorder)
                    .Padding(FMargin(3))
                    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                        .AutoWidth()
                    [
                        SNew(STextBlock)
                            .Font(FAppStyle::GetFontStyle("CurveEd.LabelFont"))
                            .Text(LOCTEXT("Import_CurrentFileTitle", "Current Asset: "))
                    ]
                    +SHorizontalBox::Slot()
                        .Padding(5, 0, 0, 0)
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                            .Font(FAppStyle::GetFontStyle("CurveEd.InfoFont"))
                            .Text(InArgs._FullPath)
                            .ToolTipText(InArgs._FullPath)
                    ]
                ]
            ]
            + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2)
            [
                SAssignNew(InspectorBox, SBox)
                    .MaxDesiredHeight(650.0f)
                    .WidthOverride(400.0f)
            ]
            + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Right)
                .Padding(2)
            [
                SNew(SUniformGridPanel)
                    .SlotPadding(2)
                + SUniformGridPanel::Slot(1, 0)
                [
                    SNew(SButton)
                        .HAlign(HAlign_Center)
                        .Text(LOCTEXT("OptionWindow_ImportAll", "Import All"))
                        .ToolTipText(LOCTEXT("OptionWindow_ImportAll_ToolTip", "Import all files with these same settings"))
                        .OnClicked(this, &SCPGDTFOptionWindow::OnImportAll)
                ]
                + SUniformGridPanel::Slot(2, 0)
                [
                    SAssignNew(ImportButton, SButton)
                        .HAlign(HAlign_Center)
                        .Text(LOCTEXT("OptionWindow_Import", "Import"))
                        .OnClicked(this, &SCPGDTFOptionWindow::OnImport)
                ]
                + SUniformGridPanel::Slot(3, 0)
                [
                    SNew(SButton)
                        .HAlign(HAlign_Center)
                        .Text(LOCTEXT("OptionWindow_Cancel", "Cancel"))
                        .ToolTipText(LOCTEXT("OptionWindow_Cancel_ToolTip", "Cancels importing this file"))
                        .OnClicked(this, &SCPGDTFOptionWindow::OnCancel)
                ]
            ]
        ]
    ];

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

    InspectorBox->SetContent(DetailsView->AsShared());

    ImportTypeDisplay->SetContent(
        SNew(SBorder)
            .Padding(FMargin(3))
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                    .Text(this, &SCPGDTFOptionWindow::GetImportTypeDisplayText)
            ]
            + SHorizontalBox::Slot()
            [
                SNew(SBox)
                    .HAlign(HAlign_Right)
                [
                    SAssignNew(HeaderButtons, SHorizontalBox)
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(FMargin(2.0f, 0.0f))
                    [
                        SNew(SButton)
                            .Text(LOCTEXT("OptionWindow_ResetOptions", "Reset to Default"))
                            .OnClicked(this, &SCPGDTFOptionWindow::OnResetToDefaultClick)
                    ]
                ]
            ]
        ]
    );

    if (UCPGDTFImportUI* ImportUIPtr = ImportUI.Get())
    {
		DetailsView->SetObject(ImportUIPtr);
    }
}

FReply SCPGDTFOptionWindow::OnResetToDefaultClick() const
{
    if (UCPGDTFImportUI* ImportUIPtr = ImportUI.Get())
    {
        ImportUIPtr->ResetToDefault();
		//Refresh the view to make sure the custom UI are updating correctly
		DetailsView->SetObject(ImportUIPtr, true);
    }
    return FReply::Handled();
}

FText SCPGDTFOptionWindow::GetImportTypeDisplayText() const
{
    return FText::FromString(TEXT("Clay Paky GDTF Import"));
}

/*
 * Just open a windows with a given UI
 */
void SCPGDTFOptionWindow::GetImportOptions(UCPGDTFImportUI* ImportUI, const FString& FullPath, bool& OutOperationCanceled, bool& bOutImportAll) {
    
    OutOperationCanceled = false;
    TSharedPtr<SWindow> ParentWindow;

    if (FModuleManager::Get().IsModuleLoaded("MainFrame")) {

        IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
        ParentWindow = MainFrame.GetParentWindow();
    }

    // Compute centered window position based on max window size, which include when all categories are expanded
    const float ImportWindowWidth = 410.0f;
    const float ImportWindowHeight = 750.0f;
    FVector2D ImportWindowSize = FVector2D(ImportWindowWidth, ImportWindowHeight); // Max window size it can get based on current slate

    FSlateRect WorkAreaRect = FSlateApplicationBase::Get().GetPreferredWorkArea();
    FVector2D DisplayTopLeft(WorkAreaRect.Left, WorkAreaRect.Top);
    FVector2D DisplaySize(WorkAreaRect.Right - WorkAreaRect.Left, WorkAreaRect.Bottom - WorkAreaRect.Top);

    FVector2D WindowPosition = (DisplayTopLeft + (DisplaySize - ImportWindowSize) / 2.0f);

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(LOCTEXT("GDTFImportOpionsTitle", "ClayPaky GDTF importer"))
        .SizingRule(ESizingRule::Autosized)
        .AutoCenter(EAutoCenter::None)
        .ClientSize(ImportWindowSize)
        .ScreenPosition(WindowPosition);

    TSharedPtr<SCPGDTFOptionWindow> OptionWindow;
    Window->SetContent
    (
        SAssignNew(OptionWindow, SCPGDTFOptionWindow)
        .ImportUI(ImportUI)
        .WidgetWindow(Window)
        .FullPath(FText::FromString(FullPath))
        .MaxWindowHeight(ImportWindowHeight)
        .MaxWindowWidth(ImportWindowWidth)
    );

    // @todo: we can make this slow as showing progress bar later
    FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

    if (OptionWindow->ShouldImport()) bOutImportAll = OptionWindow->ShouldImportAll();
    else OutOperationCanceled = true;
}

#undef LOCTEXT_NAMESPACE
