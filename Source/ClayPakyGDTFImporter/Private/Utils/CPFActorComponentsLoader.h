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

/**
 * GDTFFixtureActor's DMXComponents importer Utils
 */
class FCPFActorComponentsLoader {

public:

	/**
	 * Setup the actor's DMXComponents using one Attribute.
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 30 june 2022
	 *
	 * @param Actor Actor to attach the components
	 * @param DMXMode DMXMode to instanciate
	 * @return True if no problem occured
	 */
	static bool LoadDMXComponents(ACPGDTFFixtureActor* Actor, FDMXImportGDTFDMXMode DMXMode);

	/**
	 * Remove all the DMXComponents for a given actor.
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 09 september 2022
	 *
	 * @param Actor Actor to purge
	 */
	static void PurgeDMXComponents(ACPGDTFFixtureActor* Actor);

	/**
	 * Clears the dmx components, destroys the Geometry Tree and deletes all of the construction scripts related to the Actor
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 22 May 2023
	 *
	 * @param Actor Actor to purge
	 */
	static void PurgeAllComponents(ACPGDTFFixtureActor* Actor);

private:

	/**
	 * Setup the actor's DMXComponents using one Attribute.
	 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
	 * @date 30 june 2022
	 *
	 * @param Actor Actor to attach the components
	 * @param DMXChannels DMXChannels to instanciate
	 */
	static void LoadSimpleAttributeComponents_INTERNAL(ACPGDTFFixtureActor* Actor, TArray<FDMXImportGDTFDMXChannel>* DMXChannels);

	/**
	 * Setup the actor's DMXComponents using multiple Attributes.
	 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
	 * @date 30 june 2022
	 *
	 * @param Actor Actor to attach the components
	 * @param DMXChannels DMXChannels to instanciate
	 * @return True if no problem occured
	 */
	static bool LoadMultipleAttributesComponents_INTERNAL(ACPGDTFFixtureActor* Actor, TArray<FDMXImportGDTFDMXChannel>* DMXChannels);

	/**
	 * Find all DMXChannels referencing a given Attribute name pattern. Also remove the found channels from the given array.
	 * In the case where we find multiple channel for a unique Attribute Type (multiple ColorAdd_R for LED Beam moving head for example)
	 * we ensure that all the returned Chanels are linked to the same geometry
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 30 june 2022
	 * 
	 * @param AttributeNamePattern Regex pattern to look for specific Attributes.
	 * @param DMXChannels Array where to look for.
	 * @return Channels found
	 */
	static TSet<int> FindDMXChannelsIdxByAttributeTypePattern(FString AttributeNamePattern, TArray<FDMXImportGDTFDMXChannel> &DMXChannels);
};