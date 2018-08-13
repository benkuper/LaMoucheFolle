/*
  ==============================================================================

    CFLog.h
    Created: 19 Jun 2018 9:30:28am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class CFLogVariable
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
		Definition(StringRef name, Type type, uint8 id) : name(name), type(type), id(id) {}
		String name;
		Type type;
		uint8 id;
	};

	CFLogVariable(StringRef name, Type type, int id, var value = var());
	CFLogVariable(Definition def, var value = var());
	~CFLogVariable();

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

class CFLogToc
{
public:
	CFLogToc(int crc, int numVariables = 0);
	~CFLogToc() {}

	int crc;
	int numVariables;


	void addVariableDef(const String &name, uint8 id, CFLogVariable::Type type);

	void save();

	OwnedArray<CFLogVariable> variables;
	HashMap<String, CFLogVariable *> variableNamesMap;
	HashMap<uint8, CFLogVariable *>variableIdsMap;

	var getLogVariableValue(const String &name);
	var getLogVariableValue(uint8 id);
	int getLogVariableIdForName(const String &name);
	String getLogVariableNameForId(uint8  id) const;
	CFLogVariable * getLogVariable(const String &name);
	CFLogVariable * getLogVariable(uint8 id);

	bool isInitialized();

	static OwnedArray<CFLogToc> tocs;
	static HashMap<int, CFLogToc *> tocCrcMap;
	static bool logTocsAreLoaded;

	static void loadLogTocs();
	static CFLogToc * getLogToc(int crc);
	static CFLogToc * addLogToc(int crc, int size);

};