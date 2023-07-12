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
#include "CPGDTFMultipleAttributeFixtureComponent.h"
#include "CPGDTFColorSourceFixtureComponent.generated.h"

/// Extension of FCPDMXChannelData to store the color managed by a specific channel
/// \ingroup DMXComp
USTRUCT(BlueprintType)
struct FCPDMXColorChannelData : public FCPDMXChannelData {

	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "DMX Channel")
		ECPGDTFAttributeType ColorAttribute = ECPGDTFAttributeType::NoFeature;

	FCPDMXColorChannelData() {}

	FCPDMXColorChannelData(FDMXImportGDTFDMXChannel Channel, ECPGDTFAttributeType ColorAttribute_);
};

/// Abstract Object who regroup all the colored sources DMXComponents
UCLASS(Abstract)
class UCPGDTFColorSourceFixtureComponent : public UCPGDTFMultipleAttributeFixtureComponent {

	GENERATED_BODY()

protected:

	FLinearColor CurrentColor;

public:

	void BeginPlay(int interpolationsNeededNo, float RealFade, float RealAcceleration, float rangeSize, float defaultValue) override;
	void BeginPlay(TArray<FCPDMXChannelData> interpolationValues) override;
	//void OnConstruction() override;
	
	virtual void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue);
	virtual void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* beam, float value, int interpolationId);

	  /*******************************************/
	 /*           Component Specific            */
	/*******************************************/

	UFUNCTION(BlueprintCallable, Category = "DMX")
		FLinearColor GetCurrentColor() { return this->CurrentColor; };
};