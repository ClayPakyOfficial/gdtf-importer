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


#include "CPFActorComponentsLoader.h"
#include "ClayPakyGDTFImporterLog.h"

#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Internationalization/Regex.h"
#include "Engine/SimpleConstructionScript.h"

#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFSubstractiveColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFAdditiveColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFHSVColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFCIEColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorCorrection/CPGDTFCTOFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFColorWheelFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFGoboWheelFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFShutterFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFFrostFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFMovementFixtureComponent.h"
#include "Components/DMXComponents/SimpleAttribute/CPGDTFDimmerFixtureComponent.h"
#include "Components/DMXComponents/SimpleAttribute/CPGDTFZoomFixtureComponent.h"

/**
 * Setup the actor's DMXComponents for a given DMXNode.
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 30 june 2022
 * 
 * @param Actor Actor to attach the components
 * @param DMXMode DMXMode to instanciate
 * @return True if no problem occured
*/
bool FCPFActorComponentsLoader::LoadDMXComponents(ACPGDTFFixtureActor* Actor, FDMXImportGDTFDMXMode DMXMode) {

	bool ReturnFlag = LoadMultipleAttributesComponents_INTERNAL(Actor, &DMXMode.DMXChannels);
	LoadSimpleAttributeComponents_INTERNAL(Actor, &DMXMode.DMXChannels);
	return ReturnFlag;
}

/**
 * Remove all the DMXComponents for a given actor.
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 09 september 2022
 *
 * @param Actor Actor to purge
 */
void FCPFActorComponentsLoader::PurgeDMXComponents(ACPGDTFFixtureActor* Actor) {

	Actor->UnregisterAllComponents();
	Actor->ClearInstanceComponents(true);
	Actor->ClearInstanceComponents(false);
	for (UCPGDTFFixtureComponentBase* DMXComponent : TInlineComponentArray<UCPGDTFFixtureComponentBase*>(Actor)) {
		
		Actor->RemoveOwnedComponent(DMXComponent);
		Actor->RemoveInstanceComponent(DMXComponent);
		DMXComponent->DestroyComponent();
		DMXComponent->ConditionalBeginDestroy();
	}
	Actor->UnregisterAllComponents();
	Actor->ClearInstanceComponents(true);
	Actor->ClearInstanceComponents(false);
	//Actor->GetWorld()->ForceGarbageCollection(true);
	GEngine->ForceGarbageCollection();
}

void FCPFActorComponentsLoader::PurgeAllComponents(ACPGDTFFixtureActor* Actor) {
	UClass* cls = Actor->GetClass();
	UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>(cls);
	USimpleConstructionScript* scs = nullptr;
	if (bpClass) {
		bpClass->SimpleConstructionScript;
		scs = bpClass->SimpleConstructionScript;
	}

	FCPFActorComponentsLoader::PurgeDMXComponents(Actor);
	Actor->GeometryTree.DestroyGeometryTree();

	if (scs) {
		TArray<USCS_Node*> allNodes = scs->GetAllNodes();
		TArray<USCS_Node*> rootNodes = scs->GetRootNodes();
		TArray<USCS_Node*> noRootNodes = allNodes;
		for (USCS_Node* node : rootNodes)
			noRootNodes.Remove(node);
		for (USCS_Node* node : noRootNodes)
			scs->AddNode(node); //limortaccituaResetAll method
		for (USCS_Node* node : allNodes)
			scs->RemoveNode(node);
	}

	GEditor->ForceGarbageCollection(true);
}

/**
 * Setup the actor's DMXComponents using one Attribute.
 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
 * @date 30 june 2022
 *
 * @param Actor Actor to attach the components
 * @param DMXChannels DMXChannels to instanciate
*/
void FCPFActorComponentsLoader::LoadSimpleAttributeComponents_INTERNAL(ACPGDTFFixtureActor* Actor, TArray<FDMXImportGDTFDMXChannel>* DMXChannels) {

	int attrIndexes[((int) ECPGDTFAttributeType::DIMENSION)] = { 0 };

	for (FDMXImportGDTFDMXChannel DMXChannel : *DMXChannels) {

		TSubclassOf<UCPGDTFSimpleAttributeFixtureComponent> ComponentClass = nullptr;
		ECPGDTFAttributeType DMXChannelAttributeType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(DMXChannel.LogicalChannels[0].Attribute.Name.ToString());

		switch (DMXChannelAttributeType) {

		case ECPGDTFAttributeType::Pan:
		case ECPGDTFAttributeType::Tilt:
			//ComponentClass = UCPGDTFMovementFixtureComponent::StaticClass();
			break;
		case ECPGDTFAttributeType::Dimmer:
			ComponentClass = UCPGDTFDimmerFixtureComponent::StaticClass();
			break;

		case ECPGDTFAttributeType::Zoom:
			ComponentClass = UCPGDTFZoomFixtureComponent::StaticClass();
			break;
		default:
			break;
		}

		if (ComponentClass != nullptr) {
			FName DMXChannelName = FName(DMXChannel.LogicalChannels[0].Attribute.Pretty/* + TEXT("_KEK")*/);
			UCPGDTFSimpleAttributeFixtureComponent* Component = NewObject<UCPGDTFSimpleAttributeFixtureComponent>(Actor, ComponentClass, DMXChannelName);

			if (Component != nullptr) {
				Component->Setup(DMXChannel, attrIndexes[((int) DMXChannelAttributeType)]++);
				Component->bUseInterpolation = (uint8)((ECPGDTFDescriptionSnap)DMXChannel.LogicalChannels[0].Snap) & 1; // We use interpolation if snap disabled.
				Component->OnComponentCreated();
				Actor->AddInstanceComponent(Component);
				Component->Activate();
				//Component->RegisterComponent(); //TODO This causes a "Ensure condition failed: MyOwnerWorld" exception
			}
		}
	}
}

