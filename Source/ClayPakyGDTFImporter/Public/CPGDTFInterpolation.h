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

#include "HAL/CriticalSection.h"
#include "Misc/AssertionMacros.h"
#include "ClayPakyGDTFImporterLog.h"
#include "CPGDTFInterpolation.generated.h"

#define MIN_MOVEMENT_TIME 0.7f
#define MIN_MOVEMENT_REALFADE_RATIO 0.55

/*
  _   _  ____ _______ _____ _____ ______
 | \ | |/ __ \__   __|_   _/ ____|  ____|
 |  \| | |  | | | |    | || |    | |__
 | . ` | |  | | | |    | || |    |  __|
 | |\  | |__| | | |   _| || |____| |____
 |_| \_|\____/  |_|  |_____\_____|______|


Having trouble understanding this class? This is widely documented in the Docs/Interpolation.md file.
Take it a look before changing anything!
*/


/// Interpolation that provides a damping effect and support direction changes
USTRUCT(BlueprintType)
struct CLAYPAKYGDTFIMPORTER_API FChannelInterpolation {

	GENERATED_BODY()
	
private:
	enum Direction {
		FORWARD = 1,
		STOP = 0,
		BACKWARD = -1
	};

	// Struct holding the current acceleration and speed status
	struct InterpolationStatus {
		//The current Acceleration 
		float currentAcceleration;
		//Current acceleration with the sign normalized respect our direction. If this is positive, it ALWAYS means we're accelerating, if it's negative we're decelerating
		float currentAccelerationNoDirection;
		//Absolute value of the current acceleration
		float currentAccelerationAbs;
		//The current speed normalized between 0 and 1
		float currentSpeed;
		//Absolute value of the current speed
		float currentSpeedAbs;
		//If IsAccelerationOverriden() we MUST use this value instead of currentAcceleration (except in getStopDistance()) 
		float accelerationOverride;
		//If true we're overriding the acceleration and you MUST use accelerationOverride instead of currentAcceleration
		bool _isAccelerationOverriden;
		//Current direction of the interpolation
		Direction currentDirection;

		//sets the normalized speed of the interpolation
		inline void setCurrentSpeed(float speed) {
			currentSpeed = speed;
			currentSpeedAbs = FMath::Abs(speed);
		}
		//sets the current acceleration of the interpolation
		inline void setCurrentAcceleration(float _acceleration) {
			currentAcceleration = _acceleration;
			currentAccelerationNoDirection = currentAcceleration * currentDirection;
			currentAccelerationAbs = FMath::Abs(_acceleration);
			_isAccelerationOverriden = false;
		}
		//Forcefully sets the acceleration and enables the acceleration override mode
		inline void setAccelerationOverride(float newAccel) {
			accelerationOverride = newAccel;
			_isAccelerationOverriden = true;
		}

		//check if we're overriding the acceleration. If yes, you should use accelerationOverride instead of currentAcceleration
		FORCEINLINE bool IsAccelerationOverriden() {
			return _isAccelerationOverriden;
		}
		/*Check if the interpolation is currently stopped. NOTE: This may differ from IsMoving() since we
		check if our speed is gonna stop in the next frame (instead of checking if we actually have speed) and also if our acceleration is equals to 0*/
		inline bool IsStopped(float DeltaSeconds, float acceleration) {
			return currentSpeedAbs <= acceleration * DeltaSeconds && currentAcceleration == 0;
		}
		//Checks whenever the interpolation is running at the maximum of the speed
		inline bool IsAtMaxSpeed() {
			return currentSpeedAbs == 1 && currentAcceleration == 0;
		}
		//Checks if the interpolation is moving. NOTE: This may differ from IsStopped() since we ONLY check if our speed is not 0
		inline bool IsMoving() {
			return currentSpeedAbs > 0;
		}
		//Checks if we're gaining speed
		inline bool IsAccelerating() {
			return currentAccelerationNoDirection > 0 || (IsAccelerationOverriden() && (accelerationOverride * currentDirection) > 0);
		}
		//Check if we're loosing speed
		inline bool IsDecelerating() {
			return currentAccelerationNoDirection < 0 || (IsAccelerationOverriden() && (accelerationOverride * currentDirection) < 0);
		}

