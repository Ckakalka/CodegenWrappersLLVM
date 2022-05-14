
#ifndef PROGRAM_BLOCK_H
#define PROGRAM_BLOCK_H

#include <cstdio>
#include <vector>

class BlockHeader {
public:
	enum BlockName { Class, Method, Undefined };
	BlockHeader() : name(Undefined), _isOpening(true) {}
	BlockName getBlockName() { return name; }
	void setBlockName(BlockName value) { name = value; }
	const char *getBlockNameCString() { return nameStrings[name]; }
	void setOpening(bool opening) { _isOpening = opening; }
	bool isOpening() const { return _isOpening; }
	static bool isMatch(BlockHeader first, BlockHeader second) {
		return (!first._isOpening && second._isOpening) ||
			   ((first._isOpening && !second._isOpening) && (first.name == second.name));
	}
	static constexpr const char *nameStrings[] = {"class", "method", "undefined"};

private:
	BlockName name;
	bool _isOpening;
};


class BlockBody {
	friend class ConfigWalker;

public:
	explicit BlockBody(BlockHeader::BlockName name);
	char *writeToMemory(char *ptrToMem);
	size_t calcOutputLength();
	BlockHeader::BlockName getName();

private:
	BlockHeader::BlockName name;
	// beginning of string, length of string
	std::vector<std::pair<const char *, off_t>> commonStrings;
	// position before the common string, macroSubstitution(*string)
	std::vector<std::pair<int, const char *const *>> macroSubstitutions;
};


#endif	// PROGRAM_BLOCK_H
