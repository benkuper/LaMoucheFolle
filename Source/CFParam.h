/*
  ==============================================================================

    CFParam.h
    Created: 19 Jun 2018 9:30:25am
    Author:  Ben

  ==============================================================================
*/


/*
#pragma once
#include "JuceHeader.h"
#include "crtp.h"


class CFParam
{
public:
	enum Type {
		Uint8 = 0x00 | (0x00 << 2) | (0x01 << 3),
		Int8 = 0x00 | (0x00 << 2) | (0x00 << 3),
		Uint16 = 0x01 | (0x00 << 2) | (0x01 << 3),
		Int16 = 0x01 | (0x00 << 2) | (0x00 << 3),
		Uint32 = 0x02 | (0x00 << 2) | (0x01 << 3),
		Int32 = 0x02 | (0x00 << 2) | (0x00 << 3),
		Float = 0x02 | (0x01 << 2) | (0x00 << 3),
		TYPE_MAX
	};


	class Definition
	{
	public:
		Definition(StringRef name, Type type, uint8 id) : name(name), type(type), id(id) 
		{
			StringArray pSplit;
			pSplit.addTokens(name, ".", "\"");
			group = pSplit[0];
			localName = pSplit[1];
		}

		String group;
		String localName;

		String name;
		Type type;
		uint8 id;
	};

	CFParam(StringRef name, Type type, int id, var value = var());
	CFParam(Definition def, var value = var());
	~CFParam();

	Definition definition;
	var value;

	const static String getTypeString(Type type) {
		switch (type)
		{
		case Int8: return "int8";
		case Uint8: return "uint8";
		case Uint16: return "uint16";
		case Int16: return "int16";
		case Uint32: return "uint32";
		case Int32: return "int32";
		case Float: return "float";
		default: return "unknown";
		}
	}
};

class CFParamToc
{
public:
	CFParamToc(int crc = -1, int numParams = -1);
	~CFParamToc();

	int crc;
	int numParams;

	bool isInitialized();

	void addParamDef(const String &name, uint8 id, CFParam::Type type);

	void save();

	OwnedArray<CFParam> params;
	HashMap<String, CFParam *> paramNamesMap;
	HashMap<uint8, CFParam *> paramIdsMap;

	var getParamValue(const String &name);
	var getParamValue(uint8 id);
	int getParamIdForName(const String &name);
	String getParamNameForId(uint8  id) const;
	CFParam * getParam(const String &name);
	CFParam * getParam(uint8 id);

	Array<uint8> getMissingIds();

	static OwnedArray<CFParamToc> tocs;
	static HashMap<int, CFParamToc *> tocCrcMap;
	static bool paramTocsAreLoaded;

	static void loadParamTocs();
	static CFParamToc * getParamToc(int crc);
	static CFParamToc * addParamToc(int crc, int size);
};

*/