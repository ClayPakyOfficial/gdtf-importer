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

#include "CPGDTFInterpolation.h"
#include "Kismet/KismetMathLibrary.h"

#define MAX_FAST_DECELERATION_RATIO 1.5f //Multiplier of the acceleration to get the max Deceleration speed, in case we have to slow down faster then the normal acceleration

FChannelInterpolation::FChannelInterpolation(float Default) {
	if(criticalSection == nullptr)
		criticalSection = new FCriticalSection();

	TargetValue = Default;
	__EndInterpolation_noLocks(true);

	bFirstValueNotSet = true;
	bInterpolationEnabled = true;
	bSpeedCapEnabled = true;
}

void FChannelInterpolation::destroyInterpolation() {
	//This is not an object destructor, otherwise it would be called continuosly causing unreal engine to just freeze
	if (criticalSection != nullptr) {
		delete criticalSection;
		criticalSection = nullptr;
	}
}

void FChannelInterpolation::__EndInterpolation_noLocks(bool snapToTargetValue) {
	//Snaps to the target value or stop in place
	if (snapToTargetValue) CurrentValue = TargetValue;
		else __setTargetValue_noLocks(CurrentValue);
	oldTargetValue = TargetValue;

	currentStatus.currentDirection = STOP;
	oldMovementStatus = false;
	setCurrentAcceleration(0);
	setCurrentSpeed(0);
}

float FChannelInterpolation::__calcNextMovement_internal(float previousSpeed, float deltaSeconds) {
	//Calcs the area of the rectangle with height = previousSpeed and base = deltaSeconds
	float rectArea = previousSpeed * deltaSeconds;
	float totalMovement = rectArea;

	//If we're not at constant speed, we calc the area of the triangle with height = currentSpeed - previousSpeed, and base = deltaSeconds
	if (currentStatus.currentSpeed != previousSpeed) {
		float triangleArea = (currentStatus.currentSpeed - previousSpeed) * deltaSeconds / 2;
		totalMovement += triangleArea; //We add the triangle area together with the rectangle
	}
	return totalMovement;
}
float FChannelInterpolation::calcNextMovement(float DeltaSeconds) {
	//You can find the idea of this function in the algo section at the end of "TestStuff/interpolation tests result.ini"

	//get the current speed, acceleration and time to fully accelerate
	float previousSpeed = currentStatus.currentSpeed; 
	float totalMovement = 0;
	float accel = currentStatus.IsAccelerationOverriden() ? currentStatus.accelerationOverride : currentStatus.currentAcceleration; //check if acceleration is overriden
	float ttfa = currentStatus.IsAccelerationOverriden() ? 1 / currentStatus.accelerationOverride : timeToFullyAccelerate;

	//if we're not at constant speed
	if (accel != 0) {
		//Calc the speed of this tick
		float previousSpeedAbs = currentStatus.currentSpeedAbs;
		setCurrentSpeed(currentStatus.currentSpeed + (accel * DeltaSeconds));

		//If our speed is bigger then 1 or less then 0, clamp it and calculate the movement before we reach 1 or 0
		bool overSpeed = currentStatus.currentSpeedAbs >= 1;
		if ((currentStatus.currentSpeed * currentStatus.currentDirection) <= 0 || overSpeed) {

			float remainingSpeed = overSpeed ? 1 - previousSpeedAbs : previousSpeedAbs; //Get the speed we need to reach 1 or 0 starting from the previous speed
			float remainingTime = remainingSpeed * ttfa; //Get the time we need to reach 0 or 1. This is the same calc of getStopDistance()
			
			//Clamp speed and set the current acceleration to 0
			setCurrentSpeed(overSpeed ? currentStatus.currentDirection : 0);
			setCurrentAcceleration(0);

			//if we surpass the clamp boundaries (and we don't EXACTLY meet them), we calculate first the movement from the previous tick to when we have reached 1 or 0, and then we update DeltaSeconds and and the local speed to calculate the movement from that moment to the current tick
			if (previousSpeedAbs != 0 && previousSpeedAbs != 1) {
				totalMovement += __calcNextMovement_internal(previousSpeed, remainingTime);

				previousSpeed = currentStatus.currentSpeed;
				DeltaSeconds -= remainingTime;
			}
		}
	}

	if (IsMoving()) totalMovement += __calcNextMovement_internal(previousSpeed, DeltaSeconds); //calc the movement
	return totalMovement * maxPhysicalSpeed; //Transform the movement to a physical movement
}

