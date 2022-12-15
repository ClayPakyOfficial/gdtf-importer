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


#include "Components/DMXComponents/MultipleAttributes/ColorCorrection/CPGDTFCTOFixtureComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UCPGDTFCTOFixtureComponent::Setup(FName AttachedGeometryNamee, TArray<FDMXImportGDTFDMXChannel> DMXChannels) {

	Super::Setup(AttachedGeometryNamee, DMXChannels);
	if (DMXChannels[0].Offset.Num() > 0) this->ChannelAddress = DMXChannels[0].Offset[0];
	else this->ChannelAddress = -1;
	this->GDTFDMXChannelDescription = DMXChannels[0];
	this->DMXChannelTree.Insert(this->GDTFDMXChannelDescription.LogicalChannels[0], this->GDTFDMXChannelDescription.Offset.Num());

	this->bIsRawDMXEnabled = true;
}

void UCPGDTFCTOFixtureComponent::BeginPlay() {

	Super::BeginPlay();
	if (this->DMXChannelTree.IsEmpty()) this->DMXChannelTree.Insert(this->GDTFDMXChannelDescription.LogicalChannels[0], this->GDTFDMXChannelDescription.Offset.Num());
	this->bIsRawDMXEnabled = true; // Just to make sure
	this->IsCTOEnabled = false;
}

void UCPGDTFCTOFixtureComponent::PushDMXRawValues(UDMXEntityFixturePatch* FixturePatch, const TMap<int32, int32>& RawValuesMap) {

	if (this->DMXChannelTree.IsEmpty()) return;

	const int32* DMXValuePtr = RawValuesMap.Find(this->ChannelAddress);
	if (DMXValuePtr) {

		TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> DMXBehaviour = this->DMXChannelTree.GetBehaviourByDMXValue(*DMXValuePtr);
		// If we are unable to find the behaviour in the tree we can't do anything
		if (DMXBehaviour.Key == nullptr || DMXBehaviour.Value == nullptr) return;

		if (DMXBehaviour.Value->DMXFrom.Value == 0) { // Disable CTO
			this->IsCTOEnabled = false;
			this->SetValueNoInterp(-1);
			return;
		}

		float PhysicalValue = UKismetMathLibrary::MapRangeClamped(*DMXValuePtr, DMXBehaviour.Value->DMXFrom.Value, DMXBehaviour.Value->DMXTo.Value, DMXBehaviour.Value->PhysicalFrom, DMXBehaviour.Value->PhysicalTo);

		this->ColorTemperature = PhysicalValue;
		this->IsCTOEnabled = true;
		this->SetValueNoInterp(this->ColorTemperature);
	}
}

void UCPGDTFCTOFixtureComponent::InterpolateComponent(float DeltaSeconds) {

	if (this->AttachedBeams.Num() < 1) return;

	if (this->IsCTOEnabled) this->SetValueNoInterp(this->ColorTemperature);
}

void UCPGDTFCTOFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Value) {

	if (this->IsCTOEnabled) Beam->SetLightColorTemp(Value);
	else if (Value == -1) Beam->ResetLightColorTemp();
}