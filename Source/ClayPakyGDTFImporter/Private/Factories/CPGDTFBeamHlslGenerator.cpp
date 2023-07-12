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


#include "Factories/CPGDTFBeamHlslGenerator.h"

#define TEMPLATE_PLACEHOLDER "/*__HLSL_GENERATE_PIPELINE__*/"

#define BASE_TEMPLATE "																						\n\
struct FunctionsWrapper {																					\n\
	float sCurve(float f){																					\n\
		const float exp = -4;																				\n\
		f = saturate(f);																					\n\
		return 1 / (1 + pow((f / (1.125 - f)), exp));														\n\
	}																										\n\
																											\n\
	void rotateUV(float2 texCoor, float rads, out float x, out float y) {									\n\
		float c = cos(rads);																				\n\
		float s = sin(rads);																				\n\
		x = c * (texCoor.x - 0.5) + s * (texCoor.y - 0.5) + 0.5;											\n\
		y = s * (texCoor.x - 0.5) - c * (texCoor.y - 0.5) + 0.5;											\n\
	}																										\n\
																											\n\
	float4 sampleWheel(Texture2D txt, SamplerState txtSampler, float2 texCoor, float nSlots, float index){	\n\
		float scale = 1 / nSlots;																			\n\
		float offset = index / nSlots;																		\n\
		texCoor.x = texCoor.x * scale + offset;																\n\
		return txt.SampleLevel(txtSampler, texCoor, 0);														\n\
	}																										\n\
																											\n\
	float mapLineValue(float passed, float d, float frost, float frostMultiplier){							\n\
		float max = (0.05 * frost + 0.002) * frostMultiplier; /*0.04-0.002 Hardcoded blur value*/			\n\
		float clamp = d > max;																				\n\
		d = clamp + (1 - clamp) * (d / max);																\n\
																											\n\
		return sCurve((passed * 2 - 1) * (d / 2) + 0.5);													\n\
	}																										\n\
																											\n\
	float sampleIris(float2 texCoor, float irisVal, float frost) {											\n\
		/*irisVal = 1 - irisVal;*/																			\n\
		const float x = texCoor.x * 2 - 1, y = texCoor.y * 2 - 1;											\n\
		float dist = x * x + y * y;																			\n\
		/*dist = sqrt(dist);*/																				\n\
		float d = abs(dist - irisVal);																		\n\
		const float frostMulActivation = 0.15;																\n\
		float frostMulEnabled = irisVal >= frostMulActivation;												\n\
		float frostMul = frostMulEnabled + (1 - frostMulEnabled) * (irisVal / frostMulActivation);			\n\
		float passed = dist <= irisVal;																		\n\
		return mapLineValue(passed, d, frost, frostMul);													\n\
	}																										\n\
																											\n\
	float sampleBladeAB(float2 texCoor, float a, float b, float orientationRads, float frost){				\n\
		float x, y;																							\n\
		rotateUV(texCoor, orientationRads, x, y);															\n\
		float m = b - a;																					\n\
		float eq = (x * m) + a;																				\n\
		float d = abs(y - eq) /*/ sqrt(1 + m * m)*/;														\n\
		float passed = y > eq;																				\n\
		return mapLineValue(passed, d, frost, 1);															\n\
	}																										\n\
	float sampleBladeARot(float2 texCoor, float a, float rotRads, float orientationRads, float frost){		\n\
		float x, y;																							\n\
		rotateUV(texCoor, orientationRads, x, y);															\n\
		float m = rotRads * x;																				\n\
		float eq = m - (rotRads * 0.5) + a;																	\n\
		float d = abs(y - eq) /*/ sqrt(1 + m * m)*/;														\n\
		float passed = y > eq;																				\n\
		return mapLineValue(passed, d, frost, 1);															\n\
	}																										\n\
};																											\n\
																											\n\
float traversalDepth = FDepth - NDepth;																		\n\
uint numSteps = floor(traversalDepth / StepSize);															\n\
float3 posOffset = normalize(FSlice - NSlice) * StepSize;													\n\
																											\n\
float Adj = AdjOpp.x;																						\n\
float Opp = AdjOpp.y + ConeRadius;																			\n\
																											\n\
float3 cumul = 0;																							\n\
																											\n\