float FChannelInterpolation::getCorrectStopTimeDelta(const float distanceDifference, const float distanceDifferenceAbs, const float previousCurrentAcceleration, bool wasAccelerating) {
	if (wasAccelerating) {
		//Solve second grade equation to get the time difference.
		//We want to find the base of the red polygon you can find in TestStuff/testAccelDecel.png
		//We obtained the equation solving a system you can find in the Notes section at the end of "TestStuff/interpolation tests result.ini"
		const float y = previousCurrentAcceleration;
		const float h = currentStatus.currentSpeed;
		const float area = distanceDifference;

		//by multiplying everything per h we do less calcs. However the result (x1 and x2) are swapped and with the opposite sign
		const float a = y / 4; // y / (4 * h);
		const float b = h; //-1;
		const float c = area; //area / h;

		const float delta = b * b - 4 * a * c;
		const float rad = FMath::Sqrt(delta);

		if (distanceDifference > 0) {
			const float x1 = ((b - rad) / (2 * a)) / 2; //The delta time is equal to just half of the polygon base
			return x1;
		} else {
			//If our direction is backward the correct result will be in x2 instad of x1
			const float x2 = ((b + rad) / (2 * a)) / 2;
			return x2;
		}
	} else { //Constant speed
		//We want to find the base of the red rectangle (it's a rectangle because speed was constant) you can find in TestStuff/testAccelDecel.png
		return distanceDifferenceAbs / currentStatus.currentSpeedAbs; 
	}

}

bool FChannelInterpolation::compensateLateCall(float *DeltaSecond, float distanceDifference, InterpolationStatus& prevStatus) {
	//Since this interpolation class works only with relative changes (we don't know WHEN the target value changed nor how much time it passed since then)
	//we cannot compensate a late call where our target become less than the previous tick(considering a forward speed), specifically if it goes backwardsand we have to change direction.
	bool wasAccelerating = prevStatus.IsAccelerating();
	float prevAcceleration = prevStatus.currentAcceleration;

	//If we were accelerating and now we're at constant speed, it means that we've reaced maxSpeed between the previous and the current frame. We additionally check if we need to start decelerating before or after hitting need
	if (prevStatus.IsAccelerating() && !currentStatus.IsAccelerating() && currentStatus.currentSpeedAbs == 1) {
		float timeToReach1 = (1 - prevStatus.currentSpeedAbs) * timeToFullyAccelerate;
		float lclNextMov = __calcNextMovement_internal(prevStatus.currentSpeed, timeToReach1);
		float stopPositionBeforeWeReach1 = lclNextMov + CurrentValue + getStopDistance();
		if (currentStatus.currentDirection == FORWARD ? stopPositionBeforeWeReach1 > TargetValue : stopPositionBeforeWeReach1 < TargetValue) {
			//We need to decelerate before we hit maxSpeed
			distanceDifference = stopPositionBeforeWeReach1 - TargetValue; //Edit the distance difference interval as the stop position we have when we reach speed = 1
		} else {
			//We need to decelerate after hitting maxSpeed
			wasAccelerating = false; //Change the previous acceleration status to a constant speed
			prevAcceleration = 0;
		}
	}

	float normalizedDistance = distanceDifference / maxPhysicalSpeed; //Gets the distance in normalized speed
	float normalizedDistanceAbs = FMath::Abs(normalizedDistance);
	float deltaTimeDifference = getCorrectStopTimeDelta(normalizedDistance, normalizedDistanceAbs, prevAcceleration, wasAccelerating); //Gets the time difference when we had to start decelerating
	float correctStartDecelTime = *DeltaSecond - deltaTimeDifference; //Gets the exact time when we had to start decelerating

	//If we had to start decelerating between the previous tick and the current tick
	if (correctStartDecelTime > 0) {
		currentStatus = prevStatus; //Rollback to the previous tick
		float movement = calcNextMovement(correctStartDecelTime); //calc the movement from the previous tick to the moment when we had to start decelerating
		CurrentValue += movement; //add the movement to the currentValue

		*DeltaSecond = deltaTimeDifference; //update the delta time, so we're gonna calculate the movement from when we have started decelerating to the current tick
		return true;
	} else {
		//If we're here it means that target value has changed becoming closer to the CurrentValue since last time that Update() was called.
		//We have to decelerate faster, but only if this doesn't means we have to decelerate TOO MUCH fast or suddenly stop
		return false;
	}
}

