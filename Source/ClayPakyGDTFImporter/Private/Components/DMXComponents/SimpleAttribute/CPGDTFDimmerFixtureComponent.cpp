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


#include "Components/DMXComponents/SimpleAttribute/CPGDTFDimmerFixtureComponent.h"

void UCPGDTFDimmerFixtureComponent::BeginPlay() {
	Super::BeginPlay();
	this->SetValueNoInterp(0.0f);
}

void UCPGDTFDimmerFixtureComponent::InterpolateComponent(float DeltaSeconds) {

	if (this->AttachedBeams.Num() < 1) return;

	Super::InterpolateComponent(DeltaSeconds);
}

void UCPGDTFDimmerFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Intensity) {

	if (Beam == nullptr) return;

	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetScalarParameterValue("DMX Dimmer", Intensity);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetScalarParameterValue("DMX Dimmer", Intensity);
	if (Beam->DynamicMaterialPointLight) Beam->DynamicMaterialPointLight->SetScalarParameterValue("DMX Dimmer", Intensity);
	if (Beam->DynamicMaterialSpotLight) Beam->DynamicMaterialSpotLight->SetScalarParameterValue("DMX Dimmer", Intensity / 4.5f); // / 4.5 to avoid a white sur-exposed effect
}