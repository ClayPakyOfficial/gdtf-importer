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

	float CurrentTime;
	float PeriodTime;
	float DutyCycle;

	//Used to calc the new values in ChangePeriod

	/**
	 * This defines the fraction of one period in which the pulse is on.
	 */
	float DutyCyclePercent;

	/**
	 * This defines the offset of the end of the pulse from the start as percentage of the total period.
	 */
	float TimeOffsetPercent;

	//True if the last interpolation restarted the duty cycle
	bool loopedBack;

	float __CalcValue_internal(float Time);

public:
	FPulseEffectManager() { this->Reset(); }

	void Reset() { this->SetSettings(EPulseEffectType::Pulse); }

	//Returns true if the last interpolation restarted the duty cycle
	FORCEINLINE bool hasLoopedBack() { return loopedBack; }

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
	void SetSettings(EPulseEffectType EffectType, float Period = 1, float _DutyCycle = 1, float TimeOffset = 1);

	/**
	 * Change the Effect period
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 22 july 2022
	 * 
	 * @param Period in seconds
	*/
	void ChangePeriod(float Period = 1);

	float GetCurrentTime() { return this->CurrentTime; }
	float GetPeriodTime()  { return this->PeriodTime; }

	/**
	 * Calc the value for a given time
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * @date 21 july 2022
	 * 
	 * @param Time 
	 * @return Result
	*/
	FORCEINLINE float CalcValue(float Time) { return __CalcValue_internal(fmod(Time, PeriodTime)); }

	/**
	 * Updates the pulse value with a specified amount of time.
	 * @author Dorian Gardes, Luca Sorace - Clay Paky S.R.L.
	 * @date 21 july 2022
	 *
	 * @param Time
	 * @return Result expressed in the [0, 1] range
	 */
	float InterpolatePulse(float DeltaSeconds);
};