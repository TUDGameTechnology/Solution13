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
	Option(const char* inName);

	void SetOwner(AICharacter* inOwner);

	/** Get the rank of the option, based on its considerations */
	int GetRank() const;

	/** Get the weight of the option, based on its considerations */
	float GetWeight() const;

	void Update(float DeltaTime);

	void SetTask(Task* inTask);

	const char* GetName() const;

	const char* GetStateString() const;

	void Start();

	void Stop();

	double GetLastStartTime() const {
		return LastStartTime;
	}

	bool GetIsExecuting() const {
		return isExecuting;
	}

	void SetRootConsideration(Consideration* newRoot);



protected:
	
	bool isExecuting;

	double LastStartTime;

	const char* name;

	Consideration* rootConsideration;

	/** The simulated AI character */
	AICharacter* owner;

	/** The task we are executing */
	Task* task = nullptr;

};