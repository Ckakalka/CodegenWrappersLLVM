
#include "CodeGenerator.h"

#include <fcntl.h>
#include <iostream>
#include <llvm/IR/DebugInfo.h>
#include <sys/mman.h>
#include <unistd.h>

CodeGenerator::CodeGenerator(llvm::Module *module, ConfigWalker &configWalker,
							 std::unordered_set<std::string> &classesForGen, std::string &outputFilename)
	: module(module), configWalker(configWalker), classesForGen(classesForGen), outputFilename(outputFilename) {}

void CodeGenerator::processModule() {
	llvm::DebugInfoFinder Finder;
	Finder.processModule(*module);

	for (const llvm::DIType *type : Finder.types()) {
		if (llvm::DICompositeType::classof(type)) {
			const auto *cmpstType = reinterpret_cast<const llvm::DICompositeType *>(type);
			if (cmpstType->getTag() == llvm::dwarf::DW_TAG_structure_type ||
				cmpstType->getTag() == llvm::dwarf::DW_TAG_class_type) {
				if (!(cmpstType->getName().empty() ||
					  (classesForGen.find(cmpstType->getName().str()) == classesForGen.end()))) {
					cmpstTypes.push_back(cmpstType);
					llvm::DINodeArray elements = cmpstType->getElements();
					std::vector<const llvm::DIType *> validFields;
					for (auto el : elements) {
						if (llvm::DIType::classof(el)) {
							const auto *element = reinterpret_cast<const llvm::DIType *>(el);
							if (!element->getName().empty()) {
								validFields.push_back(element);
							}
						}
					}
					fieldsOfCmpstTypes.push_back(validFields);
				}
			}
		}
	}
}

void CodeGenerator::blockPassing(const std::function<void(int)> &action) {
	size_t result = 0;
	for (int typeNumber = 0; typeNumber < cmpstTypes.size(); ++typeNumber) {
		int blockNumber													  = 0;
		configWalker.realMacroSubstitutions[MacroSubstitution::ClassName] = cmpstTypes[typeNumber]->getName().data();
		while (blockNumber < configWalker.blocks.size()) {
			while (blockNumber < configWalker.blocks.size() &&
				   configWalker.blocks[blockNumber].getName() == BlockHeader::Class) {
				action(blockNumber);
				++blockNumber;
			}
			while (blockNumber < configWalker.blocks.size() &&
				   configWalker.blocks[blockNumber].getName() == BlockHeader::Method) {
				int fieldNumber = 0;
				for (const llvm::DIType *field : fieldsOfCmpstTypes[typeNumber]) {
					configWalker.realMacroSubstitutions[MacroSubstitution::FieldName] = field->getName().data();
					configWalker.realMacroSubstitutions[MacroSubstitution::FieldNumber] =
						std::to_string(fieldNumber).c_str();
					action(blockNumber);
					++fieldNumber;
				}
				++blockNumber;
			}
		}
	}
}

void CodeGenerator::generate() {
	processModule();
	size_t totalSize   = 0;
	auto lengthCounter = [this, &totalSize](int blockNumber) {
		totalSize += this->configWalker.blocks[blockNumber].calcOutputLength();
	};
	blockPassing(lengthCounter);
	int fd;
	// TODO FileMapper
	// TODO Trunc
	if ((fd = open(outputFilename.c_str(), O_CREAT | O_RDWR)) < 0) {
		std::cerr << "Error: can\'t open file " << outputFilename << " " << strerror(errno) << ".\n";
		return;
	}
	//	/* find size of input file */
	//	if (fstat (fdin,&statbuf) < 0)
	//	{printf ("fstat error");
	//		return 0;
	//	}
	//
	if (ftruncate(fd, (off_t)totalSize) == -1) {
		std::cerr << "Error: can\'t truncate file " << outputFilename << " " << strerror(errno) << ".\n";
		close(fd);
		return;
	}
	char *startFile;
	if ((startFile = (char *)mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		std::cerr << "Error: can\'t mmap " << outputFilename << " " << strerror(errno) << ".\n";
		close(fd);
		return;
	}
	close(fd);
	auto codeWriter = [this, startFile](int blockNumber) mutable {
		startFile = this->configWalker.blocks[blockNumber].writeToMemory(startFile);
	};
	blockPassing(codeWriter);

	for (auto &cmpstType : cmpstTypes) {
		classesForGen.erase(cmpstType->getName().str());
	}

	for (auto &classForGen : classesForGen) {
		std::cout << "Warning: class/structure " << classForGen << " not found.\n";
	}

	if (munmap(startFile, totalSize) < 0) {
		std::cerr << "Error: can\'t munmap " << outputFilename << " " << strerror(errno) << ".\n";
		return;
	}
}
