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

#define SPEED_MINIMUM 5.0F //50.0f Initial values
#define SPEED_ACCELERATION_MIN 1.0F // 15
#define SPEED_ACCELERATION_MID 3.4F // 20
#define SPEED_ACCELERATION_MAX 10.0F // 30

/// Interpolation that provides a damping effect and support direction changes
struct CLAYPAKYGDTFIMPORTER_API FChannelInterpolation {
	
	bool IsUpdating;
	float InterpolationScale;

	/// Current position in interpolation
	float CurrentValue;

	/// End position of interpolation
	float TargetValue;
	
	/// Lenght remaining to travel until the TargetValue
	float ToTravel;

	/// Size between min and max value
	float RangeSize;
	
	/// -1 or 1
	float Direction;

	/// Total interpolation lenght
	float TotalTravel;

	/// Percent of the travel at previous travel step
	float PreviousTravelPercent;
	float PreviousStep;
	
	float CurrentSpeed;
	float AccelerationThreshold;
	
	bool bFirstValueWasSet;

	/// Create object and set default values
	FChannelInterpolation();

	/// Check if the gap between Value and current interpolated value is bigger than SkipThreshold
	bool IsTargetValid(float Value, float SkipThreshold) const;

	/// Set current value without any interpolation
	void SetValueNoInterp(float NewValue);

	bool IsInterpolationDone() const;

	void EndInterpolation();

	/// Start interpolation
	void StartTravel(float NewTarget);

	/// Change target value without stopping interpolation
	void UpdateTravel(float NewTarget);


	void Travel(float DeltaSeconds);

	/// Set new target value in interpolation system
	void Push(float NewTarget);

};



