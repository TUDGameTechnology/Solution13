#include "Task.h"

namespace{
	using namespace Kore;
}

Task::Task(AICharacter* aiCharacter) : myCharacter(aiCharacter)
{

}

Task::~Task()
{

}

void SteeringTask::Update(float DeltaTime){
	// Update the steering behaviours
	float duration = DeltaTime;
	
	// One steering output is re-used
	SteeringOutput steer;
	
	// Handle the character
	steeringBehaviour->getSteering(&steer);
	myCharacter->integrate(steer, 0.95f, duration);
	myCharacter->trimMaxSpeed(1.0f);
	
	myCharacter->meshObject->M = Kore::mat4::Translation(myCharacter->Position[0], 0.0f, myCharacter->Position[1]);
	
}

WanderTask::WanderTask(AICharacter* aiCharacter):SteeringTask(aiCharacter){
	// Create wandering behaviour
	Wander* wander = new Wander();
	wander->character = myCharacter;
	wander->maxAcceleration = 2.0f;
	wander->turnSpeed = 2.0f;
	wander->volatility = 20.0f;
	steeringBehaviour = wander;
}


FollowTask::FollowTask(AICharacter* aiCharacter, AICharacter* inTargetCharacter):SteeringTask(aiCharacter){
	//Set target in order to create the seeking behaviour
	setTarget(inTargetCharacter);
}

void FollowTask::setTarget(AICharacter* inTargetCharacter){
	// Set the target
	targetCharacter = inTargetCharacter;
	
	// Create or update (if target has changed) the seeking behaviour
	Seek* seek = new Seek();
	seek->character = myCharacter;
	seek->maxAcceleration = 3.0f;
	seek->target = &(targetCharacter->Position);
	
	steeringBehaviour = seek;
}