/**
 * Setup the actor's DMXComponents using multiple Attributes.
 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
 * @date 30 june 2022
 *
 * @param Actor Actor to attach the components
 * @param DMXChannels DMXChannels to instanciate
 * @return True if no problem occured
*/
bool FCPFActorComponentsLoader::LoadMultipleAttributesComponents_INTERNAL(ACPGDTFFixtureActor* Actor, TArray<FDMXImportGDTFDMXChannel>* DMXChannels) {

	/*
	Regex notes:
	
	'+' match one or more of the preceding token
	'*' match zero or more of the preceding token

	To match one digit or more: "[\\d]+" WARNING: please replace by "{0}"
	To not match if a word end by a specific suffix "(?<!suffix)$"
	*/

	//Each name MUST end with '{1}'!!!! This is to be sure to get different channels even when they have the same name
	TTuple<TSubclassOf<UCPGDTFMultipleAttributeFixtureComponent>, FString> MultipleAttributeComponentsClasses[] = {
		{UCPGDTFAdditiveColorSourceFixtureComponent::StaticClass(), "^ColorAdd_.*{1}$"},      // WARNING: This use Regular Expressions !!!!
		{UCPGDTFSubstractiveColorSourceFixtureComponent::StaticClass(), "^ColorSub_.*{1}$"}, // Use https://regexr.com/ to test them
		{UCPGDTFCIEColorSourceFixtureComponent::StaticClass(), "^CIE_.*{1}$"},  // For numbered attributes replace the number by '{0}'
		{UCPGDTFHSVColorSourceFixtureComponent::StaticClass(), "^HSB_.*{1}$"}, //  it will be replaced by the corresponding numbers in the loop below
		/******************** End Color Sources ****************************/

 		{UCPGDTFCTOFixtureComponent::StaticClass(), "^CTO{1}$"},
		{UCPGDTFColorWheelFixtureComponent::StaticClass(), "^((Color)|(ColorMacro)){0}.*((?<!Mode)(?<!Rate)){1}$"},
		/******************** End Color Wheels *****************************/

		{UCPGDTFIrisFixtureComponent::StaticClass(), "^Iris(?:StrobeRandom|Strobe|PulseClose|PulseOpen|RandomPulseClose|RandomPulseOpen|Iris|Mode|MSpeed)*{1}$"},
		{UCPGDTFGoboWheelFixtureComponent::StaticClass(), "^Gobo{0}.*(?<!Mode){1}$"},
		{UCPGDTFFrostFixtureComponent::StaticClass(), "^Frost{0}.*(?<!MSpeed){1}$"},
		{UCPGDTFShutterFixtureComponent::StaticClass(), "^(Shutter{0}.*{1})|(StrobeMode.+{1})|(StrobeFrequency{1})$"},
		{UCPGDTFShaperFixtureComponent::StaticClass(), "^(Blade(?:Soft)*{0}(?:A|B|Rot){1})|(Shaper(?:Rot|Macros|MacrosSpeed){1})$"},
		{UCPGDTFMovementFixtureComponent::StaticClass(), "^(?:Pan|Tilt)(?:Rotate)*{1}$"}
	};

	//Copy the channels adding an index to differentiate between channels with same name
	TArray<FDMXImportGDTFDMXChannel> lclChannels;
	TMap<FString, int32> counters;
	for (int i = 0; i < DMXChannels->Num(); i++) {
		FDMXImportGDTFDMXChannel chCopy = (*DMXChannels)[i];
		chCopy.LogicalChannels.Empty();
		for (int j = 0; j < (*DMXChannels)[i].LogicalChannels.Num(); j++) {
			FDMXImportGDTFLogicalChannel lCopy = (*DMXChannels)[i].LogicalChannels[j];
			FString name = lCopy.Attribute.Name.ToString();

			int32* count = counters.Find(name);
			int32 cnt = 1;
			if (count != nullptr)
				cnt = *count + 1;
			counters.Add(name, cnt);

			lCopy.Attribute.Name = FName(name + TEXT("_") + FString::FromInt(cnt));
			chCopy.LogicalChannels.Add(lCopy);
		}
		lclChannels.Add(chCopy);
	}

	for (TTuple<TSubclassOf<UCPGDTFMultipleAttributeFixtureComponent>, FString> ComponentDefinition : MultipleAttributeComponentsClasses) {

		TSet<int> DMXChannelsExtractedPrev;
		TSet<int> DMXChannelsIdxExtracted;
		TArray<FDMXImportGDTFDMXChannel> DMXChannelsExtracted;
		int AttributeIndex = 1; // Used to load correctly the numberred attributes
		int SetIndex = 1;

		while (true) { // We loop because we can find multiple time the same type. For example GoboWheels or multiple RGB... channels.

			DMXChannelsIdxExtracted = FindDMXChannelsIdxByAttributeTypePattern(FString::Format(*(ComponentDefinition.Value), {AttributeIndex, TEXT("_") + FString::FromInt(SetIndex)}), lclChannels);
			if (DMXChannelsIdxExtracted.Num() < 1) break; // We quit the loop
			//If the two sets are identical, we have obtained the same DMXChannelsExtracted array, so we try obtaining a different set with the same channel names
			if (DMXChannelsIdxExtracted.Difference(DMXChannelsExtractedPrev).IsEmpty() && DMXChannelsExtractedPrev.Difference(DMXChannelsIdxExtracted).IsEmpty()) {
				SetIndex++;
				continue;
			}
			DMXChannelsExtractedPrev = DMXChannelsIdxExtracted;

			DMXChannelsExtracted.Empty();
			for (int idx : DMXChannelsIdxExtracted) //We have to exctract first the indexes and then convert them because we can't store a FDMXImportGDTFDMXChannel in a TSet (used later)
				DMXChannelsExtracted.Add((*DMXChannels)[idx]);

			FString ComponentName = DMXChannelsExtracted[0].Geometry.ToString();
			ComponentName.Append("_");
			for (FDMXImportGDTFDMXChannel chan : DMXChannelsExtracted)
				ComponentName.Append(chan.LogicalChannels[0].Attribute.Pretty);

			UCPGDTFMultipleAttributeFixtureComponent* NewComponent = NewObject<UCPGDTFMultipleAttributeFixtureComponent>(Actor, ComponentDefinition.Key, FName(ComponentName));
			bool setupSuccessfull = NewComponent->Setup(DMXChannelsExtracted, AttributeIndex - 1);
			if (setupSuccessfull) {
				NewComponent->OnComponentCreated();
				Actor->AddInstanceComponent(NewComponent);
			} else NewComponent->DestroyComponent();

			AttributeIndex++;
		}
	}
	return true;
}

