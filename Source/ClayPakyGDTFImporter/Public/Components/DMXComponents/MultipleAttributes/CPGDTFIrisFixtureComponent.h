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
#include "Utils/CPGDTFPulseEffectManager.h"
#include "Components/DMXComponents/CPGDTFMultipleAttributeFixtureComponent.h"
#include "Factories/CPGDTFRenderPipelineBuilder.h"
#include "CPGDTFIrisFixtureComponent.generated.h"

/**
 * Component to manage Frost
 * \ingroup DMXComp
 */
 /// \cond NOT_DOXYGEN
UCLASS(ClassGroup = (DMX), Meta = (BlueprintSpawnableComponent, DisplayName = "Iris Component", RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFIrisFixtureComponent : public UCPGDTFMultipleAttributeFixtureComponent {
	GENERATED_BODY()

protected:
	//Specific for effects interpolation

	FPulseEffectManager PulseManager;

	//Param name inside the materials that controls the iris
	UPROPERTY(BlueprintReadOnly)
	FName mIrisParamName;
public:
	UCPGDTFIrisFixtureComponent() {}
	~UCPGDTFIrisFixtureComponent() {}

	bool Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) override;

	void BeginPlay() override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) override;

protected:
	virtual float getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) override;

	TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups() override;

	void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float value, int interpolationId) override;

	void InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) override;

	void StartPulseEffect(ECPGDTFAttributeType AttributeType, float Period, FCPGDTFDescriptionChannelFunction* ChannelFunction);
};
