#include "ClangLLVMIRTranslator.h"

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <iostream>
#include <llvm/Support/Host.h>

ClangLLVMIRTranslator::ClangLLVMIRTranslator() = default;

// FIXME ARGS
std::unique_ptr<llvm::Module> ClangLLVMIRTranslator::getLLVMModule() {
	clang::CompilerInstance compilerInstance;
	auto &compilerInvocation = compilerInstance.getInvocation();

	// Диагностика работы Clang
	clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions;
	clang::TextDiagnosticPrinter *textDiagPrinter = new clang::TextDiagnosticPrinter(llvm::outs(), &*DiagOpts);

	clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;

	clang::DiagnosticsEngine *pDiagnosticsEngine = new clang::DiagnosticsEngine(pDiagIDs, &*DiagOpts, textDiagPrinter);


	// FIXME ARGS
	std::vector<const char *> args = {};


	clang::CompilerInvocation::CreateFromArgs(compilerInvocation, llvm::ArrayRef<const char *>(args),
											  *pDiagnosticsEngine);

	auto *languageOptions			  = compilerInvocation.getLangOpts();
	auto &preprocessorOptions		  = compilerInvocation.getPreprocessorOpts();
	auto &targetOptions				  = compilerInvocation.getTargetOpts();
	auto &frontEndOptions			  = compilerInvocation.getFrontendOpts();
	auto &codeGenOptions			  = compilerInvocation.getCodeGenOpts();
	languageOptions->CPlusPlus		  = 1;
	languageOptions->EmitAllDecls	  = 1;
	languageOptions->DebuggerSupport  = 1;
	languageOptions->ModulesDebugInfo = 1;
	codeGenOptions.DwarfDebugFlags	  = "-g";
	codeGenOptions.setDebugInfo(clang::codegenoptions::UnusedTypeInfo);

	targetOptions.Triple = llvm::sys::getDefaultTargetTriple();
	compilerInstance.createDiagnostics(textDiagPrinter, false);

	std::string file =
		"/media/valeriy/cbbdec04-2749-4d0d-9d0a-bb4c4eaa0f68/Documents/4_course/Diplom/llvm-ir-exploration/main.h";
	llvm::StringRef filename = file;

	frontEndOptions.Inputs.clear();
	frontEndOptions.Inputs.push_back(clang::FrontendInputFile(filename, clang::InputKind(clang::Language::CXX)));

	llvm::LLVMContext *context = new llvm::LLVMContext();
	// FIXME
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