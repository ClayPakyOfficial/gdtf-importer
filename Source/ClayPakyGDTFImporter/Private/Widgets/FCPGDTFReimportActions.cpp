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

#include "Widgets/FCPGDTFReimportActions.h"
#include "EditorReimportHandler.h"
#include "ClayPakyGDTFImporterLog.h"

UClass* FCPGDTFActorsReimportActions::GetSupportedClass() const {

	/// TODO \todo This is not working. The actors are blueprints based inherited from ACPGDTFFixtureActor class 
	return TSubclassOf<ACPGDTFFixtureActor>(ACPGDTFFixtureActor::StaticClass()).Get();
}

UClass* FCPGDTFDescriptionsReimportActions::GetSupportedClass() const {

	return UCPGDTFDescription::StaticClass();
}

void FCPGDTFActorsReimportActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) {

	FText ButtonLabel = FText::FromString("Reimport");
	FText ButtonToolTip = FText::FromString("Reload this GDTF generated actor(s) based on GDTF on disk");
	
	TArray<TWeakObjectPtr<ACPGDTFFixtureActor>> GDTFActors = GetTypedWeakObjectPtrs<ACPGDTFFixtureActor>(InObjects);

	if (GDTFActors.Num() > 0) {
		//The lambda to call on click.		
		FExecuteAction Action = FExecuteAction::CreateLambda([](TArray<TWeakObjectPtr<ACPGDTFFixtureActor>> GDTFActorsWeakPtrs) {
				for (TWeakObjectPtr<ACPGDTFFixtureActor> ActorPtr : GDTFActorsWeakPtrs) {
					ACPGDTFFixtureActor* Actor = ActorPtr.Get();

					if (Actor != nullptr) {
						if (Actor->GDTFDescription != nullptr)
							FReimportManager::Instance()->Reimport(Actor->GDTFDescription, true);
					}
				}
			}, GDTFActors);
		MenuBuilder.AddMenuEntry(ButtonLabel, ButtonToolTip, FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"), FUIAction(Action));
	}
}

void FCPGDTFDescriptionsReimportActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) {

	FText ButtonLabel = FText::FromString("Reimport");
	FText ButtonToolTip = FText::FromString("Reload this GDTF descriptions based on GDTF on disk");

	TArray<TWeakObjectPtr<UCPGDTFDescription>> GDTFDescriptions = GetTypedWeakObjectPtrs<UCPGDTFDescription>(InObjects);

	if (GDTFDescriptions.Num() > 0) {
		//The lambda to call on click.		
		FExecuteAction Action = FExecuteAction::CreateLambda([](TArray<TWeakObjectPtr<UCPGDTFDescription>> GDTFDescriptionsWeakPtrs) {
			for (TWeakObjectPtr<UCPGDTFDescription> DescriptionPtr : GDTFDescriptionsWeakPtrs) {
				UCPGDTFDescription* Description = DescriptionPtr.Get();

				if (Description != nullptr) {
					FReimportManager::Instance()->Reimport(Description, true);
				}
			}
		}, GDTFDescriptions);
		MenuBuilder.AddMenuEntry(ButtonLabel, ButtonToolTip, FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"), FUIAction(Action));
	}
}
