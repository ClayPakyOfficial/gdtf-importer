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
#include "Components/DMXComponents/CPGDTFAdditiveColorFixtureComponent.h"
#include "CPGDTFHSVColorSourceFixtureComponent.generated.h"

 /// Component to manage HSV colors attributes
 /// \ingroup DMXComp
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = "DMX", Meta = (BlueprintSpawnableComponent, DisplayName = "Color Source Component", RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFHSVColorSourceFixtureComponent : public UCPGDTFAdditiveColorFixtureComponent {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelH; // HSB_Hue

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelS; // HSB_Saturation

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelV; // HSB_Brightness

	UCPGDTFHSVColorSourceFixtureComponent();

	/// The array is present for future support of more complex LED engines
	virtual bool Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables, int attributeIndex) override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/// Pushes DMX Values to the Component. Expects normalized values in the range of 0.0f - 1.0f
	virtual void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) override;

protected:
	TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups() override;
};
