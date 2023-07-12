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
#include "Factories/Importers/Wheels/CPGDTFWheelImporter.h"

/**
 * Generates the HLSL code for the custom expression block of the beam's pipeline
 */
class CPGDTFBeamHlslGenerator
{
private:
	//Number of wheels per each WheelType
	int mWheelsNo[FCPGDTFWheelImporter::WheelType::WHEEL_TYPE_SIZE];
	//Attached shaper blades
	TArray<UCPGDTFShaperFixtureComponent*> mShapers;
	//If true we should include the iris inside the code pipeline
	bool mIrisEnabled;
public:

	/**
	 * Generates the HLSL code for the custom expression block of the beam's pipeline
	 * 
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023
	*/
	FString generateCode();

	/**
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * @date 09 january 2023 (Happy new year!)
	 *
	 * @param mWheelsNo Number of wheels per each WheelType
	 */
	void setWheels(int* wheelsNo);
	void setShapers(TArray<UCPGDTFShaperFixtureComponent*> shapers);
	void setIris(bool irisEnabled);


	CPGDTFBeamHlslGenerator();
	~CPGDTFBeamHlslGenerator();
};
