/*
  ==============================================================================

    ControllerUI.h
    Created: 30 Oct 2017 9:21:53am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "../Controller.h"

class ControllerUI :
	public BaseItemUI<Controller>
{
public:
	ControllerUI(Controller * c);
	~ControllerUI();

	std::unique_ptr<TriggerImageUI> inActivityUI;
	std::unique_ptr<TriggerImageUI> outActivityUI;

	void resizedInternalHeader(Rectangle<int> &r) override;

};
