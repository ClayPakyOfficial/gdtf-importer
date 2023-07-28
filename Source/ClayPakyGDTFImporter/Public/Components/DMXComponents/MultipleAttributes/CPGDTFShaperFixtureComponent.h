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
#include "Kismet/KismetMathLibrary.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Factories/CPGDTFRenderPipelineBuilder.h"
#include "Components/DMXComponents/CPGDTFMultipleAttributeFixtureComponent.h"
#include "CPGDTFShaperFixtureComponent.generated.h"


/**
 * Component to manage a single Shaper blade
 * \ingroup DMXComp
 */
 /// \cond NOT_DOXYGEN
UCLASS(ClassGroup = (DMX), Meta = (BlueprintSpawnableComponent, DisplayName = "Shaper Component", RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFShaperFixtureComponent : public UCPGDTFMultipleAttributeFixtureComponent {

	GENERATED_BODY()

protected:

	enum InterpolationIds {
		BLADE_A,
		BLADE_BROT,
		SHAPER_ROT
	};

	//Param name inside the materials that controls the blade A
	UPROPERTY(BlueprintReadOnly)
	FName mBladeAParamName;
	//Param name inside the materials that controls the blade B+rot
	UPROPERTY(BlueprintReadOnly)
	FName mBladeBRotParamName;
	//Param name inside the materials that controls the blade orientation (and the movement of the whole shaper system)
	UPROPERTY(BlueprintReadOnly)
	FName mBladeOrientationName;

	//Orientation of the blade (0 = top, 1 = right, 2 = bottom, 3 = left)
	UPROPERTY(BlueprintReadOnly)
	int orientation;

	//If true the shape is controlled with param A + param B. If false, A + Rot is used. This parameter is used only in the Render Pipeline Builder
	UPROPERTY(BlueprintReadOnly)
	bool abMode;

public:
	UCPGDTFShaperFixtureComponent() {};
	~UCPGDTFShaperFixtureComponent() {};

	bool Setup(TArray<FDMXImportGDTFDMXChannel> DMXChannels, int attributeIndex) override;

	void BeginPlay() override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	/**
	 * Read DMXValue and apply the effect to the Beam
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 31 january 2022
	 *
	 * @param DMXValue
	 */
	void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) override;

	FORCEINLINE int getOrientation() {
		return orientation;
	}

	FORCEINLINE bool isInAbMode() {
		return abMode;
	}

protected:

	TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups() override;
	virtual float getDefaultRealAcceleration(FCPDMXChannelData& channelData, int interpolationId) override;
	virtual float getDefaultRealFade(FCPDMXChannelData& channelData, int interpolationId) override;

	void SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float value, int interpolationId) override;
};
