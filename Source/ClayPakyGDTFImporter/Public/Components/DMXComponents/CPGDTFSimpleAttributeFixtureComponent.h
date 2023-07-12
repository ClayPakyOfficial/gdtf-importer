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
#include "CPGDTFFixtureComponentBase.h"
#include "CPGDTFSimpleAttributeFixtureComponent.generated.h"

/// Base object of all DMXComponents dealing with basic DMX channels
/// \ingroup DMXComp
/// \cond NOT_DOXYGEN
UCLASS(ClassGroup = "DMX", Abstract, meta = (RestrictedToClasses = "ACPGDTFFixtureActor"), HideCategories = ("Variable", "Sockets", "Tags", "Activation", "Cooking", "ComponentReplication", "AssetUserData", "Collision", "Events"))
/// \endcond
class CLAYPAKYGDTFIMPORTER_API UCPGDTFSimpleAttributeFixtureComponent : public UCPGDTFFixtureComponentBase {
	
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	TSet<ECPGDTFAttributeType> mMainAttributes;


public:
	UCPGDTFSimpleAttributeFixtureComponent() {};

	virtual TArray<TSet<ECPGDTFAttributeType>> getAttributeGroups();

	using UCPGDTFFixtureComponentBase::Setup; //https://stackoverflow.com/questions/32100413/disabling-visual-c-virtual-function-override-warning-for-certain-methods
	virtual bool Setup(FDMXImportGDTFDMXChannel DMXChannell, int attributeIndex);

	virtual void BeginPlay() override;

	  /*******************************************/
	 /*               DMX Related               */
	/*******************************************/

	void ApplyEffectToBeam(int32 DMXValue, FCPComponentChannelData& channel, TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*>& DMXBehaviour, ECPGDTFAttributeType& AttributeType, float physicalValue) override;

	  /*******************************************/
	 /*          Interpolation Related          */
	/*******************************************/

	/// Sets the target value. Interpolates to the value if bUseInterpolation is true. Expects the value to be in value range of the component
	virtual void SetTargetValue(float AbsoluteValue);

	/// Retuns true if the target differs from the previous target, and when interpolating, from the current value
	bool IsTargetValid(float Target);
};
