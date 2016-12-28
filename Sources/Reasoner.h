#pragma once

#include "pch.h"

class Option;
class AICharacter;

/** A utility-based reasoner */
class Reasoner {

public:

	Reasoner(AICharacter* inOwner);

	/** Update the reasoner each tick. This chooses which option(s) should be executed and updates the running tasks */
	void Update(float DeltaTime);

	/** Add an option to the reasoner */
	void AddOption(Option* option);

	const char* GetStateString() const;

private:

	/** Updates the options and chooses which ones to execute */
	void EvaluateOptions();

	/** The maximal number of options */
	const int maxOptions = 20;

	/** Null-terminated array of options */
	Option* options[20];

	/** Arrays used during evaluation */
	int Rank[20];
	float Weight[20];

	/** The currently executing option. For simplicity purposes, we only allow one option to be executed at any time */
	Option* executingOption = nullptr;

	/** The AICharacter controlled by this reasoner */
	AICharacter* owner;

};