#include "CodeGenerator/CodeGenerator.h"
#include "ConfigWalker/ConfigFormatException.h"
#include "ConfigWalker/ConfigWalker.h"
#include "Translator/ClangLLVMIRTranslator.h"
#include "utils.h"

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Sema/Sema.h>
#include <fcntl.h>
#include <iostream>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

llvm::cl::OptionCategory compilerCategory("Codegenerator Options", "Options for class wrappers codegenerator");

llvm::cl::opt<std::string> configFilename("c", llvm::cl::Required, llvm::cl::desc("Specify config filename"),
										  llvm::cl::value_desc("filename"), llvm::cl::cat(compilerCategory));

llvm::cl::opt<std::string> headerFilename("h", llvm::cl::Required, llvm::cl::desc("Specify header filename"),
										  llvm::cl::value_desc("filename"), llvm::cl::cat(compilerCategory));

llvm::cl::list<std::string> includeDirectories("Idir", llvm::cl::Required,
											   llvm::cl::desc("adds include directory of header files."),
											   llvm::cl::ZeroOrMore);

llvm::cl::opt<AvailableLanguages>
	languageType("l", llvm::cl::Required, llvm::cl::desc("Specify header filename language"),
				 llvm::cl::values(clEnumValN(AvailableLanguages::C, "c", "C language"),
								  clEnumValN(AvailableLanguages::CXX, "cxx", "CXX language")),
				 llvm::cl::cat(compilerCategory));

llvm::cl::opt<std::string> outputFilename("o", llvm::cl::Required, llvm::cl::desc("Specify output filename"),
										  llvm::cl::value_desc("filename"), llvm::cl::cat(compilerCategory));


void printVersion(llvm::raw_ostream &ostream) { ostream << "Version 1.0.0\n"; }

int main(int argc, char **argv) {
	llvm::cl::HideUnrelatedOptions(compilerCategory);
	llvm::cl::SetVersionPrinter(&printVersion);
	llvm::cl::ParseCommandLineOptions(argc, argv);

	// TODO FileMapper
	int fd;
	struct stat statbuf;
	char *startFile;
	char *ptr;

	if ((fd = open(configFilename.c_str(), O_RDONLY)) < 0) {
		std::cout << "Can\'t open file for reading\n";
		return 0;
	}

	if (fstat(fd, &statbuf) < 0) {
		std::cout << "Can\'t fstat file\n";
		close(fd);
		return 0;
	}

	if ((startFile = (char *)mmap(nullptr, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		std::cout << "Can\'t mapped file\n";
		close(fd);
		return 0;
	}
	close(fd);

	off_t length = statbuf.st_size;

	ConfigWalker configWalker(startFile, length);
	try {
		configWalker.walk();
	} catch (ConfigFormatException &exception) {
		std::cout << exception.what();
		// FIXME
		if (munmap(startFile, statbuf.st_size) < 0) {
			std::cout << "Can\'t unmap file\n";
			return 0;
		}
		return 0;
	}

	std::unique_ptr<Translator> translator;
	if (languageType <= AvailableLanguages::CXX) {
		translator = std::make_unique<ClangLLVMIRTranslator>(headerFilename, includeDirectories, languageType);
	} else {
		std::cout << "Unsupported language\n";
		return -1;
	}

	std::unique_ptr<llvm::Module> module = translator->getLLVMModule();

	CodeGenerator codeGenerator(module.get(), configWalker, outputFilename);
	codeGenerator.generate();

	if (munmap(startFile, statbuf.st_size) < 0) {
		std::cout << "Can\'t unmap file\n";
		return 0;
	}
	return 0;
}
