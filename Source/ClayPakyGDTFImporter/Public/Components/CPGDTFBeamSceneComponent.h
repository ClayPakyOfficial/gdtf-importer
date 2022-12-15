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
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "CPGDTFDescription.h"

#include "CPGDTFBeamSceneComponent.generated.h"

/** Helper Object who contains a complete light output tree */

/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = (GDTF), meta = (BlueprintSpawnableComponent, DisplayName = "Beam Component", RestrictedToClasses = "ACPGDTFFixtureActor"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFBeamSceneComponent : public USceneComponent
{
	GENERATED_BODY()

	/*********************************
	 *           C++ Only            *
	 *********************************/

protected:

	//~ Begin UActorComponent Interface
	#if WITH_EDITOR
		virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif
	//~ End UActorComponent interface

	void UpdateProperties();

public:	

	/// Sets default values for this component's properties
	UCPGDTFBeamSceneComponent();

	/// Called when the game starts or when spawned
	void BeginPlay();
	/// Called during Actor spawn to a specific world
	void OnConstruction();

	/**
	 * Setup the component for a specific GDTF Beam Description
	 * @author Dorian Gardes - Clay Paky S.P.A.
     * @date 08 june 2022
	 * 
	 * @param BeamDescription Description of the Beam to construct
	 * @param Model Model linked to this beam if any on GDTF file (represent Lens mesh)
	 * @param FixturePathOnContentBrowser Path of the fixture in Content Browser (ex: "/Game/MyMovingHead")
	 * @return True if everything OK.
	*/
	bool PreConstruct(UCPGDTFDescriptionGeometryBeam* BeamDescription, FCPGDTFDescriptionModel* Model, FString FixturePathOnContentBrowser);

	/*********************************
	 *        BP Accessible          *
	 *********************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UArrowComponent* OcclusionDirection;

	/// Enable/disable cast shadow on the spotlight and pointlight
	UPROPERTY(EditAnywhere, BlueprintSetter = SetLightCastShadow, Category = "DMX Light Fixture Beam Components")
		bool LightCastShadow;

	/// Intensity (in Lumens) of the light source (set by GDTF Beam Description)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Light Fixture Beam Components")
		float LightIntensityMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Light Fixture Beam Components")
		float LensRadius;

	/// Sets if the light should cast shadows
	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void SetLightCastShadow(bool bLightShouldCastShadow);

	/// Sets Attenuation Radius on the spotlight(s) and pointlight(s)
	UPROPERTY(EditAnywhere, BlueprintSetter = SetLightDistanceMax, Category = "DMX Light Fixture Beam Components")
		float LightDistanceMax;

	/// Sets a new max light distance
	UFUNCTION(BlueprintCallable, Category = "DMX Fixture")
		void SetLightDistanceMax(float NewLightDistanceMax);

	/// Light color temperature on the spotlight(s) and pointlight(s)
	UPROPERTY(EditAnywhere, BlueprintSetter = SetLightColorTemp, Category = "DMX Light Fixture Beam Components")
		float LightColorTemp;

	/// Light color temperature on the spotlight(s) and pointlight(s)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Light Fixture Beam Components")
		float DefaultLightColorTemp;

	/// Sets a new light color temperature
	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void SetLightColorTemp(float NewLightColorTemp);

	/// Sets a new light color temperature
	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void SetLightColor(FLinearColor NewLightColor);

	/// Reset color temp changes during simulation
	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void ResetLightColorTemp() { this->SetLightColorTemp(this->DefaultLightColorTemp); };

	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void ToggleLightVisibility();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DMX Light Fixture Beam Components")
		FTransform PreviousHeadTransform;

	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		bool IsMoving();

	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void CheckOcclusion();

	/*********************************
	 *             Beam              *
	 *********************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UStaticMeshComponent* BeamStaticMeshComponent;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstance* BeamMaterialInstance;

	UPROPERTY(BlueprintReadOnly, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstanceDynamic* DynamicMaterialBeam;

	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void SetBeamQuality(float Quality);

	/*********************************
	 *           SpotLight           *
	 *********************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class USpotLightComponent* SpotLight;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstance* SpotLightMaterialInstance;

	UPROPERTY(BlueprintReadOnly, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstanceDynamic* DynamicMaterialSpotLight;

	/// Scales spotlight intensity
	UPROPERTY(EditAnywhere, BlueprintSetter = SetSpotlightIntensityScale, Category = "DMX Light Fixture Beam Components")
		float SpotlightIntensityScale;

	/// Sets a new spotlight intensity scale
	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void SetSpotlightIntensityScale(float NewSpotlightIntensityScale);

	/*********************************
	 *          PointLight           *
	 *********************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UPointLightComponent* PointLight;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstance* PointLightMaterialInstance;

	UPROPERTY(BlueprintReadOnly, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstanceDynamic* DynamicMaterialPointLight;

	/// Scales pointlight intensity
	UPROPERTY(EditAnywhere, BlueprintSetter = SetPointlightIntensityScale, Category = "DMX Light Fixture Beam Components")
		float PointlightIntensityScale;

	/// Sets a new pointlight intensity scale
	UFUNCTION(BlueprintCallable, Category = "DMX Light Fixture Beam Components")
		void SetPointlightIntensityScale(float NewPointlightIntensityScale);

	/*********************************
	 *             Lens              *
	 *********************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UStaticMeshComponent* LensStaticMeshComponent;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstance* LensMaterialInstance;

	UPROPERTY(BlueprintReadOnly, Category = "DMX Light Fixture Beam Components")
		class UMaterialInstanceDynamic* DynamicMaterialLens;
};
