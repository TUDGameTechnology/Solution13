#include "Consideration.h"
#include "Kore/Math/Core.h"
#include <math.h>

int Consideration::GetRank() const
{
	float value = GetValue();
	float rankEvaluation = RankCurve->EvaluateAt(value);
	// Round down to the next int
	int rank = Kore::roundUp(rankEvaluation - 0.99999f);
	return rank;
}

float Consideration::GetWeight() const
{
	float value = GetValue();
	float weightEvaluation = WeightCurve->EvaluateAt(value);
	return weightEvaluation;
}

Consideration::~Consideration()
{
}

void Consideration::SetCurves(Curve* inRankCurve, Curve* inWeightCurve)
{
	RankCurve = inRankCurve;
	WeightCurve = inWeightCurve;
}

void Consideration::SetOwner(AICharacter* inOwner)
{
	owner = inOwner;
}

float ConstCurve::EvaluateAt(float x) const
{
	return ConstValue;
}

float BooleanCurve::EvaluateAt(float x) const
{
	switch (comparisonOperator)
	{
	case LessThen:
		return x < Threshold;
		break;
	case MoreThen:
		return x > Threshold;
		break;
	default:
		return 0.0f;
		break;
	}
}

float ExponentialDecayCurve::EvaluateAt(float x) const
{	
	float result = Kore::pow(Base, x) * Multiplier;
	// Clamp NAN values to the highest/lowest value to allow multiplication
	if (isinf(result))
	{
		if (result < 0.0f)
		{
			result = FLT_MIN;
		}
		else
		{
			result = FLT_MAX;
		}
	}
	return result;
}

CompositeConsideration::CompositeConsideration(CombinationMethod rankMethod /*= CM_Max*/, CombinationMethod weightMethod /*= CM_Multiply*/)
	: RankCombinationMethod(rankMethod), WeightCombinationMethod(weightMethod)
{
	considerations = new Consideration*[maxConsiderations];
	for (int i = 0; i < maxConsiderations; i++)
	{
		considerations[i] = nullptr;
	}
}

int CompositeConsideration::GetRank() const
{
	int result = GetNeutralValue<int>(RankCombinationMethod);
	for (Consideration** currentConsideration = considerations; *currentConsideration != nullptr; currentConsideration++)
	{
		int currentRank = (*currentConsideration)->GetRank();
		result = CombineValues(result, currentRank, RankCombinationMethod);
	}
	return result;
}

float CompositeConsideration::GetWeight() const
{
	float result = GetNeutralValue<float>(WeightCombinationMethod);
	for (Consideration** currentConsideration = considerations; *currentConsideration != nullptr; currentConsideration++)
	{
		float currentWeight = (*currentConsideration)->GetWeight();
		result = CombineValues(result, currentWeight, WeightCombinationMethod);
	}
	return result;
}

void CompositeConsideration::SetOwner(AICharacter* inOwner)
{
	Consideration::SetOwner(inOwner);
	for (Consideration** currentConsideration = considerations; *currentConsideration != nullptr; currentConsideration++)
	{
		(*currentConsideration)->SetOwner(owner);
	}
}

void CompositeConsideration::AddConsideration(Consideration* consideration)
{
	Consideration** current = considerations;
	while (*current != nullptr)
	{
		current++;
	}
	*current = consideration;
}

float IdentityCurve::EvaluateAt(float x) const
{
	return x;
}
