#pragma once



/*
* Steering behaviours for the flocking demo.
*
* Part of the Artificial Intelligence for Games system.
*
* Copyright (c) Ian Millington 2003-2006. All Rights Reserved.
*
* This software is distributed under licence. Use of this software
* implies agreement with all terms and conditions of the accompanying
* software licence.
*/

#include "Steering.h"


#include <list>




/**
* This class stores a flock of creatures.
*/
class Flock
{
public:
	std::list<AICharacter*> boids;
	bool *inNeighbourhood;
	unsigned arraySize;

	Flock();

	/**
	* Sets up the boolean flags of boids in the neighbourhood of the given boid.
	*/
	unsigned prepareNeighourhood(
		const AICharacter* of,
		float size,
		float minDotProduct = -1.0
		);

	/**
	* Returns the geometric center of the flock.
	*/
	Kore::vec2 getNeighbourhoodCenter();

	/**
	* Returns the average velocity of the flock.
	*/
	Kore::vec2 getNeighbourhoodAverageVelocity();
};

class BoidSteeringBehaviour : public SteeringBehaviour
{
public:
	Flock *theFlock;
	float neighbourhoodSize;
	float neighbourhoodMinDP;
	float maxAcceleration;
};

class Separation : public BoidSteeringBehaviour
{
	Flee flee;

public:
	virtual void getSteering(SteeringOutput* output);
};

class Cohesion : public BoidSteeringBehaviour
{
	Seek seek;

public:
	virtual void getSteering(SteeringOutput* output);
};

class VelocityMatchAndAlign : public BoidSteeringBehaviour
{
public:
	virtual void getSteering(SteeringOutput* output);
};


