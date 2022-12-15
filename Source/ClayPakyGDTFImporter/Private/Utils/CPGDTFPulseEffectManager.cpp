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

#include "Utils/CPGDTFPulseEffectManager.h"

/**
 * Set the properties of the pulse effect
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 21 july 2022
 *
 * @param EffectType Type of the effect
 * @param Period in seconds
 * @param _DutyCycle Percentage (value between [0;1])
 * @param TimeOffset Percentage (value between [0;1])
 */
void FPulseEffectManager::SetSettings(EPulseEffectType EffectType, double Period, double _DutyCycle, double TimeOffset) {

	this->PulseEffect = EffectType;
	this->PeriodTime = Period;
	this->DutyCyclePercent = FMath::Max(0, FMath::Min(1, _DutyCycle));
	this->TimeOffsetPercent = FMath::Max(0, FMath::Min(1, TimeOffset));
	this->DutyCycle = Period * this->DutyCyclePercent;
	this->CurrentTime = Period * this->TimeOffsetPercent;
}

/**
 * Change the Effect period
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 22 july 2022
 * 
 * @param Period in seconds
*/
void FPulseEffectManager::ChangePeriod(double Period) {

	double DeltaTimeOffset = (Period * this->TimeOffsetPercent) - (this->PeriodTime * this->TimeOffsetPercent);
	this->CurrentTime += DeltaTimeOffset;
	this->DutyCycle = Period * this->DutyCyclePercent;
	this->PeriodTime = Period;
}

/**
 * Calc the value for a given time without updating the internal CurrentTime.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 21 july 2022
 *
 * @param Time
 * @return Result
 */
double FPulseEffectManager::CalcValue(double Time) {

	while (Time >= this->PeriodTime) {
		Time -= this->PeriodTime;
	}
	if (Time > this->DutyCycle) return 0;

	// Reference: https://en.wikipedia.org/wiki/Waveform#Examples

	double Result = (UE_DOUBLE_TWO_PI * Time) / this->DutyCycle;

	switch (this->PulseEffect) {

	case EPulseEffectType::Pulse:
		Result = FMath::Asin(FMath::Sin(Result));
		break;

	case EPulseEffectType::PulseOpen:
		Result = FMath::Atan(FMath::Tan(Result));
		break;

	case EPulseEffectType::PulseClose:
		Result = - FMath::Atan(FMath::Tan(Result));
		break;

	default:
		break;
	}

	return UE_DOUBLE_INV_PI * Result + 0.5;
}

double FPulseEffectManager::InterpolatePulse(double DeltaSeconds) {
	this->CurrentTime += DeltaSeconds;
	while (this->CurrentTime >= this->PeriodTime) {
		this->CurrentTime -= this->PeriodTime;
	}
	return this->CalcValue(this->CurrentTime);
}