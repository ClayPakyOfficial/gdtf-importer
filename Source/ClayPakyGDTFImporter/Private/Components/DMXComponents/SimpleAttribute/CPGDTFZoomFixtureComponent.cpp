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


#include "Components/DMXComponents/SimpleAttribute/CPGDTFZoomFixtureComponent.h"

void UCPGDTFZoomFixtureComponent::InterpolateComponent(float DeltaSeconds) {

	if (this->AttachedBeams.Num() < 1) return;

	Super::InterpolateComponent(DeltaSeconds);
}

void UCPGDTFZoomFixtureComponent::SetValueNoInterp_BeamInternal(UCPGDTFBeamSceneComponent* Beam, float Angle) {

	if (Beam == nullptr) return;

	float HalfAngle = Angle / 2.0f; // / 2.0 because the angle in Unreal is between the center and the border of the beam

	if (Beam->DynamicMaterialBeam) Beam->DynamicMaterialBeam->SetScalarParameterValue("DMX Zoom", HalfAngle);
	if (Beam->DynamicMaterialLens) Beam->DynamicMaterialLens->SetScalarParameterValue("DMX Zoom", HalfAngle);
	if (Beam->DynamicMaterialPointLight) Beam->DynamicMaterialPointLight->SetScalarParameterValue("DMX Zoom", HalfAngle);
	if (Beam->DynamicMaterialSpotLight) Beam->DynamicMaterialSpotLight->SetScalarParameterValue("DMX Zoom", HalfAngle);
	if (Beam->SpotLight) {
		Beam->SpotLight->SetOuterConeAngle(HalfAngle);
		Beam->SpotLight->SetInnerConeAngle(HalfAngle * 0.85f); /// TODO \todo Find something on GDTF to be more accurate
	}
}