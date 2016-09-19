#pragma once

#include "pch.h"

class AICharacter;

/** Base class for curves */
class Curve
{
public:

	/** Return the evaluation of the function at the point x */
	virtual float EvaluateAt(float x) const = 0;

};

/** A curve with a constant value */
class ConstCurve : public Curve
{
public:

	float ConstValue;

	virtual float EvaluateAt(float x) const override;

};

/** A curve that evaluates to 0 below a threshold and 1 above the threshold */
class BooleanCurve : public Curve
{

public:

	enum ComparisonOperator{
		LessThen,				// Return 1 if the value is below the threshold
		MoreThen				// Return 1 if the value is above the threshold
	} ;

	ComparisonOperator comparisonOperator;

	float Threshold;

	virtual float EvaluateAt(float x) const override;
};

/** Base class for utility-based AI considerations. */
class Consideration
{
public:

	/** Implement this function to return a value, depending on what the consideration does. */
	virtual float GetValue() const = 0;

	int GetRank() const;

	float GetWeight() const;

	virtual ~Consideration();

	void SetCurves(Curve* inRankCurve, Curve* inWeightCurve);

	void SetOwner(AICharacter* inOwner);

protected:

	Curve* RankCurve;

	Curve* WeightCurve;

	AICharacter* owner;

};


/** Mixin for considerations that have a target character */
class ITargetedConsideration
{

public:

	void SetTarget(AICharacter* inTarget)
	{
		target = inTarget;
	}

protected:

	/** The target of this consideration */
	AICharacter* target;
};

