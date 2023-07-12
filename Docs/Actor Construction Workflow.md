
# Actor Construction Workflow

## Step 0 : GDTF Import
1. Import of GDTF Description
2. Parsing of GDTF Description
3. Import of PNG textures
4. Import of 3D models

## Step 1 : Creation of Actor in Content Browser
1. Creation of an instance of a CPGDTFFixtureActor
2. Creation of the Geometries and the DMXComponents based on GDTF Description (ACPGDTFFixtureActor::PreConstruct() method called)
3. Creation of a Blueprint based on this Actor instance (CreateBlueprintFromActor())
4. Save this Blueprint in Content Browser (Actor instance is dropped)

## Step 2 : Spawning the BluePrint Actor in a world
__Note :__ This can be on a Edit window or on a more standard "Game" world.
1. Unreal call SpawnActor and initialize the Actor with multiple methods calls (more details on Remarks section of [AActor C++ Reference](https://docs.unrealengine.com/5.0/en-US/API/Runtime/Engine/GameFramework/AActor/))
2. ACPGDTFFixtureActor::OnConstruction() is called by Unreal
3. Recreation of GeometryTree Util Object
4. Initialization of BeamComponents with UCPGDTFBeamSceneComponent::OnConstruction()
5. Initialization of DMXComponents with UCPGDTFFixtureComponentBase::OnConstruction() (and subobjects)

## Step 3 : Start of the simulation
1. Unreal call ACPGDTFFixtureActor::BeginPlay() on all the Actors
  1. Enable dynamic occlusion if enabled in settings
  2. Bind the main DMX component (called "DMX") to the OnFixturePatchReceived event.
2. Unreal call UCPGDTFBeamSceneComponent::BeginPlay() on BeamComponents
  1. Creation of the DynamicMaterials to render the Beam
3. Unreal call BeginPlay() on DMXFixtureComponents
  1. Setup of Interpolation if enabled in settings