		//starts acelerating the interpolation
		inline void start(float acceleration) {
			if (currentAccelerationNoDirection <= 0 && currentSpeedAbs < 1)
				setCurrentAcceleration(acceleration * currentDirection);
		}
		//starts decelerating the interpolation
		inline void stop(float acceleration) {
			if (currentAccelerationNoDirection >= 0 && currentSpeedAbs > 0)
				setCurrentAcceleration(-acceleration * currentDirection);
		}
	};
public:
	/// Last value assumed from the "physical" parameter controlled by this interpolation
	float lastValue;
	//If false we always jump to the target value
	bool bInterpolationEnabled;
	//Enables the speed cap system, to slow down small movements
	bool bSpeedCapEnabled;

private:
	//max physical speed expressed in value per second
	float maxPhysicalSpeed;
	//acceleration on the interpolation. THIS IS DIFFERENT FROM CURRENT ACCELERATION. Expressed in normalized speed per second. It's the value we'll add each second to currentSpeed (that's normalized between 0 and 1)
	float acceleration;
	// Time in seconds to get from speed 0 to 1 (maxPhysicalSpeed)
	float timeToFullyAccelerate;
	// speedCap for the currentSpeed. If < 0 it's disabled
	float maxNormalizedSpeed = -1;

	// Time in seconds to get from minValue to maxValue. Used to cap the currentSpeed if we're moving too fast. Use setFadeAndAcceleration() to set that. Using setAcceleration() or setMaxSpeed() will invalidate that
	float realFade = -1;

	// Current position in the interpolation
	float CurrentValue;
	// End position of the interpolation
	float TargetValue;
	// Target value in the previous tick
	float oldTargetValue;
	// Target value with the float precision truncated after N positions
	int targetValueNoPrecision;

	//Current acceleration and speed status
	InterpolationStatus currentStatus;
	//Lock struct
	FCriticalSection *criticalSection = nullptr;

	//previous movement status, so if in the previous tick we were acelerating/at max speed or decelerating/stopped
	bool oldMovementStatus;
	//If true it means that the interpolation object was newly created and no TargetValue has been set yet. Next time we set a TargetValue we have to snap there instead of fading
	bool bFirstValueNotSet = true;

	//truncates the precision of a float and returns it as integer
	FORCEINLINE static int removePrecision(float n, float precision = 100) {
		return FMath::RoundToInt(n * precision);
	}

	inline void aquireLock(){
		criticalSection->Lock();
	}
	inline void releaseLock(){
		criticalSection->Unlock();
	}

	//starts acelerating the interpolation
	FORCEINLINE void start() {
		currentStatus.start(acceleration);
	}
	//starts decelerating the interpolation
	FORCEINLINE void stop() {
		currentStatus.stop(acceleration);
	}

	//sets the normalized speed of the interpolation
	FORCEINLINE void setCurrentSpeed(float speed) {
		currentStatus.setCurrentSpeed(speed);
	}
	//sets the current acceleration of the interpolation
	FORCEINLINE void setCurrentAcceleration(float _acceleration) {
		currentStatus.setCurrentAcceleration(_acceleration);
	}

	//Checks if the target value has changed since the last tick
	inline bool isTargetValueChanged() {
		return TargetValue != oldTargetValue;
	}

	//Calculates the amount of physical movement the interpolation is gonna do if we start deceleraing in this tick till we reach the stop position
	inline float getStopDistance() {
		// (timeToFullyAccelerate/2) * currentSpeed^2
		// We're intentionally using the normal acceleration instead of the overriden, otherwise we will break the update override conditions!
		float timeToDecelerate = timeToFullyAccelerate * currentStatus.currentSpeedAbs; //timeToFullyAccelerate : 1 = x : currentSpeed
		float distanceNormalized = timeToDecelerate * currentStatus.currentSpeed / 2; //calcs the integral under our deceleration function. Since it's linear, it's just the area of a triangle with currentSpeed as height and timeToDecelerate as base
		return distanceNormalized * maxPhysicalSpeed; //Transforms the area into a physical value
	}

