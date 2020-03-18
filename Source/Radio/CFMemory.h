/*
  ==============================================================================

    CFMemory.h
    Created: 13 Jan 2020 8:41:10pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once
/*
#include "JuceHeader.h"

class CFMemory
{
public:
	enum Type {
		MemoryTypeEEPROM = 0x00,
		MemoryTypeOneWire = 0x01,
		MemoryTypeLED12 = 0x10,
		MemoryTypeLOCO = 0x11,
		MemoryTypeTRAJ = 0x12,
		MemoryTypeLOCO2 = 0x13,
		MemoryTypeLH = 0x14,
		MemoryTypeTester = 0x15,
		MemoryTypeUSD = 0x16, // Crazyswarm experimental
		TYPE_MAX
	};


	class Definition
	{
	public:
		Definition(int64 address, Type type, int size) : address(address), type(type), size(size) {}
		int64 address;
		Type type;
		int size;
	};

	CFMemory(int64 address, Type type, int size, Array<uint8> data = Array<uint8>());
	CFMemory(Definition def, Array<uint8> data = Array<uint8>());
	~CFMemory() {}

	Definition definition;
	Array<uint8> data;

	const static String getTypeString(Type type) {
		switch (type)
		{
		case MemoryTypeEEPROM:return "EEPRom"; 
			case MemoryTypeOneWire:return "OneWire";
			case MemoryTypeLED12:return "LED12";
			case MemoryTypeLOCO:return "LOCO";
			case MemoryTypeTRAJ:return "TRAJ";
			case MemoryTypeLOCO2:return "LOCO2";
			case MemoryTypeLH:return "Lighthouse";
			case MemoryTypeTester:return "Tester";
			case MemoryTypeUSD:return "USD"; // Crazyswarm experimental
		default: return "unknown";
		}
	}

};

class CFMemoryToc
{
public:
	CFMemoryToc(int crc, int numVariables = 0);
	~CFMemoryToc() {}

	int numMemories;
	OwnedArray<CFMemory> memories;

	void addVariableDef(int64 address, CFMemory::Type type);

	Array<uint8> getMemoryData(CFMemory::Type type);

	bool isInitialized();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CFLogToc)
};
*/