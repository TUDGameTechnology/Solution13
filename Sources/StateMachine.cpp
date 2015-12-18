#include "pch.h"
#include "StateMachine.h"

StateMachineState * FixedTargetTransitionMixin::getTargetState()
{
	return target;
}


Action * StateMachineState::getEntryActions()
{
	return nullptr;
}

Action * StateMachineState::getActions()
{
	return nullptr;
}

Action * StateMachineState::getExitActions()
{
	return nullptr;
}

Action* StateMachine::update()
{
	// The variable to hold the actions to perform
	Action * actions = nullptr;

	// First case - we have no current state.
	if (currentState == nullptr)
	{
		// In this case we use the entry action for the initial state.
		if (initialState != nullptr) {

			// Transition to the first state
			currentState = initialState;

			// Returns the initial states entry actions
			actions = currentState->getEntryActions();

		}
		else {

			// We have nothing to do
			actions = nullptr;
		}
	}

	// Otherwise we have a current state to work with
	else {
		// Start off with no transition
		Transition * transition = nullptr;

		// Check through each transition in the current state.
		BaseTransition * testTransition = currentState->firstTransition;
		while (testTransition != nullptr) {
			if (testTransition->isTriggered()) {
				transition = (Transition*)testTransition;
				break;
			}
			testTransition = testTransition->next;
		}

		// Check if we found a transition
		if (transition) {
			// Find our destination
			StateMachineState * nextState = transition->getTargetState();

			// Accumulate our list of actions
			Action * tempList = nullptr;
			Action * last = nullptr;

			// Add each element to the list in turn
			actions = currentState->getExitActions();
			last = actions->getLast();

			tempList = transition->getActions();
			last->next = tempList;
			last = tempList->getLast();

			tempList = nextState->getEntryActions();
			last->next = tempList;
			last = tempList->getLast();

			tempList = nextState->getActions();
			last->next = tempList;

			// Update the change of state
			currentState = nextState;
		}

		// Otherwise our actions to perform are simply those for the
		// current state.
		else {

			actions = currentState->getActions();
		}
	}

	return actions;
}