	//calculates the movement between this and the last tick and updates the speed
	float calcNextMovement(float DeltaSeconds);
	//calculates the movement using the current speed, the previous speed and the delta seconds
	float __calcNextMovement_internal(float previousSpeed, float deltaSeconds);
	//Using the difference between stopPosition and TargetValue together with the current speed, it calculates how much time it passed since the moment we had to start decelerating to meet and not surpass the TargetValue
	float getCorrectStopTimeDelta(const float distanceDifference, const float distanceDifferenceAbs, const float previousCurrentAcceleration, bool wasAccelerating);
	//When the tick where we start decelerating comes too late, we have to calculate when we needed to start decelerating, calculate the movement until there, and then update the DeltaSeconds from that moment to the current tick 
	bool compensateLateCall(float* DeltaSecond, float stopDistance, InterpolationStatus& prevStatus);
	//When the TargetValue changes and comes closer to the CurrentValue (in the direction we're moving) we may need to decelerate faster than usual to meet it, instead of suprassing it and then coming back. This function calculates the new acceleration
	float overrideDeceleration(float prevStopDistance, InterpolationStatus& prevStatus);
	//Time needed to entirely move through a range, starting from a stopped state, using the normal acceleration and then fully stopping
	float getTimeToMoveThruRange(float range);
	//Calculates and sets the maxNormalizedSpeed value we need to run through the range in the specified amount of time
	void setMaxNormalizedSpeedFromTimeAndRange(float time, float range);
public:

	/// Create object and set default values
	FChannelInterpolation(float Default = 0.0f);
	//Destroys the interpolation object
	void destroyInterpolation();

	/*Check if the interpolation is currently stopped. NOTE: This may differ from IsMoving() since we
	check if our speed is gonna stop in the next frame (instead of checking if we have speed) and also if our acceleration is equals to 0*/
	inline bool IsStopped(float DeltaSeconds) {
		const float accel = currentStatus.IsAccelerationOverriden() ? FMath::Abs(currentStatus.accelerationOverride) : acceleration;
		return currentStatus.IsStopped(DeltaSeconds, accel);
	}
	//Checks whenever the interpolation is running at the maximum of the speed
	FORCEINLINE bool IsAtMaxSpeed() {
		return currentStatus.IsAtMaxSpeed();
	}
	//Checks if the interpolation is moving. NOTE: This may differ from IsStopped() since we ONLY check if our speed is not 0
	FORCEINLINE bool IsMoving() {
		return currentStatus.IsMoving();
	}
	//Checks if we're gaining speed
	FORCEINLINE bool IsAccelerating() {
		return currentStatus.IsAccelerating();
	}
	//Check if we're loosing speed
	FORCEINLINE bool IsDecelerating() {
		return currentStatus.IsDecelerating();
	}
	//Checks if CurrentValue has surpassed TargetValue in the direction we're moving
	inline bool hasSurpassedTargetValue() {
		int cv = removePrecision(CurrentValue);
		return currentStatus.currentDirection == FORWARD ? cv > targetValueNoPrecision : cv < targetValueNoPrecision;
	}
	//Check if the interpolation is still going
	inline bool IsUpdating() {
		return CurrentValue != TargetValue;
	}
	//Checks if the interpolation is done by checking if the difference between the current and the target value is less then the space we're gonna move during this tick
	inline bool IsInterpolationDone__ValueDiff(float currentValue, float movement) {
		float movNoDir = movement * currentStatus.currentDirection;
		return FMath::Abs(currentValue - TargetValue) <= movNoDir || FMath::Abs(currentValue + movement - TargetValue) <= movNoDir;
	}
	//Checks if the interpolation is done by checking if our speed is less than the speed we're gonna lose during this tick
	inline bool IsInterpolationDone__CheckSpeed(float DeltaSeconds) {
		const float accel = currentStatus.IsAccelerationOverriden() ? FMath::Abs(currentStatus.accelerationOverride) : currentStatus.currentAccelerationAbs;
		return removePrecision(currentStatus.currentSpeedAbs) <= removePrecision(accel * DeltaSeconds);
	}
	//Check if the interpolation is done using multiple conditions
	inline bool IsInterpolationDone(float DeltaSeconds, float movement) {
		const bool valueDiff = IsInterpolationDone__ValueDiff(CurrentValue, movement);
		const bool checkSpeed = IsInterpolationDone__CheckSpeed(DeltaSeconds);

		const float accel = currentStatus.IsAccelerationOverriden() ? FMath::Abs(currentStatus.accelerationOverride) : currentStatus.currentAccelerationAbs;
		return valueDiff && checkSpeed;
	}
	/// Check if the gap between Value and current interpolated value is bigger than SkipThreshold
	inline bool IsTargetValid(float Value, float SkipThreshold) {
		return FMath::Abs(TargetValue - Value) >= SkipThreshold;
	}

