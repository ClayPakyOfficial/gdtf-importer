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


#include "CPFActorComponentsLoader.h"
#include "ClayPakyGDTFImporterLog.h"

#include "Internationalization/Regex.h"

#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFSubstractiveColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFAdditiveColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFHSVColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorSource/CPGDTFCIEColorSourceFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/ColorCorrection/CPGDTFCTOFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFColorWheelFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFGoboWheelFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFShutterFixtureComponent.h"
#include "Components/DMXComponents/MultipleAttributes/CPGDTFFrostFixtureComponent.h"
#include "Components/DMXComponents/SimpleAttribute/CPGDTFMovementFixtureComponent.h"
#include "Components/DMXComponents/SimpleAttribute/CPGDTFDimmerFixtureComponent.h"
#include "Components/DMXComponents/SimpleAttribute/CPGDTFZoomFixtureComponent.h"

/**
 * Setup the actor's DMXComponents for a given DMXNode.
 * @author Dorian Gardes - Clay Paky S.P.A.
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
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 09 september 2022
 *
 * @param Actor Actor to purge
 */
void FCPFActorComponentsLoader::PurgeDMXComponents(ACPGDTFFixtureActor* Actor) {

	for (UCPGDTFFixtureComponentBase* DMXComponent : TInlineComponentArray<UCPGDTFFixtureComponentBase*>(Actor)) {
		
		Actor->RemoveInstanceComponent(DMXComponent);
		DMXComponent->DestroyComponent();
	}
}

/**
 * Setup the actor's DMXComponents using one Attribute.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 30 june 2022
 *
 * @param Actor Actor to attach the components
 * @param DMXChannels DMXChannels to instanciate
*/
void FCPFActorComponentsLoader::LoadSimpleAttributeComponents_INTERNAL(ACPGDTFFixtureActor* Actor, TArray<FDMXImportGDTFDMXChannel>* DMXChannels) {

	for (FDMXImportGDTFDMXChannel DMXChannel : *DMXChannels) {

		TSubclassOf<UCPGDTFSimpleAttributeFixtureComponent> ComponentClass = nullptr;
		ECPGDTFAttributeType DMXChannelAttributeType = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(DMXChannel.LogicalChannels[0].Attribute.Name.ToString());

		switch (DMXChannelAttributeType) {

		case ECPGDTFAttributeType::Pan:
		case ECPGDTFAttributeType::Tilt:
			ComponentClass = UCPGDTFMovementFixtureComponent::StaticClass();
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
			FName DMXChannelName = FName(DMXChannel.LogicalChannels[0].Attribute.Pretty);
			UCPGDTFSimpleAttributeFixtureComponent* Component = NewObject<UCPGDTFSimpleAttributeFixtureComponent>(Actor, ComponentClass, DMXChannelName);

			if (Component != nullptr) {
				Component->Setup(DMXChannel);
				Component->bUseInterpolation = (uint8)((ECPGDTFDescriptionSnap)DMXChannel.LogicalChannels[0].Snap) == 0; // We use interpolation if snap disabled.
				Component->OnComponentCreated();
				Actor->AddInstanceComponent(Component);
				Component->Activate();
				Component->RegisterComponent();
			}
		}
	}
}

