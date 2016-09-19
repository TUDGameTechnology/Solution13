#pragma once

#include "pch.h"

#include "Consideration.h"

/** A consideration that returns the distance between a target and the AICharacter */
class DistanceConsideration : public Consideration, public ITargetedConsideration
{

public:

	virtual float GetValue() const override;

};