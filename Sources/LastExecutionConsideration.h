#pragma once

#include "pch.h"

#include "Consideration.h"



class TargetOptionConsideration : public Consideration
{

public:

	TargetOptionConsideration(class Option* inTargetOption)
		: TargetOption(inTargetOption){
	}

	Option* TargetOption;

	virtual float GetValue() const override;

};


/** Returns the time elapsed since the last time the specified option was started. Negative values indicate that it has not been started yet. */ 
class LastExecutionConsideration : public TargetOptionConsideration
{

public:

	LastExecutionConsideration(Option* inTargetOption)
		: TargetOptionConsideration(inTargetOption)
	{
	}

	virtual float GetValue() const override;

};

/** Returns 1 iff the option is being executed */
class IsExecutingConsideration : public TargetOptionConsideration
{

public:

	IsExecutingConsideration(Option* inTargetOption)
		: TargetOptionConsideration(inTargetOption)
	{
	}

	virtual float GetValue() const override;

};