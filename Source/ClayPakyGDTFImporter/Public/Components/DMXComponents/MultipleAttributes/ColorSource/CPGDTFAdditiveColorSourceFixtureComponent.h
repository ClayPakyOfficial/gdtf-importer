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
#include "CPGDTFAdditiveColorSourceFixtureComponent.generated.h"

/**
 * Component to manage complex LED Engines
 * \ingroup DMXComp
 */
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = "DMX", Meta = (BlueprintSpawnableComponent, DisplayName = "Additive Color Source Component", RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFAdditiveColorSourceFixtureComponent : public UCPGDTFAdditiveColorFixtureComponent {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelRed; // ColorAdd_R

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelGreen; // ColorAdd_G

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelBlue; // ColorAdd_B

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelWhite; // ColorAdd_W

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelCyan; // ColorAdd_C

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelMagenta; // ColorAdd_M

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelYellow; // ColorAdd_Y

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelAmber; // ColorAdd_RY

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelLime; // ColorAdd_GY

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelBlueGreen; // ColorAdd_GC

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelLightBlue; // ColorAdd_BC

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelPurple; // ColorAdd_BM
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelPink; // ColorAdd_RM

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelWarmWhite; // ColorAdd_WW

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelCoolWhite; // ColorAdd_CW

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelUV; // ColorAdd_UV

	UCPGDTFAdditiveColorSourceFixtureComponent();

	/// The array is present for future support of more complex LED engines
	virtual bool Setup(TArray<FDMXImportGDTFDMXChannel> InputsAvailables, int attributeIndex) override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/// Pushes DMX Values to the Component. Expects normalized values in the range of 0.0f - 1.0f
	virtual void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) override;
};
