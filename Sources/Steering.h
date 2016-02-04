#pragma once

#include <Kore/pch.h>


#include <Kore/Math/Vector.h>
#include <Kore/Math/Random.h>
#include "MeshObject.h"


#include <vector>
#include <cstdlib>



/**
* SteeringOutput is a movement requested by the steering system.
*
* It consists of linear and angular components. The components
* may be interpreted as forces and torques when output from a
* full dynamic steering behaviour, or as velocity and rotation
* when output from a kinematic steering behaviour. In the former
* case, neither force nor torque take account of mass, and so
* should be thought of as linear and angular acceleration.
*/
struct SteeringOutput
{
	/**
	* The linear component of the steering action.
	*/
	Kore::vec2 linear;

	/**
	* The angular component of the steering action.
	*/
	float angular;

	void clear() {
		linear.set(0.0f, 0.0f);
		angular = 0.0f;
	}
};



class AICharacter {
public:
	Kore::vec2 Position;

	float Orientation;

	Kore::vec2 Velocity;

	float Rotation;

	MeshObject* meshObject;




	/**
	* Perfoms a forward Euler integration of the Kinematic for
	* the given duration, applying the given acceleration and
	* drag.  Because the integration is Euler, all the
	* acceleration is applied to the velocity at the end of the
	* time step.
	*
	* \note All of the integrate methods in this class are designed
	* to provide a simple mechanism for updating position. They are
	* not a substitute for a full physics engine that can correctly
	* resolve collisions and constraints.
	*
	* @param steer The acceleration to apply over the integration.
	*
	* @param drag The isotropic drag to apply to both velocity
	* and rotation. This should be a value between 0 (complete
	* drag) and 1 (no drag).
	*
	* @param duration The number of simulation seconds to
	* integrate over.
	*/
	void integrate(const SteeringOutput& steer,
		float drag, float duration);

	void trimMaxSpeed(float maxSpeed)
	{
		if (Velocity.getLength() > maxSpeed) {
			Velocity.normalize();
			Velocity *= maxSpeed;
		}
	}

	void setOrientationFromVelocity()
	{
		// If we haven't got any velocity, then we can do nothing.
		if (Velocity.getLength() > 0) {
			Orientation = Kore::atan2(Velocity.x(), Velocity.y());
		}
	}

	Kore::vec2 getOrientationAsVector() const
	{
		return Kore::vec2(Kore::sin(Orientation),
			Kore::cos(Orientation));
	}

};





/**
* The steering behaviour is the base class for all dynamic
* steering behaviours.
*/
class SteeringBehaviour
{
public:
	/**
	* The character who is moving.
	*/
	AICharacter *character;

	/**
	* Works out the desired steering and writes it into the given
	* steering output structure.
	*/
	virtual void getSteering(SteeringOutput* output) = 0;
};


/**
* The seek steering behaviour takes a target and aims right for
* it with maximum acceleration.
*/
class Seek : public SteeringBehaviour
{
public:
	/**
	* The target may be any vector (i.e. it might be something
	* that has no orientation, such as a point in space).
	*/
	const Kore::vec2 *target;

	/**
	* The maximum acceleration that can be used to reach the
	* target.
	*/
	float maxAcceleration;

	/**
	* Works out the desired steering and writes it into the given
	* steering output structure.
	*/
	virtual void getSteering(SteeringOutput* output) {
		/************************************************************************/
		// Task 1.1: Fill out this function
		// Write the result to output->linear
		// You can get the AI's position using character->Position
		/************************************************************************/
		// First work out the direction
		output->linear = *target - character->Position;
		// If there is no direction, do nothing
		if (output->linear.getLength() > 0)
		{
			output->linear = output->linear.normalize();
			output->linear *= maxAcceleration;
		}
	}
};


/**
* The seek steering behaviour takes a target and aims in the
* opposite direction with maximum acceleration.
*/
class Flee : public Seek
{
public:
	/**
	* Works out the desired steering and writes it into the given
	* steering output structure.
	*/
	virtual void getSteering(SteeringOutput* output) {
		/************************************************************************/
		// Task 1.1: Fill out this function
		// Write the result to output->linear
		// You can get the AI's position using character->Position
		/************************************************************************/
		Seek::getSteering(output);
		output->linear *= -1.0f;
	}
};


