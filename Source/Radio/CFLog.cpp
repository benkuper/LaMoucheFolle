/*
  ==============================================================================

	CFLog.cpp
	Created: 19 Jun 2018 9:30:28am
	Author:  Ben

  ==============================================================================
*/


#include "CFLog.h"

OwnedArray<CFLogToc> CFLogToc::tocs;
HashMap<int, CFLogToc *> CFLogToc::tocCrcMap;
bool CFLogToc::logTocsAreLoaded = false;

CFLogToc::CFLogToc(int crc, int numVariables) :
	crc(crc),
	numVariables(numVariables)
{
}

bool CFLogToc::isInitialized()
{
	return variables.size() > 0 && variables.size() == numVariables;
}


void CFLogToc::addVariableDef(const String & name, int id, CFLogVariable::Type type)
{
	CFLogVariable* v = getLogVariable(id);
	if(v != nullptr)
	{
		LOGWARNING("Weird , variable with same name but different id exists : " << name << " with id " << v->definition.id << ", new id is " << id);
	} 

	v = new CFLogVariable(CFLogVariable::Definition(name, type, id));
	variables.add(v);
	variableIdsMap.set(id, v);
	variableNamesMap.set(name, v);

}

void CFLogToc::save()
{
	var data = var();
	for (int i = 0; i < numVariables; i++)
	{
		var pData = var(new DynamicObject());
		pData.getDynamicObject()->setProperty("name", variables[i]->definition.name);
		pData.getDynamicObject()->setProperty("id", variables[i]->definition.id);
		pData.getDynamicObject()->setProperty("type", variables[i]->definition.type);
		data.append(pData);
	}

	File tocDir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(String(ProjectInfo::projectName) + "/logTocs");
	if (!tocDir.exists()) tocDir.createDirectory();


	File f = tocDir.getChildFile(String(crc) + ".json");
	if (f.existsAsFile()) f.deleteFile();
	FileOutputStream fs(f);
	JSON::writeToStream(fs, data);

	LOG("Log TOC saved to " << f.getFullPathName());
}


var CFLogToc::getLogVariableValue(const String &name)
{
	if (variableNamesMap.contains(name)) return variableNamesMap[name]->value;
	return -1;
}

var CFLogToc::getLogVariableValue(int id)
{
	if (variableIdsMap.contains(id)) return variableIdsMap[id]->value;
	return -1;
}

int CFLogToc::getLogVariableIdForName(const String &name)
{
	if (variableNamesMap.contains(name)) return variableNamesMap[name]->definition.id;
	return -1;
}

String CFLogToc::getLogVariableNameForId(int id) const
{
	if (variableIdsMap.contains(id)) return variableIdsMap[id]->definition.name;
	return "[notset]";
}

CFLogVariable * CFLogToc::getLogVariable(const String &name)
{
	if (variableNamesMap.contains(name)) return variableNamesMap[name];
	return nullptr;
}

CFLogVariable * CFLogToc::getLogVariable(int id)
{
	if (variableIdsMap.contains(id)) return variableIdsMap[id];
	return nullptr;
}

Array<int> CFLogToc::getMissingIds()
{
	Array<int> result;
	for (int i = 0; i < numVariables; i++) if (!variableIdsMap.contains(i)) result.add(i);
	return result;
}


void CFLogToc::loadLogTocs()
{
	//load files here
	File tocDir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(String(ProjectInfo::projectName) + "/logTocs");
	if (!tocDir.exists()) tocDir.createDirectory();

	Array<File> tocFiles = tocDir.findChildFiles(File::TypesOfFileToFind::findFiles, false);

	for (auto &f : tocFiles)
	{
		FileInputStream fs(f);
		String fString = fs.readString();
		var data = JSON::fromString(fString);

		if (!data.isVoid())
		{
			CFLogToc * toc = addLogToc(f.getFileNameWithoutExtension().getIntValue(), data.size());

			for (int i = 0; i < data.size(); i++)
			{
				toc->addVariableDef(data[i].getProperty("name", "[notset]"), (int)data[i].getProperty("id", 0), (CFLogVariable::Type)(int)data[i].getProperty("type", 0));
			}

		} else
		{
			DBG("Wrong format !");
		}
	}

	LOG(tocs.size() << " Param TOCs Loaded");

	logTocsAreLoaded = true;
}

CFLogToc * CFLogToc::getLogToc(int crc)
{
	if (!tocCrcMap.contains(crc)) return nullptr;
	return tocCrcMap[crc];
}

CFLogToc * CFLogToc::addLogToc(int crc, int size)
{
	CFLogToc * result = getLogToc(crc);
	if (result != nullptr) return result;

	result = new CFLogToc(crc, size);
	tocs.add(result);
	tocCrcMap.set(crc, result);
	return result;
}

CFLogVariable::CFLogVariable(StringRef name, Type type, int id, var value) :
	CFLogVariable(Definition(name, type, id), value)
{
}

CFLogVariable::CFLogVariable(Definition def, var value) :
	definition(def),
	value(value)
{
}

CFLogVariable::~CFLogVariable()
{
}