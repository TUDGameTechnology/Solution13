#pragma once

#include "pch.h"
#include <limits.h>
#include <float.h>


class AICharacter;

/** Base class for curves */
class Curve
{
public:

	/** Return the evaluation of the function at the point x */
	virtual float EvaluateAt(float x) const = 0;

};


/** Outputs the input directly */
class IdentityCurve : public Curve
{
public:

	virtual float EvaluateAt(float x) const override;

};

/** A curve with a constant value */
class ConstCurve : public Curve
{
public:

	float ConstValue;

	virtual float EvaluateAt(float x) const override;

};


class ValueInRangeCurve : public Curve
{
public:	

	float LowerBound;
	float UpperBound;
	float Multiplier;

	ValueInRangeCurve(float lower, float upper, float multiplier = 1.0f)
		: LowerBound(lower), UpperBound(upper), Multiplier(multiplier)
	{
	}

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

	BooleanCurve(ComparisonOperator inComparisonOperator = LessThen, float inThreshold = 0.5f)
		: comparisonOperator(inComparisonOperator), Threshold(inThreshold)
	{
	}

	ComparisonOperator comparisonOperator;

	float Threshold;

	virtual float EvaluateAt(float x) const override;
};

/** Calculates (Base ^ x) * Multiplier for x >= 0, 0 for x < 0 */
class ExponentialDecayCurve : public Curve
{

public:

	ExponentialDecayCurve(float base, float multiplier = 1.0f)
		: Base(base), Multiplier(multiplier)
	{}

	float Base;

	float Multiplier;

	virtual float EvaluateAt(float x) const override;
};



/** Base class for utility-based AI considerations. */
class Consideration
{
public:

	/** Implement this function to return a value, depending on what the consideration does. */
	virtual float GetValue() const = 0;

	virtual int GetRank() const;

	virtual float GetWeight() const;

	virtual ~Consideration();

	void SetCurves(Curve* inRankCurve, Curve* inWeightCurve);

	virtual void SetOwner(AICharacter* inOwner);

protected:

	Curve* RankCurve;

	Curve* WeightCurve;

	AICharacter* owner;

};

/** A consideration that combines two or more considerations */
class CompositeConsideration : public Consideration
{
public:

	enum CombinationMethod{
		CM_Min,
		CM_Max,
		CM_Multiply
	};

	CompositeConsideration(CombinationMethod rankMethod = CM_Max, CombinationMethod weightMethod = CM_Multiply);

	virtual float GetValue() const override {
		// We don't really need a value here
		return 0.0f;
	}

	virtual int GetRank() const override;

	virtual float GetWeight() const override;

	virtual void SetOwner(AICharacter* inOwner) override;

	void AddConsideration(Consideration* consideration);

protected:

	/** The maximal number of considerations */
	const int maxConsiderations = 20;

	/** Null-terminated list of considerations */
	Consideration** considerations;

	template<class T>
	T CombineValues(T first, T second, CombinationMethod method) const;

	template<class T>
	T GetNeutralValue(CombinationMethod method) const;

	template<>
	int GetNeutralValue<int>(CombinationMethod method) const;

	template<>
	float GetNeutralValue<float>(CombinationMethod method) const;

	CombinationMethod RankCombinationMethod;
	CombinationMethod WeightCombinationMethod;

};

template<class T>
T CompositeConsideration::CombineValues(T first, T second, CombinationMethod method) const
{
	switch (method)
	{
	case CM_Min:
		return Kore::min(first, second);
		break;
	case CM_Max:
		return Kore::max(first, second);
		break;
	case CM_Multiply:
		return first * second;
		break;
	default:
		break;
	}
	return 0;
}


template<>
int CompositeConsideration::GetNeutralValue(CombinationMethod method) const
{
	switch (method)
	{
	case CM_Min:
		return INT_MAX;
		break;
	case CM_Max:
		return INT_MIN;
		break;
	case CM_Multiply:
		return 1;
		break;
	default:
		break;
	}
	return 0;
}

template<>
float CompositeConsideration::GetNeutralValue(CombinationMethod method) const
{
	switch (method)
	{
	case CM_Min:
		return FLT_MAX;
		break;
	case CM_Max:
		return FLT_MIN;
		break;
	case CM_Multiply:
		return 1.0f;
		break;
	default:
		break;
	}
	return 0.0f;
}

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

