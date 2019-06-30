/*
  ==============================================================================

    CFParam.cpp
    Created: 19 Jun 2018 9:30:25am
    Author:  Ben

  ==============================================================================
*/

/*
#include "CFParam.h"

OwnedArray<CFParamToc> CFParamToc::tocs;
HashMap<int, CFParamToc *> CFParamToc::tocCrcMap;
bool CFParamToc::paramTocsAreLoaded = false;

CFParamToc::CFParamToc(int crc, int numParams) :
	crc(crc),
	numParams(numParams)
{
}

CFParamToc::~CFParamToc()
{
}

bool CFParamToc::isInitialized()
{
	return numParams > 0 && params.size() == numParams;
}

void CFParamToc::addParamDef(const String & name, uint8 id, CFParam::Type type)
{
	if (getParam(name) != nullptr)
	{
		DBG("WEIRD : Param " << name << " already exists in this TOC");
		return;
	}

	CFParam * p = new CFParam(CFParam::Definition(name, type, id));
	params.add(p);
	paramIdsMap.set(id, p);
	paramNamesMap.set(name, p);
}

void CFParamToc::save()
{
	var data = var();
	for (int i = 0; i < numParams; i++)
	{
		var pData = var(new DynamicObject());
		pData.getDynamicObject()->setProperty("name", params[i]->definition.name);
		pData.getDynamicObject()->setProperty("id", params[i]->definition.id);
		pData.getDynamicObject()->setProperty("type", params[i]->definition.type);
		data.append(pData);
	}

	File tocDir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(String(ProjectInfo::projectName) + "/paramTocs");
	if (!tocDir.exists()) tocDir.createDirectory();


	File f = tocDir.getChildFile(String(crc) + ".json");
	FileOutputStream fs(f);
	JSON::writeToStream(fs, data);

	LOG("Param TOC saved to " << f.getFullPathName());
}


var CFParamToc::getParamValue(const String &name)
{
	if (paramNamesMap.contains(name)) return paramNamesMap[name]->value;
	//DBG("Could not find parameter with name " << name << " in this TOC");
	return -1;
}

var CFParamToc::getParamValue(uint8 id)
{
	if (paramIdsMap.contains(id)) return paramIdsMap[id]->value;
	//DBG("Could not find parameter with id " << id << " in this TOC");
	return -1;
}

int CFParamToc::getParamIdForName(const String &name)
{
	if (paramNamesMap.contains(name)) return paramNamesMap[name]->definition.id;
	//DBG("Could not find parameter with name " << name << " in this TOC");
	return -1;
}

String CFParamToc::getParamNameForId(uint8 id) const
{
	if (paramIdsMap.contains(id)) return paramIdsMap[id]->definition.name;
	//DBG("Could not find parameter with id " << id << " in this TOC");
	return "[notset]";
}

CFParam * CFParamToc::getParam(const String &name)
{
	if (paramNamesMap.contains(name)) return paramNamesMap[name];
	//DBG("Could not find parameter with name " << name << "in this TOC");
	return nullptr;
}

CFParam * CFParamToc::getParam(uint8 id)
{
	if (paramIdsMap.contains(id)) return paramIdsMap[id];
	//DBG("Could not find parameter with id " << id << " in this TOC");
	return nullptr;
}

Array<uint8> CFParamToc::getMissingIds()
{
	Array<uint8> result;
	for (int i = 0; i < numParams; i++) if (!paramIdsMap.contains(i)) result.add(i);
	return result;
}

void CFParamToc::loadParamTocs()
{

	File tocDir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(String(ProjectInfo::projectName) + "/paramTocs");
	if (!tocDir.exists()) tocDir.createDirectory();

	Array<File> tocFiles = tocDir.findChildFiles(File::TypesOfFileToFind::findFiles, false);


	DBG("Found " << tocFiles.size() << " files");
	for (auto &f : tocFiles)
	{
		FileInputStream fs(f);
		DBG("Loading TOC " << f.getFullPathName());

		String fString = fs.readString();
		var data = JSON::fromString(fString);
		
		if (!data.isVoid())
		{
			CFParamToc * toc = addParamToc(f.getFileNameWithoutExtension().getIntValue(), data.size());

			DBG("Found toc with crc " << toc->crc << " and " << toc->numParams << " params");

			for (int i = 0; i < data.size(); i++)
			{
				toc->addParamDef(data[i].getProperty("name", "[notset]"), (int)data[i].getProperty("id", 0), (CFParam::Type)(int)data[i].getProperty("type", 0));
			}

		} else
		{
			DBG("Wrong format !");
		}
	}

	LOG(tocs.size() << " Param TOCs Loaded");
	paramTocsAreLoaded = true;
}

CFParamToc * CFParamToc::getParamToc(int crc)
{
	if (!tocCrcMap.contains(crc)) return nullptr;
	return tocCrcMap[crc];
}

CFParamToc * CFParamToc::addParamToc(int crc, int size)
{
	CFParamToc * result = getParamToc(crc);
	if (result != nullptr) return result;

	result = new CFParamToc(crc, size);
	tocs.add(result);
	tocCrcMap.set(crc, result);

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
*/