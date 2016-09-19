#pragma once

#include "pch.h"

class AICharacter;
class Consideration;
class Task;

/** An option of a utility-based reasoner */
class Option
{

public:

	Option();

	void SetOwner(AICharacter* inOwner);

	/** Get the rank of the option, based on its considerations */
	int GetRank() const;

	/** Get the weight of the option, based on its considerations */
	float GetWeight() const;

	void AddConsideration(Consideration* consideration);

	void Update(float DeltaTime);

	void SetTask(Task* inTask);

protected:

	/** The simulated AI character */
	AICharacter* owner;

	/** The maximal number of considerations */
	const int maxConsiderations = 20;

	/** Null-terminated list of considerations */
	Consideration** considerations;

	/** The task we are executing */
	Task* task = nullptr;

};