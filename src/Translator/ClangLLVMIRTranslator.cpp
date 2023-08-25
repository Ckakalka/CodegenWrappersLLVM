#include "ClangLLVMIRTranslator.h"

#include "TranslatorException.h"

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
	clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagOpts = new clang::DiagnosticOptions;
	clang::TextDiagnosticPrinter textDiagPrinter(llvm::outs(), diagOpts.get());
	clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagIDs;
	auto *diagnosticsEngine = new clang::DiagnosticsEngine(diagIDs, diagOpts, &textDiagPrinter);

	std::vector<const char *> args({headerFilename.c_str()});
	llvm::ArrayRef<const char *> argsRef(args);
	clang::CompilerInvocation::CreateFromArgs(compilerInvocation, argsRef, *diagnosticsEngine);
	compilerInvocation.DiagnosticOpts->IgnoreWarnings = 1;
	for (auto &includeDir : includeDirs) {
		compilerInvocation.HeaderSearchOpts->AddPath(includeDir, clang::frontend::Angled, false, false);
	}

	auto *languageOptions						   = compilerInvocation.getLangOpts();
	languageOptions->CPlusPlus					   = (language == AvailableLanguages::CXX);
	languageOptions->CPlusPlus11				   = (language == AvailableLanguages::CXX);
	languageOptions->CPlusPlus14				   = (language == AvailableLanguages::CXX);
	languageOptions->Bool						   = (language == AvailableLanguages::CXX);
	languageOptions->CXXOperatorNames			   = (language == AvailableLanguages::CXX);
	languageOptions->WChar						   = (language == AvailableLanguages::CXX);
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
		throw TranslatorException("Cannot execute action with compiler instance.");
	}

	std::unique_ptr<llvm::Module> module = action->takeModule();
	if (!module) {
		throw TranslatorException("Cannot retrieve IR module.");
	}
	return module;
}