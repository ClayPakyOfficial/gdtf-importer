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

#pragma once

#include "CoreMinimal.h"
#include "CPGDTFDescription.h"
#include "Components/CPGDTFBeamSceneComponent.h"

class ACPGDTFFixtureActor;

/**
 * Manage the Scene components tree of our Actor
 */
class FActorGeometryTree {

private:

	ACPGDTFFixtureActor* ParentActor;

	/// Prefix to make subgeometries unique using geometry references
	FString NamePrefix = "";

public:

	/// Equals to "CPSM_"
	static constexpr const TCHAR* PREFIX_STATIC_MESH = TEXT("CPSM_");
	/// Equals to "CPBEAM_"
	static constexpr const TCHAR* PREFIX_BEAM = TEXT("CPBEAM_");

	FActorGeometryTree() {};
	
	~FActorGeometryTree();

	/**
	 * Map to simplify access to Components at Runtime
	 */
	TMap<FName, USceneComponent*> Components;

	/**
	 * Map to simplify access to Components at Runtime
	 */
	TMap<FName, UCPGDTFBeamSceneComponent*> BeamComponents;

	/**
	 * Creates the object, all the SceneComponent tree and attach them to the Actor
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 13 June 2022
	 * 
	 * @param Actor Actor to attach the components
	 * @param FixturePackagePath
	 * @param DMXModeIndex Index of the DMX Mode
	 */
	void CreateGeometryTree(ACPGDTFFixtureActor* Actor, FString FixturePackagePath, int DMXModeIndex = 0);

	/**
	 * Clean the SceneComponent tree of the Actor
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 09 September 2022
	 */
	void DestroyGeometryTree();

	/**
	 * Fill the map with existing Actor Geometries
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 22 June 2022
	 * 
	 * @param Actor Parent Actor
	 */
	void ReParseGeometryTree(ACPGDTFFixtureActor* Actor);

	/**
	 * Get all the beams under a given geometry name
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 27 June 2022
	 *
	 * @param GeometryName
	 * @return Array of all beam subgeometries
	 */
	TArray<UCPGDTFBeamSceneComponent*> GetBeamsUnderGeometry(FName GeometryName);

private:

	/**
	 * Get all the beams under a given geometry name
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 27 June 2022
	 *
	 * @param GeometryName
	 * @param StartingNode Starting point for the search
	 * @return Array of all beam subgeometries
	 */
	TArray<UCPGDTFBeamSceneComponent*> GetBeamsUnderGeometry_Internal(FName GeometryName, USceneComponent* StartingNode = nullptr);

	/**
	 * Return the array of the first level of the geometry tree
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 09 September 2022
	 * 
	 * @return TArray<UCPGDTFDescriptionGeometryBase*>
	*/
	TArray<UCPGDTFDescriptionGeometryBase*> GetGDTFTopLevelGeometries();

	/**
	 * Creates a branch of the tree (the given geometry and all his childrens)
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 15 June 2022
	 * 
	 * @param Parent
	 * @param Geometry
	 * @param Models
	 * @param FixturePackagePath
	 * @return Created branch
	 */
	USceneComponent* CreateTreeBranch(USceneComponent* Parent, UCPGDTFDescriptionGeometryBase* Geometry, UCPGDTFDescriptionModels* Models, FString FixturePackagePath);

	/**
	 * Destroy a branch of the tree (All the childrens of the given USceneComponent)
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 09 September 2022
	 *
	 * @param Parent
	 */
	void DestroyTreeBranch(USceneComponent* Parent);

	/**
	 * Parse a branch of the tree (the given geometry and all his childrens)
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 15 June 2022
	 * 
	 * @param BranchRootComponent
	 */
	void ParseTreeBranch(USceneComponent* BranchRootComponent);

	/**
	 * Creates a StaticMeshComponent and attach it to the parent.
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 13 June 2022
	 * 
	 * @param Parent
	 * @param Geometry
	 * @param Models
	 * @param FixturePackagePath
	 * @return 
	 */
	bool CreateStaticMeshComponentChild(USceneComponent* Parent, UCPGDTFDescriptionGeometryBase* Geometry, UCPGDTFDescriptionModels* Models, FString FixturePackagePath);

	/**
	 * Creates a CPGDTFBeamSceneComponent and attach it to the parent.
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 15 June 2022
	 *
	 * @param Parent
	 * @param Geometry
	 * @param Models
	 * @param FixturePackagePath
	 * @return
	 */
	bool CreateBeamComponentChild(USceneComponent* Parent, UCPGDTFDescriptionGeometryBeam* Geometry, UCPGDTFDescriptionModels* Models, FString FixturePackagePath);


	/**
	 * Creates a USceneComponent and attach it to the parent.
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 13 June 2022
	 * 
	 * @param Parent Component to attach the newly created. If NULL we set the component at RootComponent
	 * @param Name
	 * @return Created Component
	 */
	USceneComponent* CreateAndAttachSceneComponent(USceneComponent* Parent, FName Name);
};
