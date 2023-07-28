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

#include "Utils/CPGDTFPulseEffectManager.h"

void FPulseEffectManager::SetSettings(EPulseEffectType EffectType, float Period, float _DutyCycle, float TimeOffset) {
	this->PulseEffect = EffectType;
	this->PeriodTime = Period;
	this->DutyCyclePercent = FMath::Max(0, FMath::Min(1, _DutyCycle));
	this->TimeOffsetPercent = FMath::Max(0, FMath::Min(1, TimeOffset));
	this->DutyCycle = Period * DutyCyclePercent;
	this->CurrentTime = fmod(Period * TimeOffsetPercent, PeriodTime);
}


void FPulseEffectManager::ChangePeriod(float Period) {
	float DeltaTimeOffset = (Period * TimeOffsetPercent) - (PeriodTime * TimeOffsetPercent);
	CurrentTime = fmod(CurrentTime + DeltaTimeOffset, PeriodTime);
	DutyCycle = Period * DutyCyclePercent;
	PeriodTime = Period;
}


float FPulseEffectManager::__CalcValue_internal(float Time) {
	if (Time > DutyCycle) return 0;

	// Reference: https://en.wikipedia.org/wiki/Waveform#Examples

	//Start from zero
	if (this->PulseEffect == EPulseEffectType::Pulse) Time -= DutyCycle / 4; //Pulse effect has to fully rise/fall twice as fast
	else Time -= DutyCycle / 2;
	//remap Time in a [0, 1] range * pi
	float x = (UE_PI * Time) / DutyCycle;
	float y;
	//Actually calc the value
	switch (this->PulseEffect) {
		case EPulseEffectType::Pulse:
			y = FMath::Asin(FMath::Sin(2 * x)); //Pulse effect has to fully rise/fall twice as fast
			break;
		case EPulseEffectType::PulseOpen:
			y = FMath::Atan(FMath::Tan(x));
			break;
		case EPulseEffectType::PulseClose:
			y = - FMath::Atan(FMath::Tan(x));
			break;
		default:
			break;
	}
	//Convert the value in a [0, 1] range
	float res = UE_INV_PI * y + 0.5;
	return res;
}

float FPulseEffectManager::InterpolatePulse(float DeltaSeconds) {
	CurrentTime += DeltaSeconds;
	loopedBack = CurrentTime > PeriodTime;
	CurrentTime = fmod(CurrentTime, PeriodTime);
	return __CalcValue_internal(CurrentTime);
}