#include "Block.h"

#include <cstring>
#include <iostream>

constexpr const char *BlockHeader::nameStrings[];

BlockBody::BlockBody(BlockHeader::BlockName name) : name(name) {}

BlockHeader::BlockName BlockBody::getName() { return name; }

size_t BlockBody::calcOutputLength() {
	size_t result = 0;
	for (auto &commonString : commonStrings) {
		result += commonString.second;
	}
	for (auto &macroSubstitution : macroSubstitutions) {
		result += strlen(*macroSubstitution.second);
	}
	// +1 for \n
	return result + 1;
}

char *BlockBody::writeToMemory(char *ptrToMem) {
	int cmnStrIdx = 0;
	int macroIdx  = 0;
	while (cmnStrIdx < commonStrings.size()) {
		while (macroIdx < macroSubstitutions.size() && macroSubstitutions[macroIdx].first <= cmnStrIdx) {
			std::memcpy(ptrToMem, *macroSubstitutions[macroIdx].second, strlen(*macroSubstitutions[macroIdx].second));
			ptrToMem += strlen(*macroSubstitutions[macroIdx].second);
			++macroIdx;
		}
		std::memcpy(ptrToMem, commonStrings[cmnStrIdx].first, commonStrings[cmnStrIdx].second);
		ptrToMem += commonStrings[cmnStrIdx].second;
		++cmnStrIdx;
	}
	// If macroSubstitution in the end of block
	while (macroIdx < macroSubstitutions.size() && macroSubstitutions[macroIdx].first <= cmnStrIdx) {
		std::memcpy(ptrToMem, *macroSubstitutions[macroIdx].second, strlen(*macroSubstitutions[macroIdx].second));
		ptrToMem += strlen(*macroSubstitutions[macroIdx].second);
		++macroIdx;
	}
	*ptrToMem = '\n';
	++ptrToMem;
	return ptrToMem;
}
