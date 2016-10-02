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

void WanderTask::Update(float DeltaTime)
{
	// Update the steering behaviours
	float duration = DeltaTime;
	
	// One steering output is re-used
	SteeringOutput steer;
	
	// Handle the moon
	SteeringBehaviour* moonBehaviour = wander;
	moonBehaviour->getSteering(&steer);
	myCharacter->integrate(steer, 0.95f, duration);
	myCharacter->trimMaxSpeed(1.0f);
	
	myCharacter->meshObject->M = mat4::Translation(myCharacter->Position[0], 0.0f, myCharacter->Position[1]);

}

void FollowTask::SetTarget(AICharacter* inTargetCharacter)
{
	// Set the Target
	targetCharacter = inTargetCharacter;
	
	// Create seeking behaviour
	seek = new Seek();
	seek->character = myCharacter;
	seek->maxAcceleration = 3.0f;
	seek->target = &(targetCharacter->Position);
	
}

void FollowTask::Update(float DeltaTime)
{
	// Update the steering behaviours
	float duration = DeltaTime;
	
	// One steering output is re-used
	SteeringOutput steer;
	
	// Handle the moon
	SteeringBehaviour* moonBehaviour = seek;
	moonBehaviour->getSteering(&steer);
	myCharacter->integrate(steer, 0.95f, duration);
	myCharacter->trimMaxSpeed(1.0f);
	
	myCharacter->meshObject->M = mat4::Translation(myCharacter->Position[0], 0.0f, myCharacter->Position[1]);
}
