/*
  ==============================================================================

    DroneDefs.h
    Created: 25 Nov 2019 10:41:35pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

// BLOCK DATA HOLDERS
#if _WIN32
#define __attribute__(x) 
#pragma pack(push,1)
#endif

struct BatteryBlock
{
	uint8_t battery;
	float voltage;
	uint8_t charging;
} __attribute__((packed));

struct PosBlock
{
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
} __attribute__((packed));


struct CalibBlock
{
	float varianceX;
	float varianceY;
	float varianceZ;
} __attribute__((packed));


#ifdef _WIN32
#pragma pack(pop)
#endif
