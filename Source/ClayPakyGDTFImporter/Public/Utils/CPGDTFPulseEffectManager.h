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

/// Different types of pulses
UENUM(BlueprintType)
enum class EPulseEffectType : uint8 {
	Pulse,
	PulseOpen,
	PulseClose
};

/**
 * Manage Pulse effects <br>
 * Implementation reference: GDTF Spec, Annex F, DIN SPEC 15800:2022-02 <br>
 * Mathematic reference: https://en.wikipedia.org/wiki/Waveform#Examples
 */
class CLAYPAKYGDTFIMPORTER_API FPulseEffectManager {

protected:

	EPulseEffectType PulseEffect;

	double CurrentTime;
	double PeriodTime;
	double DutyCycle;

	//Used to calc the new values in ChangePeriod

	/**
	 * This defines the fraction of one period in which the pulse is on.
	 */
	double DutyCyclePercent;

	/**
	 * This defines the offset of the end of the pulse from the start as percentage of the total period.
	 */
	double TimeOffsetPercent;

public:
	FPulseEffectManager() { this->Reset(); }

	void Reset() { this->SetSettings(EPulseEffectType::Pulse); }

	/**
	 * Set the properties of the pulse effect
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 21 july 2022
	 * 
	 * @param EffectType Type of the effect
	 * @param Period in seconds
	 * @param _DutyCycle Percentage (value between [0;1])
	 * @param TimeOffset Percentage (value between [0;1])
	*/
	void SetSettings(EPulseEffectType EffectType, double Period = 1, double _DutyCycle = 1, double TimeOffset = 1);

	/**
	 * Change the Effect period
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 22 july 2022
	 * 
	 * @param Period in seconds
	*/
	void ChangePeriod(double Period = 1);

	double GetCurrentTime() { return this->CurrentTime; }
	double GetPeriodTime()  { return this->PeriodTime; }

	/**
	 * Calc the value for a given time
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 21 july 2022
	 * 
	 * @param Time 
	 * @return Result
	*/
	double CalcValue(double Time);

	double InterpolatePulse(double DeltaSeconds);
};