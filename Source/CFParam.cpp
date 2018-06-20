/*
  ==============================================================================

    CFParam.cpp
    Created: 19 Jun 2018 9:30:25am
    Author:  Ben

  ==============================================================================
*/

#include "CFParam.h"

juce_ImplementSingleton(CFParamToc)

OwnedArray<CFParamToc> CFParamToc::tocs;
HashMap<int, CFParamToc *> CFParamToc::tocCrcMap;


CFParamToc::CFParamToc(int crc) :
	crc(crc)
{
}

CFParamToc::~CFParamToc()
{
}


var CFParamToc::getParamValue(StringRef name)
{
	if (paramNamesMap.contains(name)) return paramNamesMap[name]->value;
	DBG("Could not find parameter with name " << name << " in this TOC");
	return -1;
}

var CFParamToc::getParamValue(uint8 id)
{
	if (paramIdsMap.contains(id)) return paramIdsMap[id]->value;
	DBG("Could not find parameter with id " << id << " in this TOC");
	return -1;
}

int CFParamToc::getParamIdForName(StringRef name)
{
	if (paramNamesMap.contains(name)) return paramNamesMap[name]->definition.id;
	DBG("Could not find parameter with name " << name << " in this TOC");
	return -1;
}

String CFParamToc::getParamNameForId(uint8 id) const
{
	if (paramIdsMap.contains(id)) return paramIdsMap[id]->definition.name;
	DBG("Could not find parameter with id " << id << " in this TOC");
	return "[notset]";
}

CFParam * CFParamToc::getParam(StringRef name)
{
	if (paramNamesMap.contains(name)) return paramNamesMap[name];
	DBG("Could not find parameter with name " << name << "in this TOC");
	return nullptr;
}

CFParam * CFParamToc::getParam(uint8 id)
{
	if (paramIdsMap.contains(id)) return paramIdsMap[id];
	DBG("Could not find parameter with id " << id << " in this TOC");
	return nullptr;
}

void CFParamToc::loadParamTocs()
{
	//load files here
}

CFParamToc * CFParamToc::getParamToc(int crc)
{
	if (!tocCrcMap.contains(crc)) return nullptr;
	return tocCrcMap[crc];
}

CFParamToc * CFParamToc::addParamToc(int crc)
{
	CFParamToc * result = getParamToc(crc);
	if (result != nullptr) return result;

	result = new CFParamToc(crc);
	return result;
}


//PARAM

CFParam::CFParam(StringRef name, Type type, int id, var value) :
	CFParam(Definition(name,type,id), value)
{
}

CFParam::CFParam(Definition def, var value) :
	definition(def),
	value(value)
{
}

CFParam::~CFParam()
{
}
