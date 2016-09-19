#include "DistanceConsideration.h"

#include "Steering.h"

float DistanceConsideration::GetValue() const
{
	return target->Position.distance(owner->Position);
}
