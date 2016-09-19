#include "Task.h"

Task::Task(AICharacter* aiCharacter) : myCharacter(aiCharacter)
{

}

Task::~Task()
{

}

void WanderTask::Update(float DeltaTime)
{


}

void FollowTask::SetTarget(AICharacter* inTargetCharacter)
{
	targetCharacter = inTargetCharacter;
}

void FollowTask::Update(float DeltaTime)
{

}