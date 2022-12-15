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
#include "CPGDTFFixtureComponentBase.h"
#include "CPGDTFSimpleAttributeFixtureComponent.generated.h"

/// Base object of all DMXComponents dealing with basic DMX channels
/// \ingroup DMXComp
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = "DMX", Abstract, meta = (RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFSimpleAttributeFixtureComponent : public UCPGDTFFixtureComponentBase {
	
	GENERATED_BODY()

public:
	UCPGDTFSimpleAttributeFixtureComponent() {};

	virtual void BeginPlay();
	//virtual void OnConstruction() {}; // Initializes the component on spawn on a world

	virtual void Setup(FDMXImportGDTFDMXChannel DMXChannel);

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/// Information about linked DMX channel
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXChannelData DMXChannel;

	/// Pushes DMX Values to the Component. Expects normalized values in the range of 0.0f - 1.0f
	void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) override;

	  /*******************************************/
	 /*          Interpolation Related          */
	/*******************************************/

	/// The currently Interpolated (or not) value.
	FChannelInterpolation CurrentValue;

	/// Called each tick when interpolation is enabled, to calculate the next value 
	virtual void InterpolateComponent(float DeltaSeconds) override;

	/// Gets the interpolation delta value (step) for this frame
	UFUNCTION(BlueprintCallable, Category = "DMX")
		float GetDMXInterpolatedStep() const;

	/// Gets the current interpolated value
	UFUNCTION(BlueprintCallable, Category = "DMX")
		float GetDMXInterpolatedValue() const;

	/// Gets the target value towards which the component interpolates
	UFUNCTION(BlueprintCallable, Category = "DMX")
		float GetDMXTargetValue() const;

	/// True if the target value is reached and no interpolation is required
	bool IsDMXInterpolationDone() const override;

	/// Called to set the value. When interpolation is enabled this function is called by the plugin until the target value is reached, else just once.
	virtual void SetValueNoInterp(float NewValue) {};

	/// Sets the target value. Interpolates to the value if bUseInterpolation is true. Expects the value to be in value range of the component
	virtual void SetTargetValue(float AbsoluteValue);

	/// Applies the speed scale property 
	void ApplySpeedScale() override;

	/// Maps the normalized value to the compoenent's value range
	float NormalizedToAbsoluteValue(float Alpha) const;

	/// Retuns true if the target differs from the previous target, and when interpolating, from the current value
	bool IsTargetValid(float Target);
};