/**
 * Find all DMXChannels referencing a given Attribute name pattern. Also remove the found channels from the given array.
 * In the case where we find multiple channel for a unique Attribute Type (multiple ColorAdd_R for LED Beam moving head for example)
 * we ensure that all the returned Chanels are linked to the same geometry
 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
 * @date 30 june 2022
 *
 * @param AttributeNamePattern Regex pattern to look for specific Attributes.
 * @param DMXChannels Array where to look for.
 * @return Index of the channels found
*/
TSet<int> FCPFActorComponentsLoader::FindDMXChannelsIdxByAttributeTypePattern(FString AttributeNamePattern, TArray<FDMXImportGDTFDMXChannel> &DMXChannels) {
	
	TSet<int> returnSet;
	//FName CurrentGeometry = "";
	FRegexPattern Pattern = FRegexPattern(AttributeNamePattern);

	for (int i = 0; i < DMXChannels.Num(); i++) {
		FDMXImportGDTFDMXChannel DMXChannel = DMXChannels[i];
		FDMXImportGDTFDMXChannel DMXChannelCopy = DMXChannel;
		bool mustAdd = false;

		for (int j = 0; j < DMXChannelCopy.LogicalChannels.Num(); j++) {
			bool deleteLogical = false;
			
			FRegexMatcher RegexMatcher = FRegexMatcher(Pattern, DMXChannelCopy.LogicalChannels[j].Attribute.Name.ToString());
			if (RegexMatcher.FindNext()) { // If the Attribute Name of the Channel match with the pattern
				//if (CurrentGeometry.IsEqual(FName(""), ENameCase::CaseSensitive)) CurrentGeometry = DMXChannel.Geometry;
				//if (DMXChannel.Geometry.IsEqual(CurrentGeometry, ENameCase::CaseSensitive)) {
				mustAdd = true;
				//} else {
				//	deleteLogical = true;
				//}
			} else deleteLogical = true;

			if (deleteLogical) {
				DMXChannelCopy.LogicalChannels.RemoveAt(j);
				j--;
			}
		}

		if (mustAdd) returnSet.Add(i);
	}
	return returnSet;
}