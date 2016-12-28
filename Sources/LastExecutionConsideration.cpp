#include "LastExecutionConsideration.h"
#include "Option.h"
#include <Kore\System.h>


float LastExecutionConsideration::GetValue() const
{
	double now = Kore::System::time();
	double result = now - TargetOption->GetLastStartTime();
	return (float)result;
}

float IsExecutingConsideration::GetValue() const
{
	return TargetOption->GetIsExecuting();
}

float TargetOptionConsideration::GetValue() const
{
	return 0.0f;
}

float LastExecutionStoppedConsideration::GetValue() const
{
	double now = Kore::System::time();
	double result = now - TargetOption->GetLastStopTime();
	return (float)result;
}