float FChannelInterpolation::overrideDeceleration(float prevStopDistance, InterpolationStatus& prevStatus) {
	const float maxAcceleration = acceleration * MAX_FAST_DECELERATION_RATIO;
	float newAcceleration = maxAcceleration; //if correctStopTime is negative it means that TargetValue has been moved behind CurrentValue, so we have to stop faster and start moving backwards
	
	//obtain the stop time with a modified acceleration
	if (!hasSurpassedTargetValue()) {
		float distance = (prevStopDistance - TargetValue) * currentStatus.currentDirection; //get the difference between the target value and our stop distance

		//if (distance > 0) { //This should never happen since if distance < 0 it means that the moment when we had to start decelerating is between the current and the previous frame. But if we're overriding the deceleration it means that it should be before the previous frame. This is a conflict.
			float normalizedDistance = distance / maxPhysicalSpeed;
			float timeDifference = normalizedDistance * 2 / prevStatus.currentSpeedAbs; //get the time difference between when the interpolation should end to meet TargetValue and when it's going to end using a normal deceleration. This is the inverse formula to calc the base of a triangle where the area is the value difference and the height is the current speed
			float correctStopTime = (timeToFullyAccelerate * prevStatus.currentSpeedAbs) - timeDifference; //Calculate in how much time we have to reach a full stop to meet TargetValue

			//Assuming that the current time has x = 0 and y = currentSpeed, and that the stop time has x = stopTime and y = 0, calculate the coefficency of a line that passes between these two points. This is going to be our new acceleration
			if (correctStopTime > 0) {
				const float y1 = prevStatus.currentSpeedAbs;
				const float x2 = correctStopTime;
				//calc new deceleration to meet TargetValue
				newAcceleration = y1 / x2;

				//cap the new deceleration
				if (newAcceleration > maxAcceleration)
					newAcceleration = maxAcceleration;
			}
		//} else return 0;
	}
	return newAcceleration * currentStatus.currentDirection * -1;
}

float FChannelInterpolation::getTimeToMoveThruRange(float range) {
	float ttrx = timeToFullyAccelerate * currentStatus.currentSpeedAbs;
	float prevArea = ttrx * currentStatus.currentSpeedAbs / 2;
	float tArea = timeToFullyAccelerate - prevArea;

	if (tArea <= range) { //We fully accelerate
		return (timeToFullyAccelerate * 2 - ttrx) + (range - tArea);
	} else { //We don't fully accelerate
		return FMath::Sqrt((range + prevArea) / acceleration) * 2 - ttrx;
	}
}

void FChannelInterpolation::setMaxNormalizedSpeedFromTimeAndRange(float time, float range) {
	const float currentSpeed = currentStatus.currentSpeedAbs;

	//case x < currentSpeed
	const float ap = -timeToFullyAccelerate * currentSpeed;
	const float x = (0.5 * ap * currentSpeed + range) / (ap + time);
	if (x <= currentSpeed && x > 0 && x < 1) {
		maxNormalizedSpeed = x;
	} else {

		//case x >= currentSpeed
		const float a = timeToFullyAccelerate;
		const float b = timeToFullyAccelerate * currentSpeed + time;
		const float c = (timeToFullyAccelerate * currentSpeed * currentSpeed / 2) + range;

		const float delta = b * b - 4 * a * c;
		if (delta >= 0) {
			const float rad = FMath::Sqrt(delta);
			const float x1 = (b - rad) / (2 * a);
			if (x1 > 0 && x1 < 1) {
				maxNormalizedSpeed = x1;
			}
		}
	}
}
	
