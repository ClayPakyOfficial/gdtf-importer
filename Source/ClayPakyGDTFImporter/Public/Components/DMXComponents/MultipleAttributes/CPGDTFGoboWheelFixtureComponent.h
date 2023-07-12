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
#include "Factories/CPGDTFRenderPipelineBuilder.h"
#include "Components/DMXComponents/CPGDTFMultipleAttributeFixtureComponent.h"
#include "CPGDTFGoboWheelFixtureComponent.generated.h"

/**
 * Component to manage GoboWheels
 * 
 * TODO \todo Add support of missing attributes :
 * Gobo(n)SelectEffects ???
 * Gobo(n)WheelAudio // Very complex to implement.
 */
/// \ingroup DMXComp
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = (DMX), Meta = (BlueprintSpawnableComponent, DisplayName = "Gobo Wheel Component", RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFGoboWheelFixtureComponent : public UCPGDTFMultipleAttributeFixtureComponent {

	GENERATED_BODY()

protected:
	enum InterpolationIds {
		GOBO_ROTATION,
		WHEEL_ROTATION
	};

	UPROPERTY()
	UTexture2D* WheelTexture;
	UPROPERTY()
	UTexture2D* WheelTextureFrosted;

	/// Number of gobos on the wheel
	UPROPERTY()
	int NbrGobos;

	//Param name inside the materials that controls the index of the gobo wheel
	UPROPERTY(BlueprintReadOnly)
	FName mIndexParamName;

	//Specific for effects interpolation

	float GoboCurrentAngle;

	float ShakeBaseIndex;
	float GoboWheelPeriod; // Entire Wheel rotation
	float WheelCurrentTime;
	
	float GoboRotationPeriod; // Gobo rotation (unused on static gobo wheels)
	float RotationCurrentTime;

public:
	UCPGDTFGoboWheelFixtureComponent() {};

	bool Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) override;

	void BeginPlay() override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/**
	 * Read DMXValue and apply the effect to the Beam
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 27 july 2022
	 *
	 * @param DMXValue
	 * @param IsFirstChannel
	*/
	void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue);

	void InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) override;

	/*******************************************/
   /*           Component Specific            */
  /*******************************************/

protected:

	TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups() override;

	virtual float getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultAccelerationRatio(float realFade, FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultFadeRatio(float realAcceleration, FCPDMXChannelData& channelData, int interpolationId) override;


	/**
	 * Apply a Gobo to the light entire output
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 27 july 2022
	 *
	 * @param Beam
	 * @param WheelIndex
	 */
	void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* beam, float value, int interpolationId) override;
};
