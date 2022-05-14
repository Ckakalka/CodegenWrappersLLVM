#include "ClangLLVMIRTranslator.h"

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <iostream>
#include <llvm/Support/Host.h>

ClangLLVMIRTranslator::ClangLLVMIRTranslator(std::string &headerFilename, std::vector<std::string> &includeDirectories,
											 AvailableLanguages language)
	: headerFilename(headerFilename), includeDirs(includeDirectories), language(language),
	  context(new llvm::LLVMContext()) {}

ClangLLVMIRTranslator::~ClangLLVMIRTranslator() { delete context; }

std::unique_ptr<llvm::Module> ClangLLVMIRTranslator::getLLVMModule() {
	clang::CompilerInstance compilerInstance;
	auto &compilerInvocation = compilerInstance.getInvocation();
	// Clang diagnostics
	clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions;
	clang::TextDiagnosticPrinter textDiagPrinter(llvm::outs(), DiagOpts.get());
	clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
	auto *pDiagnosticsEngine = new clang::DiagnosticsEngine(pDiagIDs, DiagOpts, &textDiagPrinter);

	clang::CompilerInvocation::CreateFromArgs(compilerInvocation, llvm::ArrayRef<const char *>(headerFilename.c_str()),
											  *pDiagnosticsEngine);

	for (auto &includeDir : includeDirs) {
		compilerInvocation.HeaderSearchOpts->AddPath(includeDir, clang::frontend::Angled, false, false);
	}

	auto *languageOptions						   = compilerInvocation.getLangOpts();
	languageOptions->CPlusPlus					   = (language == AvailableLanguages::CXX);
	languageOptions->CPlusPlus11				   = (language == AvailableLanguages::CXX);
	languageOptions->CPlusPlus14				   = (language == AvailableLanguages::CXX);
	languageOptions->Bool						   = 1;
	languageOptions->CXXOperatorNames			   = 1;
	languageOptions->WChar						   = 1;
	languageOptions->DoubleSquareBracketAttributes = 1;
	languageOptions->GNUCVersion				   = 1;

	auto &codeGenOptions = compilerInvocation.getCodeGenOpts();
	codeGenOptions.setDebugInfo(clang::codegenoptions::UnusedTypeInfo);

	auto &targetOptions	 = compilerInvocation.getTargetOpts();
	targetOptions.Triple = llvm::sys::getDefaultTargetTriple();

	compilerInstance.createDiagnostics(&textDiagPrinter, false);

	// FIXME EmitLLVMOnlyAction
	std::unique_ptr<clang::CodeGenAction> action = std::make_unique<clang::EmitLLVMAction>(context);

	if (!compilerInstance.ExecuteAction(*action)) {
		std::cout << "Cannot execute action with compiler instance.\n";
	}

	// Runtime LLVM Module
	std::unique_ptr<llvm::Module> module = action->takeModule();
	if (!module) {
		std::cout << "Cannot retrieve IR module.\n";
	}
	return module;
}