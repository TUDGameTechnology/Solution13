#include "Consideration.h"
#include "Kore/Math/Core.h"

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
