#ifndef PROGRAM_MACROSUBSTITUTION_H
#define PROGRAM_MACROSUBSTITUTION_H


#include "Block.h"
class MacroSubstitution {
public:
	enum SubstitutionName { ClassName, FieldName, FieldNumber, Undefined };
	MacroSubstitution() : name(Undefined) {}
	SubstitutionName getName() { return name; }
	void setName(SubstitutionName value) { name = value; }
	const char *getNameCString() { return nameStrings[name]; }
	static constexpr const char *const nameStrings[] = {"ClassName", "FieldName", "FieldNumber", "Undefined"};
	bool isAvailable(BlockHeader::BlockName blockName) {
		return (blockName == BlockHeader::Class && name == ClassName) || (blockName == BlockHeader::Method);
	}

private:
	SubstitutionName name;
};


#endif	// PROGRAM_MACROSUBSTITUTION_H
