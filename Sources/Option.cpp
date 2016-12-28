#include "Option.h"
#include "Consideration.h"
#include "Task.h"
#include <float.h>
#include <Kore/System.h>

Option::Option()
	:Option(nullptr)
{
}

Option::Option(const char* inName)
	: name(inName), LastStartTime(DBL_MAX), LastStopTime(DBL_MAX), isExecuting(false), rootConsideration(nullptr)
{
}

void Option::SetOwner(AICharacter* inOwner)
{
	owner = inOwner;
	if (rootConsideration)
	{
		rootConsideration->SetOwner(inOwner);
	}
}

int Option::GetRank() const
{
	if (rootConsideration)
	{
		return rootConsideration->GetRank();
	}

	return 0;
}

float Option::GetWeight() const
{
	if (rootConsideration)
	{
		return rootConsideration->GetWeight();
	}

	return 0.0f;
}

void Option::Update(float DeltaTime)
{
	if (task != nullptr)
	{
		task->Update(DeltaTime);
	}
}

void Option::SetTask(Task* inTask)
{
	task = inTask;
}

const char* Option::GetName() const
{
	return name;
}

const char* Option::GetStateString() const
{
	char* result = new char[256];
	result[0] = 0;
	snprintf(result, 256, "%s: R: %i, W: %.2f\n", GetName(), GetRank(), GetWeight());

	return result;
}

void Option::Start()
{
	LastStartTime = Kore::System::time();
	LastStopTime = DBL_MAX;
	isExecuting = true;
}

void Option::Stop()
{
	LastStopTime = Kore::System::time();
	isExecuting = false;
}

void Option::SetRootConsideration(Consideration* newRoot)
{
	rootConsideration = newRoot;
}