for (uint i = 0; i < numSteps; i++) {																		\n\
																											\n\
	/* Position & depth at rayHit */																		\n\
	float3 pos = NSlice + posOffset * i;																	\n\
	float depth = NDepth + StepSize * i;																	\n\
																											\n\
	float dist = length(pos);																				\n\
	float falloff = 1.0f - (dist / MaxDistance);															\n\
																											\n\
	/* Domain Transform */																					\n\
	pos.z = -pos.z;																							\n\
	pos /= float3(Opp * 2, Opp * 2, Adj);																	\n\
																											\n\
	float div = ConeRadius / Opp;																			\n\
	div = (pos.z * (1 - div)) + div;																		\n\
	pos.xy /= div;																							\n\
																											\n\
	/* Center domain */																						\n\
	pos.z -= 0.5;																							\n\
																											\n\
	/* Clip domain edges. */																				\n\
	if ((abs(pos.x) > 0.5) || (abs(pos.y) > 0.5) || (abs(pos.z) > 0.5)) continue;							\n\
																											\n\
	/* Soft clipping with scene depth. */																	\n\
	float dClip = saturate((ScDepth - depth) / SoftClipSize);												\n\
	if(dClip == 0) continue;																				\n\
																											\n\
	/* UVs from pos */																						\n\
	pos.xy = saturate(pos.xy + 0.5);																		\n\
																											\n\
	float3 outputSample = DMXColor; /*if too dark, set this to 1*/											\n\
	if(outputSample.x > 1) outputSample.x = 1;																\n\
	if(outputSample.y > 1) outputSample.y = 1;																\n\
	if(outputSample.z > 1) outputSample.z = 1;																\n\
	FunctionsWrapper _fncs;																					\n\
"TEMPLATE_PLACEHOLDER"																						\n\
	if(outputSample.x < 0) outputSample.x = 0;																\n\
	if(outputSample.y < 0) outputSample.y = 0;																\n\
	if(outputSample.z < 0) outputSample.z = 0;																\n\
																											\n\
	/* InvSqr falloff function */																			\n\
	float invsqr = 1.0f / (dist * dist);																	\n\
																											\n\
	/* Add to Result */																						\n\
	cumul += (1.f / numSteps) * outputSample * dClip * invsqr * falloff;									\n\
}																											\n\
return cumul;																								\n\
"

