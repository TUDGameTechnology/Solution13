#include "Reasoner.h"
#include "Option.h"
#include "Kore/Math/Random.h"
#include "Kore/Log.h"
#include <string.h>

Reasoner::Reasoner(AICharacter* inOwner) : owner(inOwner)
{
	for (int i = 0; i < maxOptions; i++)
	{
		options[i] = nullptr;
	}
}

void Reasoner::Update(float DeltaTime)
{
	// Update the executing option
	if (executingOption != nullptr)
	{
		executingOption->Update(DeltaTime);
	}

	// Evaluate the options
	EvaluateOptions();
}

void Reasoner::AddOption(Option* option)
{
	Option** current;
	for (current = &options[0]; *current != nullptr; current++) {};
	*current = option;
	option->SetOwner(owner);
}

const char* Reasoner::GetStateString() const
{
	char* result = new char[256];
	result[0] = 0;
	Option* const * currentOption = options;
	for (currentOption; *currentOption != nullptr; currentOption++)
	{
		strcat(result, (*currentOption)->GetStateString());
	}
	return result;
}

void Reasoner::EvaluateOptions()
{
	// Clear the two arrays
	for (int i = 0; i < maxOptions; i++)
	{
		Rank[i] = 0;
		Weight[i] = 0.0f;
	}

	// Filter so that only the options with the highest rank are left
	int MaxRank = -1000;

	// Go over all considerations, save weight and rank and find the highest rank
	Option** currentOption = options;
	int i = 0;
	for (currentOption; *currentOption != nullptr; currentOption++)
	{
		Rank[i] = (*currentOption)->GetRank();
		if (Rank[i] > MaxRank)
		{
			MaxRank = Rank[i];
		}
		Weight[i] = (*currentOption)->GetWeight();

		i++;
	}

	int numOptions = i;

	// Get the overall weight
	float TotalWeight = 0.0f;
	for (i = 0; i < numOptions; i++)
	{
		if (Rank[i] == MaxRank)
		{
			TotalWeight += Weight[i];
		}
	}

	// Adjust the weights of the chosen options
	for (i = 0; i < numOptions; i++)
	{
		if (Rank[i] == MaxRank)
		{
			Weight[i] /= TotalWeight;
		}
	}

	// Get a random number between 0 and 1
	const Kore::s32 randMax = 1000000;
	float r = (float) Kore::Random::get(randMax) / (float) randMax;

	float CurrentSum = 0.0f;

	// Iterate over all considered weights and check which one is chosen
	Option* chosenOption = nullptr;
	for (i = 0; i < numOptions; i++)
	{
		if (Rank[i] == MaxRank)
		{
			CurrentSum += Weight[i];
			if (r <= CurrentSum)
			{
				chosenOption = options[i];
				break;
			}
		}
	}

	/** Replace the currently executing option */
	if (chosenOption != executingOption)
	{
		if (executingOption)
		{
			executingOption->Stop();
		}
		executingOption = chosenOption;
		if (executingOption)
		{
			executingOption->Start();
		}
		Kore::log(Kore::Info, "Reasoner: Executing a new option: %s", executingOption->GetName());
	}
}