/**
 * Setup the actor's DMXComponents using multiple Attributes.
 * @author Dorian Gardes - Clay Paky S.P.A.
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

	TTuple<TSubclassOf<UCPGDTFMultipleAttributeFixtureComponent>, FString> MultipleAttributeComponentsClasses[] =
		{ {UCPGDTFAdditiveColorSourceFixtureComponent::StaticClass(), "^ColorAdd_.*$"},      // WARNING: This use Regular Expressions !!!!
		  {UCPGDTFSubstractiveColorSourceFixtureComponent::StaticClass(), "^ColorSub_.*$"}, // Use https://regexr.com/ to test them
		  {UCPGDTFCIEColorSourceFixtureComponent::StaticClass(), "^CIE_.*$"},  // For numbered attributes replace the number by '{0}'
		  {UCPGDTFHSVColorSourceFixtureComponent::StaticClass(), "^HSB_.*$"}, //  it will be replaced by the corresponding numbers in the loop below
		  /******************** End Color Sources ****************************/

 		  {UCPGDTFCTOFixtureComponent::StaticClass(), "^CTO$"},
		  {UCPGDTFColorWheelFixtureComponent::StaticClass(), "^((Color)|(ColorMacro)){0}.*((?<!Mode)(?<!Rate))$"},
		  /******************** End Color Wheels *****************************/

		  {UCPGDTFGoboWheelFixtureComponent::StaticClass(), "^Gobo{0}.*(?<!Mode)$"},
		  {UCPGDTFFrostFixtureComponent::StaticClass(), "^Frost{0}.*(?<!MSpeed)$"},
		  {UCPGDTFShutterFixtureComponent::StaticClass(), "^(Shutter{0}.*)|(StrobeMode.+)|(StrobeFrequency)$"}};

	for (TTuple<TSubclassOf<UCPGDTFMultipleAttributeFixtureComponent>, FString> ComponentDefinition : MultipleAttributeComponentsClasses) {

		TArray<FDMXImportGDTFDMXChannel> DMXChannelsExtracted;
		DMXChannelsExtracted.Empty();
		int AttributeIndex = 1; // Used to load correctly the numberred attributes

		while (true) { // We loop because we can find multiple time the same type. For example GoboWheels or multiple RGB... channels.

			DMXChannelsExtracted = FindDMXChannelsByAttributeTypePattern(FString::Format(*(ComponentDefinition.Value), {AttributeIndex}), DMXChannels);
			if (DMXChannelsExtracted.Num() < 1) break; // We quit the loop

			FString ComponentName = DMXChannelsExtracted[0].Geometry.ToString();
			ComponentName.Append("_");
			for (FDMXImportGDTFDMXChannel chan : DMXChannelsExtracted) {
				ComponentName.Append(chan.LogicalChannels[0].Attribute.Pretty);
			}

			UCPGDTFMultipleAttributeFixtureComponent* NewComponent = NewObject<UCPGDTFMultipleAttributeFixtureComponent>(Actor, ComponentDefinition.Key, FName(ComponentName));
			NewComponent->Setup(DMXChannelsExtracted[0].Geometry, DMXChannelsExtracted);
			NewComponent->OnComponentCreated();
			Actor->AddInstanceComponent(NewComponent);
			//NewComponent->RegisterComponent();

			// Ugly hotfix to avoid more than one wheel of each type loading
			/// TODO \todo Support multiple wheels to remove this hotfix
			if (ComponentDefinition.Key == UCPGDTFGoboWheelFixtureComponent::StaticClass()) break;
			if (ComponentDefinition.Key == UCPGDTFColorWheelFixtureComponent::StaticClass()) break;
			AttributeIndex++;
		}
	}
	return true;
}

/**
 * Find all DMXChannels referencing a given Attribute name pattern. Also remove the found channels from the given array.
 * In the case where we find multiple channel for a unique Attribute Type (multiple ColorAdd_R for LED Beam moving head for example)
 * we ensure that all the returned Chanels are linked to the same geometry
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 30 june 2022
 *
 * @param AttributeNamePattern Regex pattern to look for specific Attributes.
 * @param DMXChannels Array where to look for.
 * @return Channels found
*/
TArray<FDMXImportGDTFDMXChannel> FCPFActorComponentsLoader::FindDMXChannelsByAttributeTypePattern(FString AttributeNamePattern, TArray<FDMXImportGDTFDMXChannel>* DMXChannels) {
	
	TArray<FDMXImportGDTFDMXChannel> ReturnArray;
	FName CurrentGeometry = "";
	FRegexPattern Pattern = FRegexPattern(AttributeNamePattern);

	for (int i = 0; i < DMXChannels->Num(); i++) {
		FDMXImportGDTFDMXChannel DMXChannel = (*DMXChannels)[i];
		FRegexMatcher RegexMatcher = FRegexMatcher(Pattern, DMXChannel.LogicalChannels[0].Attribute.Name.ToString());

		if (RegexMatcher.FindNext()) { // If the Attribute Name of the Channel match with the pattern
			if (CurrentGeometry.IsEqual(FName(""), ENameCase::CaseSensitive)) CurrentGeometry = DMXChannel.Geometry;
			if (DMXChannel.Geometry.IsEqual(CurrentGeometry, ENameCase::CaseSensitive)) {
				ReturnArray.Add(DMXChannel);
				DMXChannels->RemoveAt(i);
				i--; // We removed the current array cell so we don't want to iterate to the next cell.
			}
		}
	}
	return ReturnArray;
}
