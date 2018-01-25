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
#include "Flocking.h"
#include <cstring>

using namespace Kore;

Flock::Flock()
:
inNeighbourhood(0), arraySize(0)
{}

unsigned Flock::prepareNeighourhood(
									const AICharacter* of,
									float size,
									float minDotProduct /* = -1.0 */)
{
	// Make sure the array is of the correct size
	if (arraySize != boids.size())
	{
		if (arraySize) delete[] inNeighbourhood;
		arraySize = boids.size();
		if (arraySize) inNeighbourhood = new bool[arraySize];
		memset(inNeighbourhood, 0, sizeof(bool)*arraySize);
	}
	
	// Compile the look vector if we need it
	vec2 look;
	if (minDotProduct > -1.0f)
	{
		look = of->getOrientationAsVector();
	}
	
	Flock result;
	std::list<AICharacter*>::iterator bi;
	unsigned i = 0, count = 0;;
	for (bi = boids.begin(); bi != boids.end(); bi++, i++)
	{
		AICharacter*k = *bi;
		inNeighbourhood[i] = false;
		
		// Ignore ourself
		if (k == of) continue;
		
		// Check for maximum distance
		if (k->Position.distance(of->Position) > size) continue;
		
		// Check for angle
		if (minDotProduct > -1.0)
		{
			vec2 offset = k->Position - of->Position;
			vec2 offsetNormalized = offset;
			offsetNormalized = offsetNormalized.normalize();
			if (look.dot(offsetNormalized) < minDotProduct)
			{
				continue;
			}
		}
		
		// If we get here we've passed all tests
		inNeighbourhood[i] = true;
		count++;
	}
	return count;
}

vec2 Flock::getNeighbourhoodCenter()
{
	vec2 center;
	std::list<AICharacter*>::iterator bi;
	unsigned i = 0;
	unsigned count = 0;
	for (bi = boids.begin(); bi != boids.end(); bi++, i++)
	{
		if (inNeighbourhood[i])
		{
			center += (*bi)->Position;
			count++;
		}
	}
	center *= 1.0f / (float)count;
	
	return center;
}


vec2 Flock::getNeighbourhoodAverageVelocity()
{
	vec2 center;
	std::list<AICharacter*>::iterator bi;
	unsigned i = 0;
	unsigned count = 0;
	for (bi = boids.begin(); bi != boids.end(); bi++, i++)
	{
		if (inNeighbourhood[i])
		{
			center += (*bi)->Velocity;
			count++;
		}
	}
	center *= 1.0f / (float)count;
	
	return center;
}


void Separation::getSteering(SteeringOutput* output)
{
	// Get the neighbourhood of boids
	unsigned count = theFlock->prepareNeighourhood(character, neighbourhoodSize, neighbourhoodMinDP);
	if (count <= 0) return;
	
	// Work out their center of mass
	vec2 cofm = theFlock->getNeighbourhoodCenter();
	
	// Steer away from it.
	
	flee.maxAcceleration = maxAcceleration;
	flee.character = character;
	flee.target = &cofm;
	flee.getSteering(output);
	
}

void Cohesion::getSteering(SteeringOutput* output)
{
	// Get the neighbourhood of boids
	unsigned count = theFlock->prepareNeighourhood(character, neighbourhoodSize, neighbourhoodMinDP);
	if (count <= 0) return;
	
	// Work out their center of mass
	vec2 cofm = theFlock->getNeighbourhoodCenter();
	
	// Steer towards it
	seek.maxAcceleration = maxAcceleration;
	seek.character = character;
	seek.target = &cofm;
	seek.getSteering(output);
}


void VelocityMatchAndAlign::getSteering(SteeringOutput* output)
{
	// Get the neighbourhood of boids
	unsigned count = theFlock->prepareNeighourhood(character, neighbourhoodSize, neighbourhoodMinDP);
	if (count <= 0) return;
	
	// Work out their center of mass
	vec2 vel = theFlock->getNeighbourhoodAverageVelocity();
	
	// Try to match it
	output->linear = vel - character->Velocity;
	if (output->linear.getLength()> maxAcceleration)
	{
		output->linear = output->linear.normalize();
		output->linear *= maxAcceleration;
	}
}
