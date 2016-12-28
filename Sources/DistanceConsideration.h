#pragma once

#include "pch.h"

#include "Consideration.h"

/** A consideration that returns the distance between a target and the AICharacter */
class DistanceConsideration : public Consideration, public ITargetedConsideration
{

public:

	DistanceConsideration(AICharacter* inTarget)
	{
		target = inTarget;
	}

	virtual float GetValue() const override;

};