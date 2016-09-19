#include "Option.h"
#include "Consideration.h"
#include "Task.h"

Option::Option()
{
	considerations = new Consideration*[maxConsiderations];
	for (int i = 0; i < maxConsiderations; i++)
	{
		considerations[i] = 0;
	}
}

void Option::SetOwner(AICharacter* inOwner)
{
	owner = inOwner;
	// Set on all considerations
	for (Consideration** currentConsideration = considerations; *currentConsideration != nullptr; currentConsideration++)
	{
		(*currentConsideration)->SetOwner(owner);
	}
}

int Option::GetRank() const
{
	int maxRank = 0;
	for (Consideration** currentConsideration = considerations; *currentConsideration != nullptr; currentConsideration++)
	{
		int currentRank = (*currentConsideration)->GetRank();
		if (currentRank > maxRank)
		{
			maxRank = currentRank;
		}
	}
	return maxRank;
}

float Option::GetWeight() const
{
	// Return the product of the individual weights
	float product = 1.0f;
	for (Consideration** currentConsideration = considerations; *currentConsideration != nullptr; currentConsideration++)
	{
		product *= (*currentConsideration)->GetWeight();
	}

	return product;
}

void Option::AddConsideration(Consideration* consideration)
{
	Consideration** current = considerations;
	while (*current != nullptr)
	{
		current++;
	}
	*current = consideration;
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
