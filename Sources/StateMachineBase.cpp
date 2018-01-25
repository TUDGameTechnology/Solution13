/*
* Defines the base classes used for different state machines.
*
* Part of the Artificial Intelligence for Games system.
*
* Copyright (c) Ian Millington 2003-2006. All Rights Reserved.
*
* This software is distributed under licence. Use of this software
* implies agreement with all terms and conditions of the accompanying
* software licence.
*/
#include "StateMachineBase.h"

Action * BaseTransition::getActions()
{
	return nullptr;
}

bool ConditionalTransitionMixin::isTriggered()
{
	return condition->test();
}