void FChannelInterpolation::__Update_noLocks(float DeltaSeconds) {
	if (IsUpdating()) {
		if(!bInterpolationEnabled) { __EndInterpolation_noLocks(); return; } //If interpolation is not enabled, snap directly to target value
		//Cap deltaSeconds based on our acceleration. This is a dirty hack to not have acceleration > 0.4,
		//while keeping consistency between getStopDistance(), update of our currentValue and calcNextMovement()
		float currentFrameAcceleration = acceleration * DeltaSeconds;
		if (currentFrameAcceleration > 0.4) DeltaSeconds = 0.4 / acceleration;

		//Save stuff before we update our speed and aceleration
		InterpolationStatus prevStatus = currentStatus;
		float prevStopDistance = getStopDistance();
		//Update speed and get our next position
		float movement = calcNextMovement(DeltaSeconds);
		 
		//Start/stop cases
		float stopDistance = getStopDistance();
		float stopPosition = stopDistance + CurrentValue + movement; //Get stop position
		bool movementStatus;
		bool stopped = IsStopped(DeltaSeconds);
		if (stopped) { //If we're not moving anymore
			if (IsInterpolationDone__ValueDiff(CurrentValue, movement)) { __EndInterpolation_noLocks(); return; } //Check if we actually met TargetValue. If yes, end the interpolation
			currentStatus.currentDirection = (CurrentValue + movement) < TargetValue ? FORWARD : BACKWARD; //Otherwise, update our interpolation direction and forcefully start to move
			movementStatus = true; //If we're here the interpolation is stopped BUT CurrentValue != TargetValue, so we MUST move
			oldMovementStatus = !movementStatus;
		} else { //If we're moving
			int sp = removePrecision(stopPosition);
			movementStatus = currentStatus.currentDirection == FORWARD ? sp < targetValueNoPrecision : sp > targetValueNoPrecision; //Check if, by starting decelerating in this frame, we surpass/meet TargetValue, or we still need some time before we start slowing down
		
			if (movementStatus) {
				if (maxNormalizedSpeed >= 0) {
					if ((prevStatus.currentSpeedAbs >= (maxNormalizedSpeed - acceleration * DeltaSeconds)) && (prevStatus.currentSpeedAbs <= (maxNormalizedSpeed + acceleration * DeltaSeconds))) {
						currentStatus.setAccelerationOverride(0); //We have reached our target maxSpeed. We now have to remain constant
						currentStatus.setCurrentSpeed(maxNormalizedSpeed * currentStatus.currentDirection);
					} else if (currentStatus.currentSpeedAbs > maxNormalizedSpeed) {
						movementStatus = false; //Our speed is over our maxSpeed. We need to decelerate
					} else if (currentStatus.currentSpeedAbs < maxNormalizedSpeed && movementStatus && oldMovementStatus && currentStatus.IsAccelerationOverriden() && currentStatus.accelerationOverride == 0) {
						currentStatus._isAccelerationOverriden = false; //Forcefully restart acceleration if we were at constant speed due to speed-cap and the speedcap moved upwards
					}
				} else {
					if (oldMovementStatus && currentStatus.IsAccelerationOverriden() && currentStatus.accelerationOverride == 0) {
						currentStatus._isAccelerationOverriden = false; //If we're here it means that the normalized speed cap has been removed, so we have to start accelerating again
					}
				}
			}
		}

		bool shouldCompensate = false;
		//If we're starting to accelerate or decelerate
		if (movementStatus != oldMovementStatus) {

			//Check if, even by starting to decelerate in this tick, we still surpass TargetValue. If yes, it means that OR the target value changed becoming closer to current value, OR we have to start decelerating in a moment between this and the previous tick. Here we handle the second case
			shouldCompensate = !movementStatus && (currentStatus.currentDirection == FORWARD ? stopPosition > TargetValue : stopPosition < TargetValue) && !hasSurpassedTargetValue();
			if (shouldCompensate) {
				float distanceDifference = stopPosition - TargetValue;
				shouldCompensate = compensateLateCall(&DeltaSeconds, distanceDifference, prevStatus); //Tries to correct the tick called too late respect to the moment when we had to start decelerating
			}

			//Starts accelerating/decellerating
 			oldMovementStatus = movementStatus;
			if (movementStatus) start(); else stop();

			if (shouldCompensate) { //compensateLateCall() may fail if TargetValue has changed drastically since the last tick. In that case we SHOULD NOT reload the movement, and overrideDeceleration() should handle that condition
				movement = calcNextMovement(DeltaSeconds); //reload our movement. We don't have to rollback to the previous tick since it's already done in compensateLateCall()
			}
		}

		//If the target value changed and using a normal deceleration speed we will end further from it, we need to decelerate faster. However, we don't need to decelerate faster if compensateLateCall() was able to do its job, so when the stop time difference is between this and the previous tick
		if (!shouldCompensate && IsDecelerating() && isTargetValueChanged() && (currentStatus.currentDirection == FORWARD ? stopPosition > TargetValue : stopPosition < TargetValue)) {
			float accelOverride = overrideDeceleration(prevStopDistance + CurrentValue, prevStatus);
			currentStatus.setAccelerationOverride(accelOverride); //Set the acceleration override. We don't set directly the acceleration because if TargetValue changes again to something further away from CurrentValue, we may want to just reduce our overridden acceleration until we reach again a normal acceleration
			currentStatus.currentSpeed = prevStatus.currentSpeed;
			currentStatus.currentSpeedAbs = prevStatus.currentSpeedAbs; //Rollback only the speed
			movement = calcNextMovement(DeltaSeconds); //reload our movement
		}

		//Check if we're stopped AND that we have reached TargetValue. If yes, end the interpolation
		if (IsInterpolationDone(DeltaSeconds, movement)) { __EndInterpolation_noLocks(); return; }

		//Update our position
		CurrentValue += movement;
		oldTargetValue = TargetValue;
	}

}

