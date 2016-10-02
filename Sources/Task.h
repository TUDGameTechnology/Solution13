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


/** This task makes the character wander */
class WanderTask : public Task
{

public:
	WanderTask(AICharacter* aiCharacter):Task(aiCharacter){
		// Create wandering behaviour
		wander = new Wander();
		wander->character = aiCharacter;
		wander->maxAcceleration = 2.0f;
		wander->turnSpeed = 2.0f;
		wander->volatility = 20.0f;
	};

	virtual void Update(float DeltaTime) override;
protected:
	/** The wandering behaviour the AI should have*/
	Wander* wander;

};

/** This task makes the character follow another character */
class FollowTask : public Task {
public:
	FollowTask(AICharacter* aiCharacter):Task(aiCharacter){};

	void SetTarget(AICharacter* inTargetCharacter);

	virtual void Update(float DeltaTime) override;

protected:

	/** The target our character will want to follow */
	AICharacter* targetCharacter;
	/** The seeking behaviour the AI should have*/
	Seek* seek;

};