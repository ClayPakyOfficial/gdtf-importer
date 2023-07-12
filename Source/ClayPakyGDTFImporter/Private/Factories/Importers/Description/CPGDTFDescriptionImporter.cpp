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


#include "CPGDTFDescriptionImporter.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Utils/CPGDTFImporterUtils.h"
#include "Factories/CPGDTFUnzip.h"
#include "Factories/TextureFactory.h"
#include "CPGDTFDescription.h"
#include "Library/DMXImportGDTF.h"
#include "PackageTools.h"

#include "XmlNode.h"
#include "XmlFile.h"
#include "CPGDTFDescriptionImporter.h"

#define PHYSICAL_CHANNEL_SET_PLACEHOLDER -694316420.0f

/**
 * Creates the Importer of GDTF Descriptions
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 04 may 2022
 */
FCPGDTFDescriptionImporter::FCPGDTFDescriptionImporter(UPackage* Package, FName AssetName, EObjectFlags Flags, FString GDTFPath) {
	this->AssetName = AssetName;
	this->Flags = Flags;
	this->GDTFPath = GDTFPath;
	this->XMLFile = nullptr;

	FString PackageName = UPackageTools::SanitizePackageName(Package->GetName() + "/" + AssetName.ToString());
	this->Package = CreatePackage(*PackageName);
}

/**
 * Import the GDTF Description from a GDTF file
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 04 may 2022
 */
UCPGDTFDescription* FCPGDTFDescriptionImporter::Import() {

	FXmlFile* XML = ExtractXML();
	if (XML == nullptr) {
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Error extracting description.xml from GDTF archive"));
		return nullptr;
	}
	this->XMLFile = XML;

	UCPGDTFDescription* Asset = ParseXML();
	return Asset;
}

/**
 * Import the GDTF Thumbnail from a GDTF file
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 05 may 2022
 * 
 * @param XMLDescription XMLFile to find the name of the png file on the GDTF archive
 */
UTexture2D* FCPGDTFDescriptionImporter::ImportThumbnail(UCPGDTFDescription* XMLDescription) {

	// Check if the GDTF Description is already imported
	if (this->XMLFile == nullptr) {
		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Import GDTF before the thumbnail"));
		return nullptr;
	}

	// Get Filename in XML
	FString ThumbnailFileName;
	const FXmlNode* FixtureTypeNode = this->XMLFile->GetRootNode()->GetFirstChildNode(); // Get the FixtureTypeNode
	if (FixtureTypeNode) {
		ThumbnailFileName = FixtureTypeNode->GetAttribute("Thumbnail").TrimStartAndEnd();
	}

	if (ThumbnailFileName.Equals("")) {
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Thumbnail filename not found in GDTF description"));
		return nullptr;
	}

	FString AssetPath = this->Package->GetName();
	int32 pos;
	AssetPath.FindLastChar('/', pos);
	AssetPath.RemoveAt(pos, AssetPath.Len() - pos, true);
	AssetPath += "/textures";

	return FCPGDTFImporterUtils::ImportPNG(this->GDTFPath, "Thumbnail", ThumbnailFileName, AssetPath);
}

/**
 * Get the XML Description representing the Fixture
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 06 may 2022
 */
const FXmlFile* FCPGDTFDescriptionImporter::GetXML() {
	
	return this->XMLFile;
}

/**
 * Parse the XML file and create the Asset
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 04 may 2022
 */
