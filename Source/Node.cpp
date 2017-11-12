/*
  ==============================================================================

    Node.cpp
    Created: 10 Nov 2017 10:31:10am
    Author:  Ben

  ==============================================================================
*/

#include "Node.h"

Node::Node() :
	BaseItem("Node")
{
	id = addIntParameter("Node ID", "ID of the anchor", 0, 0, 7);
	position = addPoint3DParameter("Position", "Position of the node");
	position->setBounds(-10, -10, -10, 10, 10, 10);
}

Node::~Node()
{

}