	/// Set current value without any interpolation
	inline void SetValueNoInterp(float NewValue) {
		aquireLock();
		__setTargetValue_noLocks(NewValue);
		__EndInterpolation_noLocks(true);
		releaseLock();
	}

	/**
	 * Sets the acceleration and the maxPhysicalSpeed values using the RealAcceleration and RealFade parameters present in the GDTF file
	 * @author Luca Sorace - Clay Paky S.R.L.
	 * 
	 * @param RealAcceleration Time in seconds to get from 0 to maxPhysicalSpeed
	 * @param RealFade Time in seconds to get from minValue to maxValue, including the acceleration and deceleration phase
	 * @param range Difference between the min and the max physical value that the interpolation can reach
	 */
	void setFadeAndAcceleration(float RealAcceleration, float RealFade, float range);
	/**
	 * Sets the acceleration and the maxPhysicalSpeed values using the RealAcceleration and RealFade parameters present in the GDTF file
	 * @author Luca Sorace - Clay Paky S.R.L.
	 *
	 * @param RealAcceleration Time in seconds to get from 0 to maxPhysicalSpeed
	 * @param RealFade Time in seconds to get from minValue to maxValue, including the acceleration and deceleration phase
	 * @param min Min physical value that the interpolation can reach
	 * @param max Max physical value that the interpolation can reach
	 */
	FORCEINLINE void setFadeAndAcceleration(float RealAcceleration, float RealFade, float min, float max) {
		setFadeAndAcceleration(RealAcceleration, RealFade, FMath::Abs(max - min));
	}

	//Sets the max speed, expressed in value per second
	FORCEINLINE void setMaxSpeed(float _maxSpeed) {
		aquireLock();
		_maxSpeed = FMath::Abs(_maxSpeed);
		this->maxPhysicalSpeed = _maxSpeed;
		this->realFade = -1;
		releaseLock();
	}
	//Gets the max speed, expressed in value per second
	FORCEINLINE float getMaxSpeed() {
		return maxPhysicalSpeed;
	}
	//Sets the acceleration, expressed in normalized speed per second
	inline void setAcceleration(float _acceleration) {
		aquireLock();
		_acceleration = FMath::Abs(_acceleration);
		this->timeToFullyAccelerate = 1 / _acceleration;
		this->acceleration = _acceleration;
		this->realFade = -1;
		releaseLock();
	}
	//Gets the acceleration, expressed in normalized speed per second
	FORCEINLINE float getAcceleration() {
		return acceleration;
	}
	//Sets the target value. If it hasn't been set yet, we snap directly to it. If we're moving by just a little bit, we slow down our max normalized speed to get a smooth fade
	void __setTargetValue_noLocks(float value);
	inline void setTargetValue(float value) {
		aquireLock();
		__setTargetValue_noLocks(value);
		releaseLock();
	}
	//Obtains the target value
	FORCEINLINE float getTargetValue() {
		return TargetValue;
	}
	//Obtains the current value
	FORCEINLINE float getCurrentValue() {
		return CurrentValue;
	}

private:

	//USE THIS INTERNALLY, SINCE IT DOESN'T AQUIRE ANY ADDICTIONAL LOCKS. Ends the interpolation. If snapToTargetValue is true, we jump directly to the target value, otherwise we stop with the current target value
	void __EndInterpolation_noLocks(bool snapToTargetValue = true);

	//Advance the interpolation of one tick
	void __Update_noLocks(float DeltaSeconds);

public:
	//Ends the interpolation. If snapToTargetValue is true, we jump directly to the target value, otherwise we stop with the current target value
	inline void EndInterpolation(bool snapToTargetValue = true) {
		aquireLock();
		__EndInterpolation_noLocks(snapToTargetValue);
		releaseLock();
	}

	//Advance the interpolation of one tick
	inline void Update(float DeltaSeconds) {
		aquireLock();
		__Update_noLocks(DeltaSeconds);
		releaseLock();
	}
};