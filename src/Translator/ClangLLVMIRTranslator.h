#ifndef PROGRAM_CLANGLLVMIRTRANSLATOR_H
#define PROGRAM_CLANGLLVMIRTRANSLATOR_H


#include "../utils.h"
#include "Translator.h"


class ClangLLVMIRTranslator : public Translator {
public:
	explicit ClangLLVMIRTranslator(std::string &headerFilename, std::vector<std::string> &includeDirectories,
								   AvailableLanguages language);
	std::unique_ptr<llvm::Module> getLLVMModule() override;
	~ClangLLVMIRTranslator() override;

private:
	std::string headerFilename;
	std::vector<std::string> includeDirs;
	AvailableLanguages language;
	llvm::LLVMContext *context;
};


#endif	// PROGRAM_CLANGLLVMIRTRANSLATOR_H
