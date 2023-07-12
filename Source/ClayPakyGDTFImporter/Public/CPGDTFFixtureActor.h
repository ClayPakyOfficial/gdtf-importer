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
#include "Components/CPGDTFBeamSceneComponent.h"
#include "Utils/CPFActorGeometryTree.h"
#include "Game/DMXComponent.h"
#include "DMXTypes.h"

#include "CPGDTFFixtureActor.generated.h"

UENUM()
enum ECPGDTFFixtureQualityLevel
{
	LowQuality		UMETA(DisplayName = "Low"),
	MediumQuality	UMETA(DisplayName = "Medium"),
	HighQuality		UMETA(DisplayName = "High"),
	UltraQuality	UMETA(DisplayName = "Ultra"),
	Custom			UMETA(DisplayName = "Custom")
};

/// Base class for virtual generated fixtures Blueprints
UCLASS(BlueprintType, Blueprintable, AutoExpandCategories = DMX, HideCategories = Internal)
class CLAYPAKYGDTFIMPORTER_API ACPGDTFFixtureActor : public AActor {
	
	GENERATED_BODY()
	
protected:
	//~ Begin AActor Interface
	#if WITH_EDITOR
		virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif
	//~ End AActor interface

	void UpdateProperties();

public:

	//@param FixturePackagePath Path of the package (folder) containing the fixture assets. For example: "/Game/ClayPaky_MiniB"
	ACPGDTFFixtureActor();

	//~ Begin AActor Interface
	void OnConstruction(const FTransform& Transform); // Called during Actor spawn to a specific world
	void BeginDestroy(); // Called when the Garbage Collector delete the actor from memory. See https://docs.unrealengine.com/5.0/en-US/unreal-engine-actor-lifecycle/
	void BeginPlay(); // Called when the game starts or when spawned
	void EndPlay(const EEndPlayReason::Type EndPlayReason); // Called when the game stops or when destroyed
	void Tick(float DeltaTime); // Called every frame
	//~ End AActor Interface

	FTimerHandle CheckOcclusionTimer;
	FActorGeometryTree GeometryTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Internal)
		UCPGDTFDescription* GDTFDescription;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Internal)
		FString CurrentModeName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Internal)
		int CurrentModeIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Internal)
		FString ActorsPathInContentBrowser; //Folder inside FixturePathInContentBrowser that contains an actor for each dmx mode of the fixture
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Internal)
		FString FixturePathInContentBrowser; //Base folder of the fixture

	FString getClassNameFromMode(FString modeName); //Returns the name of the class of this same fixture, but with the specified mode name

	/**
	 * Setup the actor for a specific GDTFDescription.
	 * Used during the generation of the Asset by the GDTFFactory.
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 25 may 2022
	 *
	 * @param FixtureGDTFDescription GDTF Description of the Fixture
	 */
	void PreConstruct(UCPGDTFDescription* FixtureGDTFDescription);

	/**
	 * Creates the rendering pipeline for each mode of the specified fixture. At the end of this process the DMX components will be cleared. This should be called before constructing the geometry tree
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 22 may 2023
	 *
	 * @param FixtureGDTFDescription GDTF Description of the Fixture
	 */
	void CreateRenderingPipelines(UCPGDTFDescription* FixtureGDTFDescription);

	/*******************************************************
	 *                   END OF C++ ONLY                   *
	 *******************************************************
	 

	 *******************************************************
	 *            BEGIN OF BLUEPRINT ACCESSIBLE            *
	 *******************************************************/


	// VISUAL QUALITY LEVEL----------------------

	/// Visual quality level that changes the number of samples in the volumetric beam
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DMX Light Fixture", meta = (DisplayPriority = 0))
	TEnumAsByte<ECPGDTFFixtureQualityLevel> QualityLevel = ECPGDTFFixtureQualityLevel::HighQuality;

	/// Visual quality when using smaller zoom angle (thin beam). Small value is visually better but cost more on GPU
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DMX Light Fixture", meta = (EditCondition = "QualityLevel == ECPGDTFFixtureQualityLevel::Custom", EditConditionHides))
	float MinQuality;

	/// Visual quality when using bigger zoom angle (wide beam). Small value is visually better but cost more on GPU
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DMX Light Fixture", meta = (EditCondition = "QualityLevel == ECPGDTFFixtureQualityLevel::Custom", EditConditionHides))
	float MaxQuality;

	// PARAMETERS---------------------------------

	/// Simple solution useful for walls, 1 linetrace from the center
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DMX Light Fixture")
		bool UseDynamicOcclusion;

	/// DMX COMPONENT
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DMX Light Fixture")
		class UDMXComponent* DMX;


	/*******************************************************
	 *                END OF BP PROPERTIES                 *
	 *******************************************************


	 *******************************************************
	 *                 BEGIN OF BP METHODS                 *
	 *******************************************************/

	/// Pushes DMX Values to the Fixture. Expects normalized values in the range of 0.0f - 1.0f
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& ValuePerAttributeMap);

	#if WITH_EDITOR
	/// Called by Unreal when patch changed
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void ChangeFixtureMode(const UDMXEntityFixturePatch* FixturePatch);
	#endif

	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void ToggleLightVisibility();

	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		bool IsMoving();

	/// Sets a new max light distance
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetLightDistanceMax(float NewLightDistanceMax);

	/// Sets a new light color temperature
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetLightColorTemp(float NewLightColorTemp);

	/// Sets a new spotlight intensity scale
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetSpotlightLightIntensityScale(float NewSpotlightIntensityScale);

	/// Sets a new spotlight intensity scale
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetSpotlightBeamIntensityScale(float NewSpotlightIntensityScale);

	/// Sets a new spotlight intensity scale
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetSpotlightLensIntensityScale(float NewSpotlightIntensityScale);

	/// Sets a new pointlight intensity scale
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetPointlightIntensityScale(float NewPointlightIntensityScale);

	/// Sets if the light should cast shadows
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetLightCastShadow(bool bLightShouldCastShadow);

	/// Call to check dynamic occlusion
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void CheckOcclusion();
};