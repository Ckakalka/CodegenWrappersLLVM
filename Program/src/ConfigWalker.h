
#ifndef PROGRAM_CONFIGWALKER_H
#define PROGRAM_CONFIGWALKER_H


#include "Block.h"
#include "MacroSubstitution.h"

#include <cstdio>
#include <vector>

class ConfigWalker {
public:
	ConfigWalker(const char *startOfFile, off_t length);
	static bool strCmpr(const char *first, off_t sizeFirst, const char *second, off_t sizeSecond);
	bool goToNextBlock();
	inline bool isEndOfFile() const { return position >= length; }
	bool findBlockHeader();
	bool findMacroSubstitution(int positionBeforeCommonStrings);
	bool fillBlock();
	void walk();

// FIXME
public:
	static BlockHeader getBlockHeader(const char *headerPtr, off_t headerSize);
	static MacroSubstitution getMacroSubstitution(const char *macroPtr, off_t macroSize);
	const char *startOfFile;
	off_t position;
	off_t length;
	BlockHeader lastHeader;
	std::vector<Block> blocks;
	const char **realMacroSubstitutions;
};


#endif	// PROGRAM_CONFIGWALKER_H
