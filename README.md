# Unreal GDTFImporter by Clay Paky

<div align="center" style="background-color: black; padding: 40px 0 20px 0;">
<img src="Resources/splash_docs.png" srcset="splash_docs.png" width="60%"/>
<h1 align="center" style="color: white !important;">Unreal GDTFImporter by Clay Paky</h1></div>
<h2><em><strong>WARNING :</strong></em> This plugin is still under development.
</h2>
<h2><em><strong>IMPORTANT :</strong></em> In case of fixture(s) inaccurate behaviour, before looking for a bug, please start by checking the GDTF definition. A lot of them are bad formatted or provide values who doesn't fit the real behavior.
</h2>


## TODO

- Do the TODOs
- Improve documentation
- Fix Lens texture not centered
- Check the accuracy of rotation matrix parsing in ``FCPGDTFImporterUtils``
- Add missing attributes supports on Color and Gobo Wheel components
- Create generic parametric BeamType assets on unreal (See ``ECPGDTFDescriptionGeometryBeamType``)
- ADD laser support inspired by existing implementation on DMXFixtures plugin
- linear frost?

## Idea of improvement

Add a option in the interface to ask the DMXLibrary to use and create the Fixture type automaticaly.
<br>To do see there =>
- UDMXEntityFixtureType::CreateFixtureTypeInLibrary
- FDMXEntityFixtureTypeConstructionParams
-	SDMXEntityDropdownMenu

## Known Limitations

Solvable
- Channels modifiying the behaviour of other channels is not supported

Permanent
- Bad GDTF Files (Some stuff can be worked around)
- Incompatible with GDTF < 1.2 because of ``.3ds`` models only
- Focus not relevant because of the way of Unreal Engine render spotlights light functions.

## Features

*TODO*

## Installing

### First way 'Project Plugin'
1. If your project is open in Unreal close it.
2. Create a ``Plugins`` folder in your project folder.
3. Extract the plugin.
4. Apply the `dmxEngine.patch` file to the Unreal Engine's codebase using the command `git apply dmxEngine.patch`.
5. Recompile Unreal Engine.
6. Launch the project in Unreal.
7. To confirm that the plug-in has been successfully installed and enabled, during a GDTF import the option windows should specify if this is the Clay Paky GDTF Importer.

![Clay Paky GDTF import option window](Resources/ImportWindow.png)

### Second way 'Engine Plugin'
Follow the same procedure than the 'Project Plugin installation' but extract the plugin in ``<Unreal Engine Install Folder>\Engine\Plugins`` folder.

## Developement
1. Create an new empty project (Film/Video & Live Events => Blank).
2. Create a C++ class (Tools => New C++ Class).
3. Generate Visual Studio project (Tools => Refresh Visual Studio Project).
4. Open the project in Visual Studio (Tools => Open Visual Studio).
5. Close Unreal Engine.
6. Build the project (On the the Solution Explorer under the Game folder right click on the project and Build).
7. Close Visual Studio.
8. Create a ``Plugins`` folder in your project folder.
9. Extract the plugin.
10. Right click on the ``<project_name>.uproject`` and Generate Visual Studio project files.
11. Open the project in Visual Studio.
12. Build the project.
13. Now you can open Unreal Engine and try the plugin.

***Important notes:***
- To create new classes/files Visual Studio store them in wrong folders. Use the C++ class wizard on Unreal Engine (step 2) or close VS create the files at the good place and regenerate the VS project (step 10).
- ``UPROPERTY()`` macros ending with a semicolon will make Doxygen unable to generate the documentation of the property
- ``UCLASS()`` ``UENUM()`` ``USTRUCT()`` ``GENERATED_BODY()`` macros sould **NEVER** end with a semicolon
- To avoid Doxygen code parsing problems (``warning: Found ';' while parsing initializer list! (doxygen could be confused by a macro call without semicolon)``). Nested Unreal macros calls like ``UCLASS(ClassGroup = (DMX), Meta = (BlueprintSpawnableComponent))`` needs to be excluded of Doxygen parsing with ``/// \ cond NOT_DOXYGEN`` / ``/// \ endcond`` (without space after backslash)
- To edit a .cpp/.h name o change the path of a file, you have to: Close UE4 and VS; Delete/Move the header and source to Private/Public setup you want; Delete .vs, Intermediate, Binaries, Saved and DerivedDataCache folders in the File Explorer; Right-Click .uproject file and hit regenerate VS project files; Open .sln and build solution. NOTE: It will take ages to fully rebuild the solution. SOURCE: https://www.reddit.com/r/unrealengine/comments/gbggsn/is_there_a_way_to_set_c_classes_as_public_or/ 

## Generate the docs
***Requires [Doxygen](https://www.doxygen.nl/download.html#srcbin) and [Graphviz](https://graphviz.org/download/#executable-packages) installed***
1. Open a terminal in the root of the repository
2. Execute ``doxygen``
3. Open the file [Docs/html/index.html](Docs/html/index.html)

## Release new version

1. Build plugin as described in [Developement section](#autotoc_md8).
2. In the ``ClayPakyGDTFImporter`` plugin folder delete :
  - ``.git`` folder.
  - ``Docs`` folder.
  - ``Intermediate`` folder.
  - ``Source/Private`` folder.
  - ``Doxyfile`` file.
  - ``README.md`` file.
3. In the Binaries folder delete all the files except ``UnrealEditor-ClayPakyGDTFImporter.dll`` and ``UnrealEditor.modules``.
4. In ``ClayPakyGDTFImporter.uplugin`` text file edit ``IsBetaVersion``, ``Version`` and ``VersionName`` to fit the new release number.
5. Pack the ``ClayPakyGDTFImporter`` plugin folder in an archive.
6. Installation procedure explained in [Installing section](#autotoc_md5)

## Update Unreal Developement Branch
1. Close Unreal Engine and Visual Studio
2. Go to Unreal Engine folder
3. Do a ``git pull``
4. Launch ``./Setup.bat``
5. Launch ``./GenerateProjectFiles.bat``
6. Open UE5 Solution in Visual Studio
7. Build Unreal Engine
8. Open the Clay Paky project in Visual Studio
9. Build the project 