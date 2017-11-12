/*
  ==============================================================================

    NodeManager.h
    Created: 10 Nov 2017 10:29:32am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Node.h"

class NodeManager :
	public BaseManager<Node>
{
public:
	juce_DeclareSingleton(NodeManager, true)

	NodeManager();
	~NodeManager();

	void setNodePosition(int nodeID, float x, float y, float z);

	Array<Vector3D<float>> getAllPositions();
	Node * getNodeByID(int id);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeManager)
};