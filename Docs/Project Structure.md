# Project Structure

# Code Part

#### FClayPakyGDTFImporterModule
Unreal entry point. Used to load and unload the plugin 

#### FixtureActor
Ready to use [Actor](@ref ACPGDTFFixtureActor) representing the fixtures. Parent Class of all the BluePrint generated from GDTF. 

#### GDTFDescription
[Set of objects](@ref GDTFDesc) containing GDTF XML description. Inherited from UDMXImportGDTF to remain compatible with Unreal DMX Engine.

#### FChannelInterpolation
[Class](@ref FChannelInterpolation) used to smooth the values changes. Mainly used to simulate moving parts like Pan & Tilt for example.

\warning
**HELL**<br>
If you have to edit this class run for your life.

#### UE_LOG_CPGDTFIMPORTER
C++ Macro used to print on Unreal logs

## Components
The components are classes who can be added to an actor to add some functionalities to it. (More on [Unreal Documentation](https://docs.unrealengine.com/5.0/en-US/components-in-unreal-engine/))

#### Beam Scene Component
An all in one [component](@ref UCPGDTFBeamSceneComponent) to manage all the elements needed to create a light output.
<br>Composed of :
- A ``SpotLight`` to draw the gobo on floor, walls, ceilling, etc. 
- A ``PointLight`` to create a ambient lighting effect.
- A ``Beam Static Mesh`` to attach the beginning of the beam shader.
- A ``Lens Static Mesh`` to draw the dynamic lens texture.
- A Occlusion direction ``Arrow Component`` to have a vector representing the light direction

#### DMX Components
DMX Components are a set of actor components inherited from ``UCPGDTFFixtureComponentBase`` who implement one or more GDTF DMX attribute.
See [DMX Component section](@ref DMXComp) for more details.

## Factory and Importers

### GDTFFactory
Class in charge of the creation of all the Unreal objects based on a given GDTF file. All the methods are called automaticaly by Unreal during a GDTF import.

### Importers
Classes in charge of the reading of a given GDTF file and the creation of the different Unreal objects based on it.

## Libs

### FastGaussianBlur
Lib used during gobo wheels import to simulate a frost effect.

### MiniZ
Compression lib used to extract the files from a given GDTF archive.

## Utils
Set of classes used to simplify the project with very used methods:
- ``FCPFActorComponentsLoader`` Used to automate the creation/destruction of an ACPGDTFFixtureActor [DMX Components](@ref DMXComp).
- ``FActorGeometryTree`` Used to automate the creation/destruction of the ACPGDTFFixtureActor Geometry tree.
- ``FCPColorWizard`` Class blending colors together. Used to simulate LED engines. 
- ``FDMXChannelTree`` Tree used to simplify the GDTF DMX Channels handle at runtime.
- ``FCPGDTFImporterUtils`` Multi purpose utils used everywhere in the project.  
- ``FPulseEffectManager`` Pulse effect generator created from the GDTF specification to avoid redundancy over the multiple attributes using it.

## Widgets
Different classes used to generate UI interfaces, context menu content or custom thumbnails rendering.

# Unreal Assets Part
All Unreal Assets are store under the ``Content`` folder.

## Genereic Meshes
Set of all official generic GDTF meshes. The Asset M_CP_Generic is a generic material to apply a dark paint on these basic meshes.

## Material Instances
Material instancing in Unreal Engine is used to change the appearance of a Material without incurring an expensive recompilation of the Material. See [documentation](https://docs.unrealengine.com/5.0/en-US/instanced-materials-in-unreal-engine/)

## Material Functions
Material Functions let you package parts of a Material graph into a reusable asset that you can share to a library and easily insert into other Materials. Their purpose is to streamline Material creation by giving instant access to commonly used networks of Material nodes. See [documentation](https://docs.unrealengine.com/5.0/en-US/unreal-engine-material-functions-overview/)

## Textures
Textures are image assets that are primiarly used in Materials but can also be directly applied outside of Materials, like when using an texture for a heads up display (HUD). See [documentation](https://docs.unrealengine.com/5.0/en-US/textures-in-unreal-engine/)
