/**
 * Generates the HLSL code for the custom expression block of the beam's pipeline
 *
 * @author Luca Sorace - Clay Paky S.R.L.
 * @date 09 january 2023
*/
FString CPGDTFBeamHlslGenerator::generateCode() {
	FString outputCode = TEXT(BASE_TEMPLATE);
	FString placeholderCode = TEXT("");

	FString callText = TEXT("_fncs.{0}");
	FString powText = FString::Printf(TEXT("pow(%s, 0.66)"), *callText);

	FString textAdd = FString::Printf(TEXT("\toutputSample = outputSample - 1 + %s;\n"), *powText); //outputSample = outputSample - (1 - FUNCTION)
	/*
	THIS IS A DIRTY FIX! With gobos that have small dots/lines and dark colors, if you use the previous formula you'll end up with just black
	The reason is that dark primitive color - dark grey produced by blurring a gobo with small white spots = black
	I tried fixing this by remapping the sample before subtracting it (using the formula out=pow(in, n) with 0 < n < 1, good values are around 0.55-0.7) so it could gain more brightness, but the color was feeling too much different.
	So I decided to temp fix this by just multipling gobos instead of subtracting them. This is correct 'till the gobos are just greyscaled, but will lead to wrong color blending (compared to reality) if the gobo has colors
	*/
	FString textMul = FString::Printf(TEXT("\toutputSample *= %s;\n"), *powText);

	/**************************************
	*               GENERAL               *
	**************************************/

	FString frostVarName = CPGDTFRenderPipelineBuilder::getInputFrostName();

	if (mIrisEnabled) {
		FString irisName = CPGDTFRenderPipelineBuilder::getInputIrisName();
		placeholderCode = placeholderCode + FString::Format(*textAdd, { FString::Printf(TEXT("sampleIris(pos.xy, %s, %s)"), *irisName, *frostVarName) });
	}

	/*************************************
	*               WHEELS               *
	*************************************/

	bool foundGobo = false;

	for (int i = 0; i < FCPGDTFWheelImporter::WheelType::WHEEL_TYPE_SIZE; i++) {
		FCPGDTFWheelImporter::WheelType wheelType = (FCPGDTFWheelImporter::WheelType) i;
		switch (i) {
			case FCPGDTFWheelImporter::WheelType::Color: {
				for (int j = 0; j < this->mWheelsNo[i]; j++) {
					FString textureVarName = CPGDTFRenderPipelineBuilder::getInputTextureName(wheelType, false, j);
					FString numSlotVarName = CPGDTFRenderPipelineBuilder::getInputNumSlotsName(wheelType, j);
					FString indexVarName = CPGDTFRenderPipelineBuilder::getInputIndexName(wheelType, j);

					placeholderCode = placeholderCode + FString::Format(*textAdd, { FString::Printf(TEXT("sampleWheel(%s, %sSampler, pos.xy, %s, %s)"), *textureVarName, *textureVarName, *numSlotVarName, *indexVarName) });
				}
				break;
			}
			case FCPGDTFWheelImporter::WheelType::Gobo: {
				for (int j = 0; j < this->mWheelsNo[i]; j++) {
					FString textureVarName = CPGDTFRenderPipelineBuilder::getInputTextureName(wheelType, true, j);
					FString numSlotVarName = CPGDTFRenderPipelineBuilder::getInputNumSlotsName(wheelType, j);
					FString indexVarName = CPGDTFRenderPipelineBuilder::getInputIndexName(wheelType, j);
					//TODO: Add gobo rotation part
					placeholderCode = placeholderCode + FString::Format(*textMul, { FString::Printf(TEXT("sampleWheel(%s, %sSampler, pos.xy, %s, %s)"), *textureVarName, *textureVarName, *numSlotVarName, *indexVarName) });
					foundGobo = true;
				}
				break;
			}
			case FCPGDTFWheelImporter::WheelType::Animation: { //TODO Implement a real animation wheel
				for (int j = 0; j < this->mWheelsNo[i]; j++) {
					FString textureVarName = CPGDTFRenderPipelineBuilder::getInputTextureName(wheelType, true, j);
					FString indexVarName = CPGDTFRenderPipelineBuilder::getInputIndexName(wheelType, j);
					//placeholderCode = placeholderCode + FString::Printf(textMul, FString::Printf(TEXT("sampleWheel(%s, %sSampler, pos.xy, %s, %s)"), textureVarName, textureVarName, numSlotVarName, indexVarName));
				}
				break;
			}
			case FCPGDTFWheelImporter::WheelType::Prism:
			case FCPGDTFWheelImporter::WheelType::Effects:
				//TODO: Implement both
				break;
		}
	}

	if (!foundGobo) //Filter the beam shape anyway if there's no gobo present
		placeholderCode = placeholderCode + TEXT("outputSample *= pow(TXTpGobo.SampleLevel(TXTpGoboSampler, pos.xy, 0).x, 0.66);");
	
	/*************************************
	*               SHAPER               *
	*************************************/

	for (int i = 0; i < mShapers.Num(); i++) {
		FString orientationVarName = CPGDTFRenderPipelineBuilder::getInputBladeOrientationName(mShapers[i]->getOrientation());
		FString bRotVarName = CPGDTFRenderPipelineBuilder::getInputBladeABRotName(false, mShapers[i]->getOrientation());
		FString aVarName = CPGDTFRenderPipelineBuilder::getInputBladeABRotName(true, mShapers[i]->getOrientation());

		if (mShapers[i]->isInAbMode()) { //a+b mode
			placeholderCode = placeholderCode + FString::Format(*textAdd, { FString::Printf(TEXT("sampleBladeAB(pos.xy, %s, %s, %s, %s)"), *aVarName, *bRotVarName, *orientationVarName, *frostVarName) });
		} else { //a + rot mode
			placeholderCode = placeholderCode + FString::Format(*textAdd, { FString::Printf(TEXT("sampleBladeARot(pos.xy, %s, %s, %s, %s)"), *aVarName, *bRotVarName, *orientationVarName, *frostVarName) });
		}
	}

	FString from = TEXT(TEMPLATE_PLACEHOLDER); //idk why I can't use it directly
	outputCode = outputCode.Replace(*from, *placeholderCode, ESearchCase::CaseSensitive);

	TArray<FString> lines;
	outputCode.ParseIntoArray(lines, TEXT("\n"), false);
	outputCode.Empty();
	for (int i = 0; i < lines.Num(); i++)
		//outputCode.Append(lines[i].TrimEnd() + TEXT("\n"));
		outputCode.Append(lines[i].TrimStartAndEnd());

	return outputCode;
}

void CPGDTFBeamHlslGenerator::setWheels(int* wheelsNo) {
	for (int i = 0; i < FCPGDTFWheelImporter::WheelType::WHEEL_TYPE_SIZE; i++)
		this->mWheelsNo[i] = wheelsNo[i];
}
void CPGDTFBeamHlslGenerator::setShapers(TArray<UCPGDTFShaperFixtureComponent*> shapers) {
	this->mShapers = shapers;
}
void CPGDTFBeamHlslGenerator::setIris(bool irisEnabled) {
	mIrisEnabled = irisEnabled;
}

CPGDTFBeamHlslGenerator::CPGDTFBeamHlslGenerator(){}
CPGDTFBeamHlslGenerator::~CPGDTFBeamHlslGenerator(){}

#undef TEMPLATE_PLACEHOLDER
#undef BASE_TEMPLATE