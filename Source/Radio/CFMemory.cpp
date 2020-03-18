/*
  ==============================================================================

    CFMemory.cpp
    Created: 13 Jan 2020 8:41:10pm
    Author:  bkupe

  ==============================================================================
*/

/*
#include "CFMemory.h"

CFMemory::CFMemory(int64 address, Type type, int size, Array<uint8> data) :
	CFMemory(Definition(address, type, size), data)
{
}

CFMemory::CFMemory(Definition def, Array<uint8> data) : 
	definition(def),
	data(data)
{
}

Array<uint8> CFMemoryToc::getMemoryData(CFMemory::Type type)
{
	for (int i = 0; i < memories.size(); i++)
	{
		if (memories[i]->definition.type == type) return memories[i]->data;
	}

	jassertfalse; //memorynotfound
	return Array<uint8>();
}

bool CFMemoryToc::isInitialized()
{
	return false;
}
*/