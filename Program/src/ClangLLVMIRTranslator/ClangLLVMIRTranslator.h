#ifndef PROGRAM_CLANGLLVMIRTRANSLATOR_H
#define PROGRAM_CLANGLLVMIRTRANSLATOR_H


#include <llvm/IR/Module.h>
class ClangLLVMIRTranslator {
public:
	ClangLLVMIRTranslator();
	std::unique_ptr<llvm::Module> getLLVMModule();
};


#endif	// PROGRAM_CLANGLLVMIRTRANSLATOR_H
