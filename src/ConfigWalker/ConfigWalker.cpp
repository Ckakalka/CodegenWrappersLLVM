#include "ConfigWalker.h"

#include "ConfigFormatException.h"
#include "MacroSubstitution.h"

#include <cstring>
#include <iostream>

#define currentCharacter (startOfFile[position])

#define returnIfEndOfFile()                                                                                            \
	do {                                                                                                               \
		if (position >= length) {                                                                                      \
			return false;                                                                                              \
		}                                                                                                              \
	} while (0)

#define checkEndOfFile()                                                                                               \
	do {                                                                                                               \
		if (position >= length) {                                                                                      \
			throw ConfigFormatException("End of file\n");                                                              \
		}                                                                                                              \
	} while (0)

ConfigWalker::ConfigWalker(const char *startOfFile, off_t length)
	: startOfFile(startOfFile), position(0), length(length), currentLine(1) {
	realMacroSubstitutions = new const char *[MacroSubstitution::Undefined];
	lastHeader.setOpening(false);
}

ConfigWalker::~ConfigWalker() { delete[] realMacroSubstitutions; }

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
 * Игнорируя пробельные символы и комментарии, переходит к заголовку следующего блока
 * @return * True если успешно, position на начале заголовка блока
 * False если конец файла, position на последнем символе
 * Исключение неожиданный символ, position на неверном символе
 */
bool ConfigWalker::goToNextBlock() {
	returnIfEndOfFile();
	while (true) {
		// skip whitespace characters
		while ((currentCharacter == '\n') || (currentCharacter == '\r') || (currentCharacter == '\t') ||
			   (currentCharacter == ' ')) {
			if (currentCharacter == '\n') {
				++currentLine;
			}
			++position;
			returnIfEndOfFile();
		}
		// skip comments
		if (currentCharacter == '#') {
			++position;
			returnIfEndOfFile();
			// skip comment line
			while (currentCharacter != '\n') {
				++position;
				returnIfEndOfFile();
			}
			++currentLine;
		} else {
			if (currentCharacter == '<') {
				return true;
			} else {
				// Exception
				std::cout << "Unexpected symbol " << currentCharacter << ". Expected <\n";
				return false;
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

// false если конец файла
// исключения
void ConfigWalker::checkMacroSubstitution(BlockBody &blockBody, int positionBeforeCommonStrings) {
	++position;
	checkEndOfFile();
	// исключение
	if (currentCharacter != '{') {
		//		std::cout << "Met " << currentCharacter << ". But character { expected\n";
		throw ConfigFormatException("Unexpected character\n");
	}
	// character '{'
	off_t startPosition = position;
	// Go to character '}'
	while (currentCharacter != '}') {
		++position;
		checkEndOfFile();
	}
	MacroSubstitution macro = getMacroSubstitution(startOfFile + startPosition + 1, position - startPosition - 1);

	// исключение
	if (macro.getName() == MacroSubstitution::Undefined) {
		throw ConfigFormatException("Undefined MacroSubstitution\n");
	}
	// исключение
	if (!macro.isAvailable(lastHeader.getBlockName())) {
		throw ConfigFormatException("Not available MacroSubstitution\n");
	}
	std::cout << "Found MacroSubstitution = " << macro.getNameCString() << "\n";
	blockBody.macroSubstitutions.emplace_back(
		std::make_pair(positionBeforeCommonStrings, &realMacroSubstitutions[macro.getName()]));
}

/***
 * 		   Исключение
 */
void ConfigWalker::checkBlockHeader() {
	// Character '<'
	off_t startPosition = position;
	// Go to character '>'
	while (currentCharacter != '>') {
		if (currentCharacter == '\n') {
			++currentLine;
		}
		++position;
		checkEndOfFile();
	}

	BlockHeader header = getBlockHeader(startOfFile + startPosition + 1, position - startPosition - 1);

	// exception
	if (header.getBlockName() == BlockHeader::Undefined) {
		throw ConfigFormatException("Undefined block\n");
	}
	// exception
	if (!BlockHeader::isMatch(lastHeader, header)) {
		throw ConfigFormatException("Unmatched blocks\n");
	}
	std::cout << "Found block opening = " << header.isOpening() << " " << header.getBlockNameCString() << "\n";
	lastHeader = header;
	++position;
}

void ConfigWalker::blockBodyProcessing() {
	BlockBody blockBody(lastHeader.getBlockName());
	checkEndOfFile();

	// Skip spaces characters except new line
	while (currentCharacter == '\r' || currentCharacter == '\t' || currentCharacter == ' ') {
		++position;
		checkEndOfFile();
	}
	// исключение
	if (currentCharacter != '\n') {
		throw ConfigFormatException("Expected new line\n");
	}
	++currentLine;
	++position;
	off_t startCommonString			= position;
	int positionBeforeCommonStrings = 0;
	while (true) {
		checkEndOfFile();
		switch (currentCharacter) {
			case '\\':
				++position;
				checkEndOfFile();
				//			if (returnIfEndOfFile()) {
				//				std::cout << "Expected \\, $ or < but reached end of file\n";
				//				return false;
				//			}
				// Exception
				if (!(currentCharacter == '\\' || currentCharacter == '$' || currentCharacter == '<')) {
					//					std::cout << "Expected \\, $ or < but found " << currentCharacter << "\n";
					throw ConfigFormatException("Expected \\, $ or <\n");
				} else {
					// Shielding
					// запоминаем пройденную строку за исключением слеша
					blockBody.commonStrings.emplace_back(startOfFile + startCommonString,
														 position - startCommonString - 1);
					++positionBeforeCommonStrings;
					startCommonString = position;
					std::cout << "Found shielding\n";
					++position;
				}
				break;
			case '$':
				// MacroSubstitution
				blockBody.commonStrings.emplace_back(startOfFile + startCommonString, position - startCommonString);
				++positionBeforeCommonStrings;
				checkMacroSubstitution(blockBody, positionBeforeCommonStrings);
				++position;
				startCommonString = position;
				break;
			case '<': {
				// End of block
				// Skip spaces characters except new line
				off_t savedPosition = position;
				--position;
				while (currentCharacter == '\r' || currentCharacter == '\t' || currentCharacter == ' ') {
					--position;
				}
				// exception
				if (currentCharacter != '\n') {
					throw ConfigFormatException("Expected new line\n");
				}
				if (position > startCommonString) {
					blockBody.commonStrings.emplace_back(startOfFile + startCommonString, position - startCommonString);
					++positionBeforeCommonStrings;
				}
				position = savedPosition;
				blocks.emplace_back(blockBody);
				return;
			}
			case '\n':
				++currentLine;
				++position;
				break;
			default: ++position; break;
		}
	}
}

void ConfigWalker::walk() {
	while (goToNextBlock()) {
		checkBlockHeader();
		blockBodyProcessing();
		checkBlockHeader();
	}
	std::cout << "Found " << blocks.size() << " blocks\n";
}