UCPGDTFDescription* FCPGDTFDescriptionImporter::ParseXML() {

	// Create new GDTF asset
	UCPGDTFDescription* ImportObject = NewObject<UCPGDTFDescription>(this->Package, this->AssetName, this->Flags | RF_Public);

	UDMXImportGDTFFixtureType* GDTFFixtureType = ImportObject->CreateNewObject<UDMXImportGDTFFixtureType>();
	ImportObject->FixtureType = GDTFFixtureType;

	UDMXImportGDTFAttributeDefinitions* GDTFAttributeDefinitions = ImportObject->CreateNewObject<UDMXImportGDTFAttributeDefinitions>();
	ImportObject->AttributeDefinitions = GDTFAttributeDefinitions;

	UDMXImportGDTFWheels* GDTFWheels = ImportObject->CreateNewObject<UDMXImportGDTFWheels>();
	ImportObject->Wheels = GDTFWheels;

	UDMXImportGDTFPhysicalDescriptions* GDTFPhysicalDescriptions = ImportObject->CreateNewObject<UDMXImportGDTFPhysicalDescriptions>();
	ImportObject->PhysicalDescriptions = GDTFPhysicalDescriptions;

	UCPGDTFDescriptionModels* GDTFModels = ImportObject->CreateNewObject<UCPGDTFDescriptionModels>();
	ImportObject->Models = GDTFModels;

	UCPGDTFDescriptionGeometries* GDTFGeometries = ImportObject->CreateNewObject<UCPGDTFDescriptionGeometries>();
	ImportObject->Geometries = GDTFGeometries;

	UDMXImportGDTFDMXModes* GDTFDMXModes = ImportObject->CreateNewObject<UDMXImportGDTFDMXModes>();
	ImportObject->DMXModes = GDTFDMXModes;

	UDMXImportGDTFProtocols* GDTFProtocols = ImportObject->CreateNewObject<UDMXImportGDTFProtocols>();
	ImportObject->Protocols = GDTFProtocols;

	if (const FXmlNode* XmlNode = this->XMLFile->GetRootNode())
	{
		if (const FXmlNode* FixtureTypeNode = XmlNode->FindChildNode("FixtureType"))
		{
			GDTFFixtureType->Description = FixtureTypeNode->GetAttribute("Description").TrimStartAndEnd();
			GDTFFixtureType->FixtureTypeID = FixtureTypeNode->GetAttribute("FixtureTypeID").TrimStartAndEnd();
			GDTFFixtureType->LongName = FixtureTypeNode->GetAttribute("LongName").TrimStartAndEnd();
			GDTFFixtureType->Manufacturer = FixtureTypeNode->GetAttribute("Manufacturer").TrimStartAndEnd();
			GDTFFixtureType->Name = FName(*FixtureTypeNode->GetAttribute("Name").TrimStartAndEnd());
			GDTFFixtureType->ShortName = FixtureTypeNode->GetAttribute("ShortName").TrimStartAndEnd();
			GDTFFixtureType->RefFT = FixtureTypeNode->GetAttribute("RefFT").TrimStartAndEnd();

			if (const FXmlNode* AttributeDefinitionsNode = FixtureTypeNode->FindChildNode("AttributeDefinitions"))
			{
				if (const FXmlNode* ActivationGroupsNode = AttributeDefinitionsNode->FindChildNode("ActivationGroups"))
				{
					for (const FXmlNode* ActivationGroupNode : ActivationGroupsNode->GetChildrenNodes())
					{
						FDMXImportGDTFActivationGroup ActivationGroup;
						ActivationGroup.Name = FName(*ActivationGroupNode->GetAttribute("Name").TrimStartAndEnd());
						GDTFAttributeDefinitions->ActivationGroups.Add(ActivationGroup);
					}
				}

				if (const FXmlNode* FeatureGroupsNode = AttributeDefinitionsNode->FindChildNode("FeatureGroups"))
				{
					for (const FXmlNode* FeatureGroupNode : FeatureGroupsNode->GetChildrenNodes())
					{
						if (FeatureGroupNode != nullptr)
						{
							FDMXImportGDTFFeatureGroup FeatureGroup;
							FeatureGroup.Name = FName(*FeatureGroupNode->GetAttribute("Name").TrimStartAndEnd());
							FeatureGroup.Pretty = FeatureGroupNode->GetAttribute("Pretty").TrimStartAndEnd();

							for (const FXmlNode* FeatureNode : FeatureGroupNode->GetChildrenNodes())
							{
								FDMXImportGDTFFeature Feature;
								Feature.Name = FName(*FeatureNode->GetAttribute("Name").TrimStartAndEnd());
								FeatureGroup.Features.Add(Feature);
							}

							GDTFAttributeDefinitions->FeatureGroups.Add(FeatureGroup);
						}
					}
				}

				if (const FXmlNode* AttributesNode = AttributeDefinitionsNode->FindChildNode("Attributes"))
				{
					for (const FXmlNode* AttributeNode : AttributesNode->GetChildrenNodes())
					{
						if (AttributeNode != nullptr)
						{
							FCPGDTFDescriptionAttribute Attribute;
							Attribute.Name = FName(*AttributeNode->GetAttribute("Name").TrimStartAndEnd());
							Attribute.Type = CPGDTFDescription::GetGDTFAttributeTypeValueFromString(Attribute.Name.ToString());
							Attribute.Pretty = AttributeNode->GetAttribute("Pretty").TrimStartAndEnd();
							Attribute.ActivationGroup.Name = FName(*AttributeNode->GetAttribute("ActivationGroup").TrimStartAndEnd());
							GDTFAttributeDefinitions->FindFeature(AttributeNode->GetAttribute("Feature").TrimStartAndEnd(), Attribute.Feature);
							Attribute.MainAttribute = AttributeNode->GetAttribute ("MainAttribute").TrimStartAndEnd ();
							Attribute.PhysicalUnit = DMXImport::GetEnumValueFromString<EDMXImportGDTFPhysicalUnit>(AttributeNode->GetAttribute("PhysicalUnit").TrimStartAndEnd());
							Attribute.Color = DMXImport::ParseColorCIE(AttributeNode->GetAttribute("Color").TrimStartAndEnd());
							GDTFAttributeDefinitions->Attributes.Add(Attribute);
						}
					}
				}
			}

			if (const FXmlNode* WheelsNode = FixtureTypeNode->FindChildNode("Wheels"))
			{
				for (const FXmlNode* WheelNode : WheelsNode->GetChildrenNodes())
				{
					FDMXImportGDTFWheel ImportWheel;
					ImportWheel.Name = FName(*WheelNode->GetAttribute("Name").TrimStartAndEnd());

					for (const FXmlNode* SlotNode : WheelNode->GetChildrenNodes())
					{
						FDMXImportGDTFWheelSlot ImportWheelSlot;
						ImportWheelSlot.Name = FName(*SlotNode->GetAttribute("Name").TrimStartAndEnd());
						ImportWheelSlot.Color = DMXImport::ParseColorCIE(SlotNode->GetAttribute("Color").TrimStartAndEnd());
						ImportWheel.Slots.Add(ImportWheelSlot);
					}

					GDTFWheels->Wheels.Add(ImportWheel);
				}
			}

			if (const FXmlNode* PhysicalDescriptionsNode = FixtureTypeNode->FindChildNode("PhysicalDescriptions"))
			{
				if (const FXmlNode* EmittersNode = PhysicalDescriptionsNode->FindChildNode("Emitters"))
				{
					for (const FXmlNode* EmitterNode : EmittersNode->GetChildrenNodes())
					{
						FDMXImportGDTFEmitter ImportEmitter;

						ImportEmitter.Name = FName(*EmitterNode->GetAttribute("Name").TrimStartAndEnd());
						ImportEmitter.Color = DMXImport::ParseColorCIE(EmitterNode->GetAttribute("Color").TrimStartAndEnd());
						ImportEmitter.DiodePart = EmitterNode->GetAttribute("DiodePart").TrimStartAndEnd();
						LexTryParseString(ImportEmitter.DominantWaveLength, *EmitterNode->GetAttribute("DominantWaveLength").TrimStartAndEnd());
						ImportEmitter.DiodePart = EmitterNode->GetAttribute("DiodePart").TrimStartAndEnd();

						/* Disabled for now. Unused by Unreal and slow down the GDTF Description editor because represent a lot of values
						if (const FXmlNode* MeasurementNode = EmitterNode->FindChildNode("Measurement"))
						{
							FDMXImportGDTFMeasurement ImportMeasurement;

							LexTryParseString(ImportMeasurement.Physical, *MeasurementNode->GetAttribute("DominantWaveLength").TrimStartAndEnd());
							LexTryParseString(ImportMeasurement.LuminousIntensity, *MeasurementNode->GetAttribute("LuminousIntensity").TrimStartAndEnd());
							LexTryParseString(ImportMeasurement.Transmission, *MeasurementNode->GetAttribute("Transmission").TrimStartAndEnd());
							ImportMeasurement.InterpolationTo = CPGDTFDescription::GetEnumValueFromString<EDMXImportGDTFInterpolationTo>(MeasurementNode->GetAttribute("InterpolationTo").TrimStartAndEnd());

							for (const FXmlNode* MeasurementPointNode : MeasurementNode->GetChildrenNodes())
							{
								FDMXImportGDTFMeasurementPoint ImportMeasurementPoint;

								LexTryParseString(ImportMeasurementPoint.Energy, *MeasurementPointNode->GetAttribute("Energy").TrimStartAndEnd());
								LexTryParseString(ImportMeasurementPoint.WaveLength, *MeasurementPointNode->GetAttribute("WaveLength").TrimStartAndEnd());

								ImportMeasurement.MeasurementPoints.Add(ImportMeasurementPoint);
							}

							ImportEmitter.Measurement = ImportMeasurement;
						}*/

						GDTFPhysicalDescriptions->Emitters.Add(ImportEmitter);
					}
				}

				if (const FXmlNode* ColorSpaceNode = PhysicalDescriptionsNode->FindChildNode("ColorSpace"))
				{
					FDMXImportGDTFColorSpace ImportColorSpace;
					ImportColorSpace.Mode = CPGDTFDescription::GetEnumValueFromString<EDMXImportGDTFMode>(ColorSpaceNode->GetAttribute("Mode").TrimStartAndEnd());
					ImportColorSpace.Description = ColorSpaceNode->GetAttribute("Description").TrimStartAndEnd();
					ImportColorSpace.Red = DMXImport::ParseColorCIE(ColorSpaceNode->GetAttribute("Red").TrimStartAndEnd());
					ImportColorSpace.Blue = DMXImport::ParseColorCIE(ColorSpaceNode->GetAttribute("Blue").TrimStartAndEnd());
					ImportColorSpace.Green = DMXImport::ParseColorCIE(ColorSpaceNode->GetAttribute("Green").TrimStartAndEnd());
					ImportColorSpace.WhitePoint = DMXImport::ParseColorCIE(ColorSpaceNode->GetAttribute("WhitePoint").TrimStartAndEnd());

					GDTFPhysicalDescriptions->ColorSpace = ImportColorSpace;
				}
			}

			if (const FXmlNode* ModelsNode = FixtureTypeNode->FindChildNode("Models"))
			{
				for (const FXmlNode* ModelNode : ModelsNode->GetChildrenNodes())
				{
					FCPGDTFDescriptionModel ImportModel;

					ImportModel.Name = FName(*ModelNode->GetAttribute("Name").TrimStartAndEnd());
					LexTryParseString(ImportModel.Length, *ModelNode->GetAttribute("Length").TrimStartAndEnd());
					ImportModel.Length *= 100.0f; // Conversion from GDTF meters to Unreal centimeters default unit
					LexTryParseString(ImportModel.Width, *ModelNode->GetAttribute("Width").TrimStartAndEnd());
					ImportModel.Width *= 100.0f; // Conversion from GDTF meters to Unreal centimeters default unit
					LexTryParseString(ImportModel.Height, *ModelNode->GetAttribute("Height").TrimStartAndEnd());
					ImportModel.Height *= 100.0f; // Conversion from GDTF meters to Unreal centimeters default unit
					ImportModel.PrimitiveType = CPGDTFDescription::GetEnumValueFromString<ECPGDTFDescriptionModelsPrimitiveType>(ModelNode->GetAttribute("PrimitiveType").TrimStartAndEnd());
					ImportModel.File = FName(*ModelNode->GetAttribute("File").TrimStartAndEnd());

					GDTFModels->Models.Add(ImportModel);
				}
			}

			if (const FXmlNode* GeometriesNode = FixtureTypeNode->FindChildNode("Geometries"))
			{
				for (const FXmlNode* GeometryNode : GeometriesNode->GetChildrenNodes())
				{
					GDTFGeometries->Geometries.Add(this->ParseGeometriesNode(ImportObject, GeometryNode));
				}
			}

			// Depending on the GDTF spec the Break node may be called either 'DMXModes' or 'Modes'.
			const FXmlNode* const ModesNode = FCPGDTFDescriptionImporter::FindChildNodeEvenIfDMXSubstringIsMissing(*FixtureTypeNode, TEXT("DMXModes"));
			if (ModesNode)
			{
				for (const FXmlNode* DMXModeNode : ModesNode->GetChildrenNodes())
				{
					FDMXImportGDTFDMXMode DMXGDTFDescriptionDMXMode;
					DMXGDTFDescriptionDMXMode.Name = FName(*DMXModeNode->GetAttribute("Name").TrimStartAndEnd());
					DMXGDTFDescriptionDMXMode.Geometry = FName(*DMXModeNode->GetAttribute("Geometry").TrimStartAndEnd());

					// Depending on the GDTF spec in use modes may be stored in the "Modes" or "DMXModes" node.
					const FXmlNode* const ChannelsNode = FCPGDTFDescriptionImporter::FindChildNodeEvenIfDMXSubstringIsMissing(*DMXModeNode, TEXT("DMXChannels"));
					if (ChannelsNode)
					{
						for (const FXmlNode* DMXChannelNode : ChannelsNode->GetChildrenNodes())
						{
							FCPDMXImportGDTFDMXChannel ImportDMXChannel;

							// Ignore Channels that do not specify a valid offset.
							// E.g. 'Robe Lighting s.r.o.@Robin 800X LEDWash.gdtf' specifies virtual Dimmer channels that have no offset and cannot be accessed.
							if (!ImportDMXChannel.ParseOffset(DMXChannelNode->GetAttribute("Offset").TrimStartAndEnd()))
							{
								continue;
							}

							ImportDMXChannel.Highlight = FDMXImportGDTFDMXValue (DMXChannelNode->GetAttribute("Highlight").TrimStartAndEnd());
							ImportDMXChannel.Geometry = FName(*DMXChannelNode->GetAttribute("Geometry").TrimStartAndEnd());
							LexTryParseString(ImportDMXChannel.DMXBreak, *DMXChannelNode->GetAttribute("DMXBreak").TrimStartAndEnd());

							for (const FXmlNode* LogicalChannelNode : DMXChannelNode->GetChildrenNodes()) {
								FCPDMXImportGDTFLogicalChannel ImportLogicalChannel;

								GDTFAttributeDefinitions->FindAtributeByName(*LogicalChannelNode->GetAttribute("Attribute").TrimStartAndEnd(), ImportLogicalChannel.Attribute);
								ImportLogicalChannel.Snap = CPGDTFDescription::GetEnumValueFromString<EDMXImportGDTFSnap>(LogicalChannelNode->GetAttribute("Snap").TrimStartAndEnd());
								ImportLogicalChannel.Master = CPGDTFDescription::GetEnumValueFromString<EDMXImportGDTFMaster>(LogicalChannelNode->GetAttribute("Master").TrimStartAndEnd());

								LexTryParseString(ImportLogicalChannel.MibFade, *LogicalChannelNode->GetAttribute("MibFade").TrimStartAndEnd());
								LexTryParseString(ImportLogicalChannel.DMXChangeTimeLimit, *LogicalChannelNode->GetAttribute("DMXChangeTimeLimit").TrimStartAndEnd());

								for (const FXmlNode* ChannelFunctionNode : LogicalChannelNode->GetChildrenNodes()) {

									FCPDMXImportGDTFChannelFunction ImportChannelFunction;
									ImportChannelFunction.Name = FName(*ChannelFunctionNode->GetAttribute("Name").TrimStartAndEnd());
									GDTFAttributeDefinitions->FindAtributeByName(*ChannelFunctionNode->GetAttribute("Attribute").TrimStartAndEnd(), ImportChannelFunction.Attribute);
									ImportChannelFunction.OriginalAttribute = *ChannelFunctionNode->GetAttribute("OriginalAttribute").TrimStartAndEnd();
									ImportChannelFunction.DMXFrom = FDMXImportGDTFDMXValue(ChannelFunctionNode->GetAttribute("DMXFrom").TrimStartAndEnd());
									ImportChannelFunction.DMXValue = FDMXImportGDTFDMXValue(ChannelFunctionNode->GetAttribute("DMXValue").TrimStartAndEnd());
									ImportChannelFunction.Default = FDMXImportGDTFDMXValue(ChannelFunctionNode->GetAttribute("Default").TrimStartAndEnd());

									LexTryParseString(ImportChannelFunction.PhysicalFrom, *ChannelFunctionNode->GetAttribute("PhysicalFrom").TrimStartAndEnd());
									if (ChannelFunctionNode->GetAttribute("PhysicalTo").TrimStartAndEnd().IsEmpty()) ImportChannelFunction.PhysicalTo = 1.0f;
									else LexTryParseString(ImportChannelFunction.PhysicalTo, *ChannelFunctionNode->GetAttribute("PhysicalTo").TrimStartAndEnd());
									LexTryParseString(ImportChannelFunction.RealFade, *ChannelFunctionNode->GetAttribute("RealFade").TrimStartAndEnd());
									LexTryParseString(ImportChannelFunction.RealAcceleration, *ChannelFunctionNode->GetAttribute("RealAcceleration").TrimStartAndEnd());

									GDTFWheels->FindWeelByName(*ChannelFunctionNode->GetAttribute("Wheel").TrimStartAndEnd(), ImportChannelFunction.Wheel);
									GDTFPhysicalDescriptions->FindEmitterByName(*ChannelFunctionNode->GetAttribute("Emitter").TrimStartAndEnd(), ImportChannelFunction.Emitter);

									ImportChannelFunction.DMXInvert = CPGDTFDescription::GetEnumValueFromString<EDMXImportGDTFDMXInvert>(ChannelFunctionNode->GetAttribute("DMXInvert").TrimStartAndEnd());
									ImportChannelFunction.ModeMaster = *ChannelFunctionNode->GetAttribute("ModeMaster").TrimStartAndEnd();
									ImportChannelFunction.ModeFrom = FDMXImportGDTFDMXValue(ChannelFunctionNode->GetAttribute("ModeFrom").TrimStartAndEnd());
									ImportChannelFunction.ModeTo = FDMXImportGDTFDMXValue(ChannelFunctionNode->GetAttribute("ModeTo").TrimStartAndEnd());

									for (const FXmlNode* ChannelSetNode : ChannelFunctionNode->GetChildrenNodes()) {
										FDMXImportGDTFChannelSet ImportChannelSet;

										ImportChannelSet.Name = *ChannelSetNode->GetAttribute("Name").TrimStartAndEnd();
										ImportChannelSet.DMXFrom = FDMXImportGDTFDMXValue(ChannelSetNode->GetAttribute("DMXFrom").TrimStartAndEnd());
										if (LexTryParseString(ImportChannelSet.WheelSlotIndex, *ChannelSetNode->GetAttribute("WheelSlotIndex").TrimStartAndEnd()))
											ImportChannelSet.WheelSlotIndex--; // The starting index is 1 in GDTF Spec

										if (ChannelSetNode->GetAttribute("PhysicalFrom").TrimStartAndEnd().IsEmpty()) {
											ImportChannelSet.PhysicalFrom = PHYSICAL_CHANNEL_SET_PLACEHOLDER;
										} else LexTryParseString(ImportChannelSet.PhysicalFrom, *ChannelSetNode->GetAttribute("PhysicalFrom").TrimStartAndEnd());

 										if (ChannelSetNode->GetAttribute("PhysicalTo").TrimStartAndEnd().IsEmpty()) {
											ImportChannelSet.PhysicalTo = PHYSICAL_CHANNEL_SET_PLACEHOLDER;
										} else LexTryParseString(ImportChannelSet.PhysicalTo, *ChannelSetNode->GetAttribute("PhysicalTo").TrimStartAndEnd());

										ImportChannelFunction.ChannelSets.Add(ImportChannelSet);
									}
									ImportLogicalChannel.ChannelFunctions.Add(ImportChannelFunction);
								}

								//Fix the channelset physical from/to
								int lastDmx = (1 << 8 * ImportDMXChannel.Offset.Num()) - 1;
								for (int cfIdx = ImportLogicalChannel.ChannelFunctions.Num() - 1; cfIdx >= 0; cfIdx--) {
									FDMXImportGDTFChannelFunction* cf = &ImportLogicalChannel.ChannelFunctions[cfIdx];
									float physicalDiff = cf->PhysicalTo - cf->PhysicalFrom;
									int endAddress = lastDmx, startAddress = cf->DMXFrom.Value;
									int addrDiff = endAddress - startAddress;

									for (int csIdx = cf->ChannelSets.Num() - 1; csIdx >= 0; csIdx--) {
										FDMXImportGDTFChannelSet* cs = &cf->ChannelSets[csIdx];
										FDMXImportGDTFChannelSet* csPrev = csIdx > 0 ? &cf->ChannelSets[csIdx - 1] : nullptr;;
										int currentAddress = cs->DMXFrom.Value;
										lastDmx = currentAddress - 1;

										//actual fix
										//current->from
										if (cs->PhysicalFrom == PHYSICAL_CHANNEL_SET_PLACEHOLDER)
											cs->PhysicalFrom = (physicalDiff * ((float)(currentAddress - startAddress)) / addrDiff) + cf->PhysicalFrom; //phys : diffPhys = dmx : diffDmx
										//prev->to
										if (csPrev != nullptr && csPrev->PhysicalTo == PHYSICAL_CHANNEL_SET_PLACEHOLDER)
											csPrev->PhysicalTo = cs->PhysicalFrom;
										//current->to. If it hasn't already fixed, it means we're the first channelset, so our to-value should be obtained from the channel function
										if (cs->PhysicalTo == PHYSICAL_CHANNEL_SET_PLACEHOLDER)
											cs->PhysicalTo = cf->PhysicalTo;
									}
								}

								ImportDMXChannel.LogicalChannels.Add(ImportLogicalChannel);
							}
							
							//Set the default dmxchannel value
							FString initialFunction = DMXChannelNode->GetAttribute("InitialFunction").TrimStartAndEnd();
							if (!initialFunction.IsEmpty())
								ImportDMXChannel.Default = ImportDMXChannel.getChannelFunctionByName(initialFunction)->Default;

							DMXGDTFDescriptionDMXMode.DMXChannels.Add(ImportDMXChannel);
						}
					}

					if (const FXmlNode* RelationsNode = DMXModeNode->FindChildNode("Relations"))
					{
						for (const FXmlNode* RelationNode : RelationsNode->GetChildrenNodes())
						{
							FDMXImportGDTFRelation ImportRelation;
							ImportRelation.Name = RelationNode->GetAttribute("Name").TrimStartAndEnd();
							ImportRelation.Master = RelationNode->GetAttribute("Master").TrimStartAndEnd();
							ImportRelation.Follower = RelationNode->GetAttribute("Follower").TrimStartAndEnd();
							ImportRelation.Type = CPGDTFDescription::GetEnumValueFromString<EDMXImportGDTFType>(RelationNode->GetAttribute("Type").TrimStartAndEnd());

							DMXGDTFDescriptionDMXMode.Relations.Add(ImportRelation);
						}
					}

					if (const FXmlNode* FTMacrosNode = DMXModeNode->FindChildNode("FTMacros"))
					{
						for (const FXmlNode* FTMacroNode : FTMacrosNode->GetChildrenNodes())
						{
							FDMXImportGDTFFTMacro ImportFTMacro;
							ImportFTMacro.Name = FName(*FTMacroNode->GetAttribute("Name").TrimStartAndEnd());
							DMXGDTFDescriptionDMXMode.FTMacros.Add(ImportFTMacro);
						}
					}

					GDTFDMXModes->DMXModes.Add(DMXGDTFDescriptionDMXMode);
				}
			}

			if (const FXmlNode* ProtocolsNode = FixtureTypeNode->FindChildNode("Protocols"))
			{
				for (const FXmlNode* ProtocolNode : ProtocolsNode->GetChildrenNodes())
				{
					GDTFProtocols->Protocols.Add(FName(*ProtocolNode->GetTag()));
				}
			}
		}
	}

	return ImportObject;
}

