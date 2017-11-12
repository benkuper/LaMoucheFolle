/*
  ==============================================================================

    NodeManagerUI.h
    Created: 10 Nov 2017 10:29:44am
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "NodeManager.h"
#include "Node.h"

class NodeManagerUI :
	public BaseManagerShapeShifterUI<NodeManager, Node, BaseItemUI<Node>>
{
public:
	NodeManagerUI(const String &name, NodeManager * manager);
	~NodeManagerUI();

	static NodeManagerUI * create(const String &name) { return new NodeManagerUI(name, NodeManager::getInstance()); }


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeManagerUI)
};