#pragma once

#include "pch.h"

class AICharacter;

/** Base class for utility-based AI tasks. A task controls the actions of an AI and is periodically updated */
class Task
{
public:

	Task(AICharacter* aiCharacter);

	virtual ~Task();

	virtual void Update(float DeltaTime) = 0;


protected:

	/** The character that is controlled by this task */
	AICharacter* myCharacter;

};


/** This task makes the character wander */
class WanderTask : public Task
{

public:

	virtual void Update(float DeltaTime) override;

};

/** This task makes the character follow another character */
class FollowTask : public Task {
public:

	void SetTarget(AICharacter* inTargetCharacter);

	virtual void Update(float DeltaTime) override;

protected:

	/** The target our character will want to follow */
	AICharacter* targetCharacter;

};