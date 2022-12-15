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
#include "Components/DMXComponents/CPGDTFMultipleAttributeBeamFixtureComponent.h"
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
class CLAYPAKYGDTFIMPORTER_API UCPGDTFGoboWheelFixtureComponent : public UCPGDTFMultipleAttributeBeamFixtureComponent {

	GENERATED_BODY()

protected:

	ECPGDTFAttributeType RunningEffectTypeChannelOne = ECPGDTFAttributeType::Gobo_n_;
	ECPGDTFAttributeType RunningEffectTypeChannelTwo = ECPGDTFAttributeType::Gobo_n_;

	FDMXChannelTree DMXChannelTreeOne;
	FDMXChannelTree DMXChannelTreeTwo;

	UPROPERTY()
		FDMXImportGDTFDMXChannel GDTFDMXChannelDescriptionOne;

	UPROPERTY()
		FDMXImportGDTFDMXChannel GDTFDMXChannelDescriptionTwo;

	UPROPERTY()
		UTexture2D* WheelTexture;
	UPROPERTY()
		UTexture2D* WheelTextureFrosted;

	/// Number of gobos on the wheel
	UPROPERTY()
		int NbrGobos;

	//Specific for effects interpolation

	FChannelInterpolation CurrentWheelIndex;
	float GoboCurrentAngle;

	float ShakeBaseIndex;
	float GoboWheelPeriod; // Entire Wheel rotation
	float WheelCurrentTime;
	
	float GoboRotationPeriod; // Gobo rotation (unused on static gobo wheels)
	float RotationCurrentTime;

public:
	UCPGDTFGoboWheelFixtureComponent() {};

	void Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels) override;

	void BeginPlay();
	//void OnConstruction() override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "512", DisplayName = "Gobo Selection Channel Address"), Category = "DMX Channels")
		int32 AddressChannelOne;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "512", DisplayName = "Gobo Rotation Channel Address"), Category = "DMX Channels")
		int32 AddressChannelTwo;

	/// Pushes DMX Values to the Component. Expects normalized values in the range of 0.0f - 1.0f
	virtual void PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap) override;

	void SetTargetValue(float WheelIndex);

	/**
	 * Read DMXValue and apply the effect to the Beam
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 27 july 2022
	 *
	 * @param DMXValue
	 * @param IsFirstChannel
	*/
	void ApplyEffectToBeam(int32 DMXValue, bool IsFirstChannel = true);

	void InterpolateComponent(float DeltaSeconds) override;

	/*******************************************/
   /*           Component Specific            */
  /*******************************************/

protected:

	/**
	 * Apply a Gobo to the light entire output
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 27 july 2022
	 *
	 * @param Beam
	 * @param WheelIndex
	 */
	void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float WheelIndex) override;
	
	/**
	 * Set a rotation on the gobo
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 29 july 2022
	 * 
	 * @param RotationAngle
	 */
	void SetValueNoInterpGoboRotation(float RotationAngle);

	/**
	 * Set a rotation on the gobo
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 03 august 2022
	 *
	 * @param DeltaRotationAngle
	 */
	void SetValueNoInterpGoboDeltaRotation(float DeltaRotationAngle);
};