void FChannelInterpolation::__setTargetValue_noLocks(float value, bool checkSpeedCap) {
	TargetValue = value;
	if (bFirstValueNotSet) {
		__EndInterpolation_noLocks(true);
		bFirstValueNotSet = false;
		return;
	}
	targetValueNoPrecision = removePrecision(value);
	if (realFade > 0 && bSpeedCapEnabled && checkSpeedCap) {
		const float range = FMath::Abs(value - CurrentValue) / maxPhysicalSpeed;
		const float maxTime = FMath::Min(MIN_MOVEMENT_TIME, realFade * MIN_MOVEMENT_REALFADE_RATIO);
		float movTime = getTimeToMoveThruRange(range); //if you inline this in the following if, the function call is completely skipped idk why
		if (movTime < maxTime) {
			setMaxNormalizedSpeedFromTimeAndRange(maxTime, range);
		} else {
			maxNormalizedSpeed = -1;
		}
	}
}

void FChannelInterpolation::setFadeAndAcceleration(float RealAcceleration, float RealFade, float range) {
	range = FMath::Abs(range);
	//acceleration = increase of speed per second. RealAcceleration = how much time we need to get to speed = 1
	setAcceleration(1 / RealAcceleration);
	
	/*
	areaT = accelTime
	areaP = movTime - accelTime * 2

	area = areaT + areaP = movTime - accelTime 
	maxPhysicalSpeed = range / area
	*/
	if (RealFade <= RealAcceleration) {
		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Interpolation.setFadeAndAcceleration: Found a component with a RealFade <= RealAcceleration! RealFade: %f, RealAcceleration: %f"), RealFade, RealAcceleration);
		FDebug::DumpStackTraceToLog(TEXT("STACK-TRACE:"), ELogVerbosity::Error);
		RealFade = RealAcceleration + 0.1;
	}

	float area = RealFade - RealAcceleration;
	setMaxSpeed(range / area);

	this->realFade = RealFade;
}

#undef MAX_FAST_DECELERATION_RATIO