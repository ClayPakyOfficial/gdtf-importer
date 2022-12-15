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

#include "CPGDTFInterpolation.h"
#include "Kismet/KismetMathLibrary.h"

FChannelInterpolation::FChannelInterpolation() {
	
	IsUpdating = false;
	InterpolationScale = 1.0f;
	RangeSize = 1.0f;
	TargetValue = 0.0f;
	CurrentValue = 0.0f;
	Direction = 1.0f;
	TotalTravel = 0.0f;
	ToTravel = 0.0f;
	PreviousTravelPercent = 0.0f;
	PreviousStep = 0.0f;
	CurrentSpeed = 0.0f;
	AccelerationThreshold = 0.5f;
	
	bFirstValueWasSet = false;
}

bool FChannelInterpolation::IsTargetValid(float Value, float SkipThreshold) const {
	return FMath::Abs(TargetValue - Value) >= SkipThreshold;
}

void FChannelInterpolation::SetValueNoInterp(float NewValue) {
	
	CurrentValue = NewValue;
	TargetValue = NewValue;
}

bool FChannelInterpolation::IsInterpolationDone() const {
	
	if (this->RangeSize == 1.0f) return ToTravel < 0.005f;
	else return ToTravel < (this->RangeSize / 1000.0f); // If less than 0.1% remaining (very low value to avoid a teleportation at the end of large ranges like pan)
}

void FChannelInterpolation::EndInterpolation() {
	
	CurrentValue = TargetValue;
	TotalTravel = 0.0f;
	ToTravel = 0.0f;
	PreviousTravelPercent = 0.0f;
	PreviousStep = 0.0f;
	AccelerationThreshold = 0.5f;
	IsUpdating = false;
}

void FChannelInterpolation::StartTravel(float NewTarget) {
	
	IsUpdating = true;
	float NewDirection = (TargetValue < NewTarget) ? 1.0f : -1.0f;
	if (CurrentSpeed == 0 || Direction != NewDirection) CurrentSpeed = SPEED_MINIMUM;//SPEED_ACCELERATION_MID * InterpolationScale;
	Direction = NewDirection;
	float Travel = FMath::Abs(TargetValue - NewTarget);
	TotalTravel = Travel;
	ToTravel = Travel;
	TargetValue = NewTarget;
}

void FChannelInterpolation::UpdateTravel(float NewTarget) {
	
	float TravelDelta = (TargetValue - NewTarget) * Direction * -1.0f;
	if (CurrentValue * Direction < NewTarget * Direction) {
		
		// front
		TotalTravel = FMath::Max(TotalTravel + TravelDelta, 0.0f);
		ToTravel = FMath::Max(ToTravel + TravelDelta, 0.0f);
		TargetValue = NewTarget;
		
		// shift accel/decel threshold
		float CurrentTravelPercent = UKismetMathLibrary::SafeDivide(TotalTravel - ToTravel, TotalTravel);
		//CurrentTravelPercent *= 1.1f;
		AccelerationThreshold = FMath::Clamp(CurrentTravelPercent, 0.5f, 0.9f);
	} else {
		
		// back
		TotalTravel = FMath::Abs(NewTarget - CurrentValue);
		ToTravel = TotalTravel;
		Direction *= -1.0f;
		TargetValue = NewTarget;
		CurrentSpeed = SPEED_MINIMUM;
		AccelerationThreshold = 0.5f;
	}
}

/// TODO \todo instead of using the derivative of the SmoothStep, use a sine wave
void FChannelInterpolation::Travel(float DeltaSeconds) {
	
	TotalTravel = FMath::Max(TotalTravel, 0.0f);
	ToTravel = FMath::Max(ToTravel, 0.0f);
	
	float CurrentTravelPercent = UKismetMathLibrary::SafeDivide(TotalTravel - ToTravel, TotalTravel);
	CurrentTravelPercent = FMath::Clamp(CurrentTravelPercent, 0.0f, 1.0f);
	
	float CurrentStep = FMath::SmoothStep(0.0f, 1.0f, CurrentTravelPercent);
	float Derivative = FMath::Clamp(UKismetMathLibrary::SafeDivide(CurrentStep - PreviousStep, CurrentTravelPercent - PreviousTravelPercent), 0.0f, 1.0f);
	float TravelAlpha = FMath::Clamp(TotalTravel / RangeSize, 0.0f, 1.0f);
	float Acceleration = FMath::Lerp(SPEED_ACCELERATION_MIN * InterpolationScale, SPEED_ACCELERATION_MID * InterpolationScale, TravelAlpha);
	float SpeedInc = FMath::Lerp(Acceleration, SPEED_ACCELERATION_MAX * InterpolationScale, Derivative);
	
	if (CurrentTravelPercent < AccelerationThreshold) CurrentSpeed = FMath::Max(CurrentSpeed + SpeedInc, 0.0f); // accelerate
	else CurrentSpeed = FMath::Max(CurrentSpeed - SpeedInc, SPEED_MINIMUM * InterpolationScale); // decelerate
	
	// save
	PreviousStep = CurrentStep;
	PreviousTravelPercent = CurrentTravelPercent;
	
	// increment 
	CurrentValue += (CurrentSpeed * DeltaSeconds * Direction);
	ToTravel -= FMath::Abs(CurrentSpeed * DeltaSeconds);
}

void FChannelInterpolation::Push(float NewTarget) {
	
	if (IsUpdating) UpdateTravel(NewTarget);
	else StartTravel(NewTarget);
}