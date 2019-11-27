/*
  ==============================================================================

    CFAssetManager.cpp
    Created: 25 Oct 2017 11:24:57am
    Author:  Ben

  ==============================================================================
*/

#include "CFAssetManager.h"


juce_ImplementSingleton(CFAssetManager);

CFAssetManager::CFAssetManager()
{
}

CFAssetManager::~CFAssetManager()
{

}

ImageButton * CFAssetManager::getSetupBTImage(const Image & image)
{
	ImageButton * bt = new ImageButton();
	bt->setImages(false, true, true,
		image, 0.7f, Colours::transparentBlack,
		image, 1.0f, Colours::transparentBlack,
		image, 1.0f, Colours::white.withAlpha(.7f), 0);
	return bt;
}

ImageButton * CFAssetManager::getToggleBTImage(const Image & image)
{
	ImageButton * bt = new ImageButton();
	Image offImage = image.createCopy();
	offImage.desaturate();

	bt->setImages(false, true, true,
		offImage, 0.5f, Colours::transparentBlack,
		offImage, 1.0f, Colours::white.withAlpha(.2f),
		image, 1.0f, Colours::transparentBlack, 0);
	return bt;
}
