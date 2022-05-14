
#ifndef PROGRAM_TRANSLATOR_H
#define PROGRAM_TRANSLATOR_H

#include <llvm/IR/Module.h>
#include <memory>

class Translator {
public:
	virtual std::unique_ptr<llvm::Module> getLLVMModule() = 0;
	virtual ~Translator()								  = default;
};

#endif
