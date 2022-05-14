
#ifndef PROGRAM_CONFIGWALKER_H
#define PROGRAM_CONFIGWALKER_H


#include "Block.h"
#include "MacroSubstitution.h"

#include <vector>

class ConfigWalker {
	friend class CodeGenerator;

public:
	ConfigWalker(const char *startOfFile, off_t length);
	~ConfigWalker();
	static bool strCmpr(const char *first, off_t sizeFirst, const char *second, off_t sizeSecond);
	bool goToNextBlock();
	void checkBlockHeader();
	void checkMacroSubstitution(BlockBody &blockBody, int positionBeforeCommonStrings);
	void blockBodyProcessing();
	void walk();

private:
	static BlockHeader getBlockHeader(const char *headerPtr, off_t headerSize);
	static MacroSubstitution getMacroSubstitution(const char *macroPtr, off_t macroSize);
	const char *startOfFile;
	off_t position;
	off_t length;
	BlockHeader lastHeader;
	std::vector<BlockBody> blocks;
	const char **realMacroSubstitutions;
	off_t currentLine;
};


#endif	// PROGRAM_CONFIGWALKER_H
