
#ifndef PROGRAM_CODEGENERATOR_H
#define PROGRAM_CODEGENERATOR_H


#include "../ConfigWalker/ConfigWalker.h"

#include <llvm/IR/Module.h>
#include <memory>
class CodeGenerator {
public:
	CodeGenerator(llvm::Module *module, ConfigWalker &configWalker, std::string &outputFilename);
	void generate();

private:
	void processModule();
	void blockPassing(const std::function<void(int)> &action);
	llvm::Module *module;
	ConfigWalker &configWalker;
	std::string outputFilename;
	// Classes and structures
	std::vector<const llvm::DICompositeType *> cmpstTypes;
	std::vector<std::vector<const llvm::DIType *>> fieldsOfCmpstTypes;
};


#endif	// PROGRAM_CODEGENERATOR_H
