#include "Block.h"

#include <cstring>
#include <iostream>

constexpr const char *BlockHeader::nameStrings[];

Block::Block()= default;

void Block::printStrings() {
	size_t totalSize = 0;
	for (auto &commonString : commonStrings) {
		totalSize += commonString.second;
	}
	for (auto &macroSubstitution : macroSubstitutions) {
		totalSize += strlen(*macroSubstitution.second);
	}
	char *totalString = new char[totalSize + 1];
	char *currentPtr  = totalString;
	int i = 0;
	int j = 0;
	while (i < commonStrings.size()) {
		while (j < macroSubstitutions.size() && macroSubstitutions[j].first <= i) {
			std::memcpy(currentPtr, *macroSubstitutions[j].second, strlen(*macroSubstitutions[j].second));
			currentPtr += strlen(*macroSubstitutions[j].second);
			++j;
		}
		std::memcpy(currentPtr, commonStrings[i].first, commonStrings[i].second);
		currentPtr += commonStrings[i].second;
		++i;
	}
	// If macroSubstitution in the end of block
	while (j < macroSubstitutions.size() && macroSubstitutions[j].first <= i) {
		std::memcpy(currentPtr, *macroSubstitutions[j].second, strlen(*macroSubstitutions[j].second));
		currentPtr += strlen(*macroSubstitutions[j].second);
		++j;
	}
	totalString[totalSize] = '\0';
	std::cout << totalString;
	delete[] totalString;
}