/**
 * Extract and read description.xml inside GDTF archive.
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 04 may 2022
 *
 * @return May be null if it was unable to read the XML file
 */
FXmlFile* FCPGDTFDescriptionImporter::ExtractXML() {

	const FString XMLString = UCPGDTFUnzip::ExtractTextFileFromGDTFArchive(this->GDTFPath, FString("description.xml"));

	if (XMLString.Equals("")) return nullptr;
	else {

		FXmlFile* XmlFileObj = new FXmlFile();
		XmlFileObj->LoadFile(XMLString, EConstructMethod::ConstructFromBuffer);
		if (XmlFileObj->IsValid()) return XmlFileObj;
		else return nullptr;
	}
}

/**
* Depending on the GDTF Spec, some XML Attribute Names might start with or without DMX.
*
* @param ParentNode		The Parent Node to search in
* @param AttributeNameWithDMXTag The Attribute Name with DMX tag. This is the correct form, e.g. 'DMXMode', not 'Mode', 'DMXChannels', not 'Channels'.
* @return					Returns a pointer to the child node, or nullptr if the child cannot be found
*/
const FXmlNode* FCPGDTFDescriptionImporter::FindChildNodeEvenIfDMXSubstringIsMissing(const FXmlNode& ParentNode, const FString& AttributeNameWithDMXTag)
{
	if (const FXmlNode* ChildNodePtrWithSubstring = ParentNode.FindChildNode(AttributeNameWithDMXTag))
	{
		return ChildNodePtrWithSubstring;
	}
	else if (const FXmlNode* ChildNodePtrWithoutSubstring = ParentNode.FindChildNode(AttributeNameWithDMXTag.RightChop(3)))
	{
		return ChildNodePtrWithoutSubstring;
	}

	return nullptr;
}

