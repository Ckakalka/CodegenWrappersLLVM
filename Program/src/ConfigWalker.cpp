#include "ConfigWalker.h"

#include "MacroSubstitution.h"

#include <cstring>
#include <iostream>

#define currentCharacter (startOfFile[position])

ConfigWalker::ConfigWalker(const char *startOfFile, off_t length)
	: startOfFile(startOfFile), position(0), length(length) {
	realMacroSubstitutions = new const char *[MacroSubstitution::Undefined];
	lastHeader.setOpening(false);
}

bool ConfigWalker::strCmpr(const char *first, off_t sizeFirst, const char *second, off_t sizeSecond) {
	if (sizeFirst != sizeSecond) {
		return false;
	}
	for (off_t i = 0; i < sizeFirst; ++i) {
		if (first[i] != second[i]) {
			return false;
		}
	}
	return true;
}

/**
 * @return * True если успешно, position на начале блока
 * False если конец файла или ошибка, position на последнем символе или на неверном символе
 */
bool ConfigWalker::goToNextBlock() {
	while (true) {
		// skip whitespace characters
		while (!isEndOfFile() && ((currentCharacter == '\n') || (currentCharacter == '\r') ||
								  (currentCharacter == '\t') || (currentCharacter == ' '))) {
			++position;
		}
		if (isEndOfFile()) {
			std::cout << "Reached end of file\n";
			return false;
		}
		// skip comments
		if (currentCharacter == '#') {
			++position;
			// skip comment line
			while (!isEndOfFile() && currentCharacter != '\n') {
				++position;
			}
			if (isEndOfFile()) {
				std::cout << "Reached end of file\n";
				return false;
			}
		} else {
			if (currentCharacter != '<') {
				std::cout << "Unexpected symbol " << currentCharacter << ". Expected <\n";
				return false;
			} else {
				return true;
			}
		}
	}
}

BlockHeader ConfigWalker::getBlockHeader(const char *headerPtr, off_t headerSize) {
	BlockHeader header;
	if (*headerPtr == '\\') {
		++headerPtr;
		--headerSize;
		header.setOpening(false);
	}
	for (int i = 0; i < (sizeof(BlockHeader::nameStrings) / sizeof(BlockHeader::nameStrings[0])) - 1; ++i) {
		if (strCmpr(headerPtr, headerSize, BlockHeader::nameStrings[i], (off_t)strlen(BlockHeader::nameStrings[i]))) {
			header.setBlockName(static_cast<BlockHeader::BlockName>(i));
			return header;
		}
	}
	return header;
}

MacroSubstitution ConfigWalker::getMacroSubstitution(const char *macroPtr, off_t macroSize) {
	MacroSubstitution macro;
	for (int i = 0; i < (sizeof(MacroSubstitution::nameStrings) / sizeof(MacroSubstitution::nameStrings[0])) - 1; ++i) {
		if (strCmpr(macroPtr, macroSize, MacroSubstitution::nameStrings[i],
					(off_t)strlen(MacroSubstitution::nameStrings[i]))) {
			macro.setName(static_cast<MacroSubstitution::SubstitutionName>(i));
			return macro;
		}
	}
	return macro;
}

bool ConfigWalker::findMacroSubstitution(int positionBeforeCommonStrings) {
	++position;
	if (isEndOfFile()) {
		std::cout << "Reached end of file\n";
		return false;
	}
	if (currentCharacter != '{') {
		std::cout << "Met " << currentCharacter << ". But character { expected\n";
		return false;
	}
	// character '{'
	off_t startPosition = position;
	// Go to character '}'
	while (!isEndOfFile() && currentCharacter != '}') {
		++position;
	}
	if (isEndOfFile()) {
		std::cout << "Reached end of file\n";
		return false;
	}
	MacroSubstitution macro = getMacroSubstitution(startOfFile + startPosition + 1, position - startPosition - 1);

	if (macro.getName() == MacroSubstitution::Undefined) {
		std::cout << "Undefined MacroSubstitution\n";
		return false;
	}
	if (!macro.isAvailable(lastHeader.getBlockName())) {
		std::cout << "Not available MacroSubstitution\n";
		return false;
	}
	std::cout << "Found MacroSubstitution = " << macro.getNameCString() << "\n";
	blocks[blocks.size() - 1].macroSubstitutions.emplace_back(
		std::make_pair(positionBeforeCommonStrings, &realMacroSubstitutions[macro.getName()]));
	return true;
}

