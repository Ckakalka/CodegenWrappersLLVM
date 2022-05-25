#include "CodeGenerator/CodeGenerator.h"
#include "ConfigWalker/ConfigFormatException.h"
#include "ConfigWalker/ConfigWalker.h"
#include "Translator/ClangLLVMIRTranslator.h"
#include "Translator/TranslatorException.h"
#include "utils.h"

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Sema/Sema.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>

llvm::cl::OptionCategory compilerCategory("Codegenerator Options", "Options for class wrappers codegenerator");

llvm::cl::opt<std::string> configFilename("c", llvm::cl::Required, llvm::cl::desc("Specify config filename"),
										  llvm::cl::value_desc("filename"), llvm::cl::cat(compilerCategory));

llvm::cl::opt<std::string> headerFilename("h", llvm::cl::Required, llvm::cl::desc("Specify header filename"),
										  llvm::cl::value_desc("filename"), llvm::cl::cat(compilerCategory));

llvm::cl::opt<std::string> listClassesFilename("classes", llvm::cl::Required,
											   llvm::cl::desc("Specify list of classes filename"),
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


	std::unique_ptr<Translator> translator;
	if (languageType <= AvailableLanguages::CXX) {
		translator = std::make_unique<ClangLLVMIRTranslator>(headerFilename, includeDirectories, languageType);
	} else {
		std::cerr << "Error: unsupported language.\n";
		return 0;
	}

	std::unique_ptr<llvm::Module> module;
	try {
		module = translator->getLLVMModule();
	} catch (TranslatorException &exception) {
		std::cerr << "Error when translating file " << headerFilename << ". " << exception.what() << "\n";
		return 0;
	}


	std::ifstream fileClasses(listClassesFilename);
	if (fileClasses.fail()) {
		std::cerr << "Error: " << listClassesFilename << " " << strerror(errno) << ".\n";
		return 0;
	}

	std::unordered_set<std::string> classesForGen;
	std::string className;
	while (std::getline(fileClasses, className)) {
		classesForGen.insert(className);
	}

	// TODO FileMapper
	int fd;
	if ((fd = open(configFilename.c_str(), O_RDONLY)) < 0) {
		std::cerr << "Error: " << configFilename << " " << strerror(errno) << ".\n";
		return 0;
	}

	struct stat statbuf;
	if (fstat(fd, &statbuf) < 0) {
		std::cerr << "Error: can\'t get information about " << configFilename << strerror(errno) << ".\n";
		close(fd);
		return 0;
	}

	char *startFile;
	off_t length = statbuf.st_size;
	if ((startFile = (char *)mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		std::cerr << "Error: can\'t mmap " << configFilename << " " << strerror(errno) << ".\n";
		close(fd);
		return 0;
	}
	if (close(fd) < 0) {
		std::cerr << "Error: can\'t close " << configFilename << " " << strerror(errno) << ".\n";
		return 0;
	}

	ConfigWalker configWalker(startFile, length);
	try {
		configWalker.walk();
	} catch (ConfigFormatException &exception) {
		std::cerr << "Error: in the configuration file line " << configWalker.getCurrentLine() << ". "
				  << exception.what() << "\n";
		if (munmap(startFile, statbuf.st_size) < 0) {
			std::cerr << "Error: can\'t munmap " << configFilename << " " << strerror(errno) << ".\n";
		}
		return 0;
	}

	CodeGenerator codeGenerator(module.get(), configWalker, classesForGen, outputFilename);
	codeGenerator.generate();

	if (munmap(startFile, statbuf.st_size) < 0) {
		std::cerr << "Error: can\'t munmap " << configFilename << " " << strerror(errno) << ".\n";
	}
	return 0;
}
