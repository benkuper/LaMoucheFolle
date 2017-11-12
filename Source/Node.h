/*
  ==============================================================================

    Node.h
    Created: 10 Nov 2017 10:31:10am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class Node :
	public BaseItem
{
public:
	Node();
	~Node();

	IntParameter * id;
	Point3DParameter * position;
	
	String getTypeString() const override { return "Node"; }

private:
	SpinLock cfLock;
};