/**
* This subclass of seek is only intended as a base class for steering
* behaviours that create their own internal target, rather than having
* one assigned.
*/
class SeekWithInternalTarget : public Seek
{
protected:
	/**
	* Holds the actual target we're aiming for. This can be written
	* to by sub-classes (whereas the 'target' member cannot because
	* it is const).
	*/
	Kore::vec2 internal_target;

	/**
	* Creates a new behaviour and target. This method is protected
	* because this class isn't meant to be instantiated directly.
	*/
	SeekWithInternalTarget() {
		// Make the target pointer point at our internal target.
		target = &internal_target;
	}
};



/**
* The wander behaviour moves the relative target around the moving
* agent at random, then uses seek to head for it.
*
* The seek target for this class is created and destroyed by the
* class, and should not be assigned to.
*/
class Wander : public SeekWithInternalTarget
{
public:
	/**
	* This controls the degree to which the character moves in a
	* straight line. Specifically it controls how far ahead to aim
	* when the character is in motion. Too small values means the
	* character may be able to overshoot their target, and so will
	* oscillate wildly.
	*/
	float volatility;

	/**
	* This controls how fast the character may turn.
	*/
	float turnSpeed;

	static float randomReal(float max)
	{
		return max * (float(Kore::Random::get(RAND_MAX) / (float) RAND_MAX));
	}

	static float randomBinomial(float max)
	{
		return randomReal(max) - randomReal(max);
	}

	/**
	* Works out the desired steering and writes it into the given
	* steering output structure.
	*/
	virtual void getSteering(SteeringOutput* output) {
		// Make sure we have a target
		if (target->getLength() == 0) {
			internal_target = character->Position;
			internal_target[0] += volatility;
		}

		Kore::vec2 offset = *target - character->Position;
		float angle;
		if (offset.x()*offset.x() + offset.y()*offset.y() > 0) {
			// Work out the angle to the target from the character
			angle = Kore::atan2(offset.y(), offset.x());
		}
		else
		{
			// We're on top of the target, move it away a little.
			angle = 0;
		}

		// Move the target to the boundary of the volatility circle.
		internal_target = character->Position;
		internal_target[0] += volatility * Kore::cos(angle);
		internal_target[1] += volatility * Kore::sin(angle);

		// Add the turn to the target
		internal_target[0] += randomBinomial(turnSpeed);
		internal_target[1] += randomBinomial(turnSpeed);

		Seek::getSteering(output);
	}
};


/**
* Blended steering takes a set of steering behaviours and generates an
* output by doing a weighted blend of their outputs.
*/
class BlendedSteering : public SteeringBehaviour
{
public:
	/**
	* Holds a steering behaviour with its associated weight.
	*/
	struct BehaviourAndWeight
	{
		SteeringBehaviour *behaviour;
		float weight;

		BehaviourAndWeight(SteeringBehaviour *behaviour, float weight = 1.0f)
			:
			behaviour(behaviour), weight(weight)
		{}
	};

	/**
	* Holds the list of behaviour and their corresponding blending
	* weights.
	*/
	std::vector<BehaviourAndWeight> behaviours;

	/**
	* Works out the desired steering and writes it into the given
	* steering output structure.
	*/
	virtual void getSteering(SteeringOutput* output) {
		// Clear the output to start with
		output->clear();

		// Go through all the behaviours in the list
		std::vector<BehaviourAndWeight>::iterator baw;
		float totalWeight = 0;
		SteeringOutput temp;
		for (baw = behaviours.begin(); baw != behaviours.end(); baw++)
		{
			// Make sure the children's character is set
			baw->behaviour->character = character;

			// Get the behaviours steering and add it to the accumulator
			baw->behaviour->getSteering(&temp);
			output->linear += temp.linear * baw->weight;
			output->angular += temp.angular * baw->weight;

			totalWeight += baw->weight;
		}

		// Divide the accumulated output by the total weight
		if (totalWeight > 0.0)
		{
			totalWeight = 1.0f / totalWeight;
			output->linear *= totalWeight;
			output->angular *= totalWeight;
		}
	}
};
