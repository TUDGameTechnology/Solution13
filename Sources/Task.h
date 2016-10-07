#pragma once

#include "pch.h"
#include "Steering.h"

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


/** This class defines a steering task with a undefined steering behaviour*/
class SteeringTask : public Task
{
public:
	SteeringTask(AICharacter* aiCharacter):Task(aiCharacter){};
	void Update(float DeltaTime);
	
protected:
	/** The steering behaviour the AI should have*/
	SteeringBehaviour* steeringBehaviour;
};


/** This task makes the character wander */
class WanderTask : public SteeringTask
{
public:
	WanderTask(AICharacter* aiCharacter);
};


/** This task makes the character follow another character */
class FollowTask : public SteeringTask
{
public:
	FollowTask(AICharacter* aiCharacter, AICharacter* inTargetCharacter);
	void setTarget(AICharacter* inTargetCharacter);

protected:
	/** The target our character will want to follow */
	AICharacter* targetCharacter;
};