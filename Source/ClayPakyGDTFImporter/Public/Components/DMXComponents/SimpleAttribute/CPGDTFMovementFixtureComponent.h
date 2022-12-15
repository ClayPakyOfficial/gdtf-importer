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
#include "Components/DMXComponents/CPGDTFSimpleAttributeFixtureComponent.h"
#include "CPGDTFMovementFixtureComponent.generated.h"


UENUM(BlueprintType)
enum class ECPGDTFMovementFixtureType : uint8 {
	Pan,
	Tilt
};


/**
 * Component to manage Pan or Tilt
 * \ingroup DMXComp
 */
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = (DMX), Meta = (BlueprintSpawnableComponent, DisplayName = "Pan/Tilt Component", RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFMovementFixtureComponent : public UCPGDTFSimpleAttributeFixtureComponent {

	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category = "Movement Component")
		bool bInvertRotation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Component")
		ECPGDTFMovementFixtureType MovementType;

public:
	UCPGDTFMovementFixtureComponent() {};

	/// Initializes the component on spawn on a world
	void OnConstruction() override;

	void Setup(FDMXImportGDTFDMXChannel DMXChannel) override;

	/// Called to set the value. When interpolation is enabled this function is called by the plugin until the target value is reached, else just once.
	UFUNCTION(BlueprintCallable, Category = "DMX Component")
	void SetValueNoInterp(float Rotation) override;

	  /*******************************************/
	 /*           Component Specific            */
	/*******************************************/

	UFUNCTION(BlueprintCallable, Category = "DMX Component")
		double GetRotation();

	  /*******************************************/
	 /*          Interpolation Related          */
	/*******************************************/

	void SetTargetValue(float AbsoluteValue) override;

	void InterpolateComponent(float DeltaSeconds) override;
};
