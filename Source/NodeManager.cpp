/*
  ==============================================================================

    NodeManager.cpp
    Created: 10 Nov 2017 10:29:32am
    Author:  Ben

  ==============================================================================
*/

#include "NodeManager.h"

juce_ImplementSingleton(NodeManager)

NodeManager::NodeManager() :
	BaseManager("Nodes")
{
}

NodeManager::~NodeManager()
{
}

void NodeManager::setNodePosition(int nodeID, float x, float y, float z)
{
	Node * n = getNodeByID(nodeID);
	if (n == nullptr) return;
	n->position->setVector(x, y, z);
}

Array<Vector3D<float>> NodeManager::getAllPositions()
{
	Array<Vector3D<float>> result;
	for (int i = 0; i < 8; i++)
	{
		Vector3D<float> v(0, 0, 0);
		Node * n = getNodeByID(i);
		if (n != nullptr) v = n->position->getVector();
		result.add(v);
	}

	return result;
}
		

Node * NodeManager::getNodeByID(int id)
{
	for (Node * n : items)
	{
		if (n->id->intValue() == id) return n;
	}

	return nullptr;
}