/***
 * @return True если нашли верный
 * 		   False неверное имя или конец файла
 */
bool ConfigWalker::findBlockHeader() {
	// Character '<'
	off_t startPosition = position;
	// Go to character '>'
	while (!isEndOfFile() && currentCharacter != '>') {
		++position;
	}
	if (isEndOfFile()) {
		std::cout << "Reached end of file\n";
		return false;
	}

	BlockHeader header = getBlockHeader(startOfFile + startPosition + 1, position - startPosition - 1);

	if (header.getBlockName() == BlockHeader::Undefined) {
		std::cout << "Undefined block\n";
		return false;
	}
	if (!BlockHeader::isMatch(lastHeader, header)) {
		std::cout << "Unmatched blocks\n";
		return false;
	}
	std::cout << "Found block opening = " << header.isOpening() << " " << header.getBlockNameCString() << "\n";
	lastHeader = header;
	return true;
}

bool ConfigWalker::fillBlock() {
	++position;

	// Skip spaces characters except new line
	while (!isEndOfFile() && (currentCharacter == '\r' || currentCharacter == '\t' || currentCharacter == ' ')) {
		++position;
	}
	if (isEndOfFile()) {
		std::cout << "Reached end of file\n";
		return false;
	}
	if (currentCharacter != '\n') {
		std::cout << "Expected new line\n";
		return false;
	}
	++position;
	off_t startCommonString			= position;
	int positionBeforeCommonStrings = 0;
	while (!isEndOfFile()) {
		if (currentCharacter == '\\') {
			++position;
			if (isEndOfFile()) {
				std::cout << "Expected \\, $ or < but reached end of file\n";
				return false;
			}
			if (!(currentCharacter == '\\' || currentCharacter == '$' || currentCharacter == '<')) {
				std::cout << "Expected \\, $ or < but found " << currentCharacter << "\n";
				return false;
			} else {
				// Shielding
				// запоминаем пройденную строку за исключением слеша
				blocks[blocks.size() - 1].commonStrings.emplace_back(startOfFile + startCommonString,
																	 position - startCommonString - 1);
				++positionBeforeCommonStrings;
				startCommonString = position;
				std::cout << "Found shielding\n";
				++position;
			}
		} else if (currentCharacter == '$') {
			// MacroSubstitution
			blocks[blocks.size() - 1].commonStrings.emplace_back(startOfFile + startCommonString,
																 position - startCommonString);
			++positionBeforeCommonStrings;
			if (!findMacroSubstitution(positionBeforeCommonStrings)) {
				return false;
			}
			++position;
			startCommonString = position;
		} else if (currentCharacter == '<') {
			// End of block
			// Skip spaces characters except new line
			off_t savedPosition = position;
			--position;
			while (currentCharacter == '\r' || currentCharacter == '\t' || currentCharacter == ' ') {
				--position;
			}
			if (currentCharacter != '\n') {
				std::cout << "Expected new line\n";
				return false;
			}
			if (position > startCommonString) {
				blocks[blocks.size() - 1].commonStrings.emplace_back(startOfFile + startCommonString,
																	 position - startCommonString);
				++positionBeforeCommonStrings;
			}
			position = savedPosition;
			if (!findBlockHeader()) {
				return false;
			}
			++position;
			return true;
		} else {
			++position;
		}
	}
	return true;
}

void ConfigWalker::walk() {
	while (goToNextBlock()) {
		if (!findBlockHeader()) {
			break;
		}
		blocks.emplace_back(Block());
		if (!fillBlock()) {
			break;
		}
	}
	//	for (auto &block : blocks) {
	//		std::cout << "Block\n";
	//		block.printStrings();
	//		std::cout << "\n";
	//	}

	std::cout << "Found " << blocks.size() << " blocks\n";
}
