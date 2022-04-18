
#ifndef PROGRAM_CODEGENERATOR_H
#define PROGRAM_CODEGENERATOR_H


#include <memory>
#include <llvm/IR/Module.h>
#include "../ConfigWalker.h"
class CodeGenerator {
public:
	CodeGenerator(std::unique_ptr<llvm::Module> module, ConfigWalker &configWalker);
};


#endif	// PROGRAM_CODEGENERATOR_H
