
#ifndef PROGRAM_CODEGENERATOR_H
#define PROGRAM_CODEGENERATOR_H


#include "../ConfigWalker/ConfigWalker.h"

#include <llvm/IR/Module.h>
#include <memory>
#include <unordered_set>
class CodeGenerator {
public:
	CodeGenerator(llvm::Module *module, ConfigWalker &configWalker, std::unordered_set<std::string> &classesForGen,
				  std::string &outputFilename);
	void generate();

private:
	void processModule();
	void blockPassing(const std::function<void(int)> &action);
	llvm::Module *module;
	ConfigWalker &configWalker;
	std::unordered_set<std::string> classesForGen;
	std::string outputFilename;
	// Classes and structures
	std::vector<const llvm::DICompositeType *> cmpstTypes;
	std::vector<std::vector<const llvm::DIType *>> fieldsOfCmpstTypes;
};


#endif	// PROGRAM_CODEGENERATOR_H
