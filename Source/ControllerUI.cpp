/*
  ==============================================================================

    ControllerUI.cpp
    Created: 30 Oct 2017 9:21:53am
    Author:  Ben

  ==============================================================================
*/

#include "ControllerUI.h"
#include "CFAssetManager.h"

ControllerUI::ControllerUI(Controller * c) :
	BaseItemUI(c)
{
	inActivityUI = c->inTrigger->createImageUI(CFAssetManager::getInstance()->getInImage());
	inActivityUI->showLabel = false;
	addAndMakeVisible(inActivityUI);

	outActivityUI = c->outTrigger->createImageUI(CFAssetManager::getInstance()->getOutImage());
	outActivityUI->showLabel = false;
	addAndMakeVisible(outActivityUI);
}

ControllerUI::~ControllerUI()
{

}

void ControllerUI::resizedInternalHeader(Rectangle<int>& r)
{
	outActivityUI->setBounds(r.removeFromRight(headerHeight));
	inActivityUI->setBounds(r.removeFromRight(headerHeight));
}