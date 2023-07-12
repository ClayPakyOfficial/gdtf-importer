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
#include "Components/DMXComponents/CPGDTFSubstractiveColorFixtureComponent.h"
#include "CPGDTFSubstractiveColorSourceFixtureComponent.generated.h"

/**
 * Component to manage substractive sources engines
 * \ingroup DMXComp
 */
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = "DMX", Meta = (BlueprintSpawnableComponent, DisplayName = "Substractive Color Source Component", RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFSubstractiveColorSourceFixtureComponent : public UCPGDTFSubstractiveColorFixtureComponent {

	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelRed; // ColorSub_R

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelGreen; // ColorSub_G

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelBlue; // ColorSub_B

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelCyan; // ColorSub_C

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelMagenta; // ColorSub_M

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DMX Channels")
		FCPDMXColorChannelData DMXChannelYellow; // ColorSub_Y

	UCPGDTFSubstractiveColorSourceFixtureComponent();

	/// The array is present for future support of more complex LED engines
	virtual bool Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> InputsAvailables, int attributeIndex) override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/// Pushes DMX Values to the Component. Expects normalized values in the range of 0.0f - 1.0f
	virtual void PushNormalizedRawValues(UDMXEntityFixturePatch* FixturePatch, const FDMXNormalizedRawDMXValueMap& RawValuesMap) override;

protected:
	TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups() override;

	virtual float getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) override;

};