/**
* Depending on the GDTF Spec, some XML Attribute Names might start with or without DMX.
*
* @param ParentNode		The Parent Node to search in
* @param AttributeNameWithDMXTag The Attribute Name with DMX tag. This is the correct form, e.g. 'DMXMode', not 'Mode', 'DMXChannels', not 'Channels'.
* @return					Returns the Attribute as String
*/
FString FCPGDTFDescriptionImporter::FindAttributeEvenIfDMXSubstringIsMissing(const FXmlNode& ParentNode, const FString& AttributeNameWithDMXTag)
{
	FString Attribute = ParentNode.GetAttribute(AttributeNameWithDMXTag).TrimStartAndEnd();
	if (Attribute.IsEmpty())
	{
		Attribute = ParentNode.GetAttribute(AttributeNameWithDMXTag.RightChop(3)).TrimStartAndEnd();
	}

	return Attribute;
}

/**
 * Parse an XML geometry node and all his childrens recursively
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 14 june 2022
*/
UCPGDTFDescriptionGeometryBase* FCPGDTFDescriptionImporter::ParseGeometriesNode(UCPGDTFDescription* DescriptionRoot, const FXmlNode* GeometryNode) {

	UCPGDTFDescriptionGeometryBase* Geometry;
	UCPGDTFDescriptionGeometryBeam* GeometryBeam;
	UCPGDTFDescriptionGeometryReference* GeometryReference;

	// Instantiation of the right object
	switch (CPGDTFDescription::GetEnumValueFromString<ECPGDTFDescriptionGeometryType>(GeometryNode->GetTag())) {

	case ECPGDTFDescriptionGeometryType::Geometry:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometry>();
		break;
	case ECPGDTFDescriptionGeometryType::Axis:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometryAxis>();
		break;
	case ECPGDTFDescriptionGeometryType::FilterBeam:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometryFilterBeam>();
		break;
	case ECPGDTFDescriptionGeometryType::FilterColor:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometryFilterColor>();
		break;
	case ECPGDTFDescriptionGeometryType::FilterGobo:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometryFilterGobo>();
		break;
	case ECPGDTFDescriptionGeometryType::FilterShaper:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometryFilterShaper>();
		break;
	case ECPGDTFDescriptionGeometryType::Beam:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometryBeam>();
		break;
	case ECPGDTFDescriptionGeometryType::GeometryReference:
		Geometry = DescriptionRoot->CreateNewObject<UCPGDTFDescriptionGeometryReference>();
		break;
	default:
		return nullptr;
	}

	// Base
	Geometry->Name = FName(*GeometryNode->GetAttribute("Name").TrimStartAndEnd());
	Geometry->Position = CPGDTFDescription::ParseMatrix(GeometryNode->GetAttribute("Position").TrimStartAndEnd());
	Geometry->Model = FName(*GeometryNode->GetAttribute("Model").TrimStartAndEnd());

	// Fill the specific object properties
	switch (Geometry->Type) {

	case ECPGDTFDescriptionGeometryType::Beam: // Beam specific
		GeometryBeam = Cast<UCPGDTFDescriptionGeometryBeam>(Geometry);
		GeometryBeam->LampType = DMXImport::GetEnumValueFromString<EDMXImportGDTFLampType>(GeometryNode->GetAttribute("LampType").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->PowerConsumption, *GeometryNode->GetAttribute("PowerConsumption").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->LuminousFlux, *GeometryNode->GetAttribute("LuminousFlux").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->ColorTemperature, *GeometryNode->GetAttribute("ColorTemperature").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->BeamAngle, *GeometryNode->GetAttribute("BeamAngle").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->FieldAngle, *GeometryNode->GetAttribute("FieldAngle").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->BeamRadius, *GeometryNode->GetAttribute("BeamRadius").TrimStartAndEnd());
		GeometryBeam->BeamType = CPGDTFDescription::GetEnumValueFromString<ECPGDTFDescriptionGeometryBeamType>(GeometryNode->GetAttribute("BeamType").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->ColorRenderingIndex, *GeometryNode->GetAttribute("ColorRenderingIndex").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->RectangleRatio, *GeometryNode->GetAttribute("RectangleRatio").TrimStartAndEnd());
		LexTryParseString(GeometryBeam->ThrowRatio, *GeometryNode->GetAttribute("ThrowRatio").TrimStartAndEnd());
		break;

	case ECPGDTFDescriptionGeometryType::GeometryReference: // GeometryReference specific
		GeometryReference = Cast<UCPGDTFDescriptionGeometryReference>(Geometry);
		GeometryReference->Geometry = FName(*GeometryNode->GetAttribute("Geometry").TrimStartAndEnd());
		for (const FXmlNode* BreakNode : GeometryNode->GetChildrenNodes()) {
			FDMXImportGDTFBreak ImportBreak;

			// Depending on the GDTF spec the Offset node may be called either 'DMXOffset' or 'Offset'
			const FString DMXOffsetAttribute = FCPGDTFDescriptionImporter::FindAttributeEvenIfDMXSubstringIsMissing(*BreakNode, TEXT("DMXOffset"));
			LexTryParseString(ImportBreak.DMXOffset, *DMXOffsetAttribute);

			// Depending on the GDTF spec the Break node may be called either 'DMXBreak' or 'Break'
			const FString DMXBreakAttribute = FCPGDTFDescriptionImporter::FindAttributeEvenIfDMXSubstringIsMissing(*BreakNode, TEXT("DMXBreak"));
			LexTryParseString(ImportBreak.DMXBreak, *DMXBreakAttribute);

			GeometryReference->Breaks.Add(ImportBreak);
		}
		return Geometry; // A geometry reference can't have any child

	default:
		break; // For other object types every field was specified before switch
	}

	// Import childrens recursively
	for (const FXmlNode* GeometryChild : GeometryNode->GetChildrenNodes()) {
		Geometry->Childrens.Add(this->ParseGeometriesNode(DescriptionRoot, GeometryChild));
	}

	// Return to parent
	return Geometry;
}

#undef PHYSICAL_CHANNEL_SET_PLACEHOLDER