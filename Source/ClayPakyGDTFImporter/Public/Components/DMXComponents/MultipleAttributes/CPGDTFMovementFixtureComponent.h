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
class CLAYPAKYGDTFIMPORTER_API UCPGDTFMovementFixtureComponent : public UCPGDTFMultipleAttributeFixtureComponent {

	GENERATED_BODY()

protected:

	FPulseEffectManager PulseManagerP, PulseManagerT;

	UPROPERTY(EditAnywhere, Category = "Movement Component")
	bool bInvertPan = false;
	UPROPERTY(EditAnywhere, Category = "Movement Component")
	bool bInvertTilt = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	USceneComponent* geometryP;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	USceneComponent *geometryT;

	UPROPERTY(EditAnywhere, Category = "Movement Component")
	float minValP;
	UPROPERTY(EditAnywhere, Category = "Movement Component")
	float maxValP;
	UPROPERTY(EditAnywhere, Category = "Movement Component")
	float minValT;
	UPROPERTY(EditAnywhere, Category = "Movement Component")
	float maxValT;

	float valueP, valueT;
	bool infinitePenabled = false, infiniteTenabled = false;
	float directionP = 0, directionT = 0;

	virtual float getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) override;


public:
	UCPGDTFMovementFixtureComponent() {};

	/*******************************************/
   /*           Component Specific            */
  /*******************************************/

	bool Setup(TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) override;

	void BeginPlay() override;
	/*******************************************/
	/*               DMX Related               */
	/*******************************************/

	/**
	 * Read DMXValue and apply the effect to the Beam
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 21 july 2022
	 *
	 * @param DMXValue
	 */
	void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) override;

	void InterpolateComponent_BeamInternal(float deltaSeconds, FCPComponentChannelData& channel) override;

	/*******************************************/
   /*           Component Specific            */
  /*******************************************/

protected:

	TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups() override;

	void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float value, int interpolationId) override;

	void SetTargetValue(float value, int interpolationId) override;
};