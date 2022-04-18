#include "ClangLLVMIRTranslator/ClangLLVMIRTranslator.h"
#include "ConfigWalker.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "CodeGenerator/CodeGenerator.h"

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Sema/Sema.h>
#include <fcntl.h>
#include <iostream>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	const char *configFilename =
		"/media/valeriy/cbbdec04-2749-4d0d-9d0a-bb4c4eaa0f68/Documents/4_course/Diplom/Program/config.txt";
	int fd;
	struct stat statbuf;
	char *startFile;
	char *ptr;

	if ((fd = open(configFilename, O_RDONLY)) < 0) {
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
	configWalker.walk();

	ClangLLVMIRTranslator clangTranslator;

	CodeGenerator codeGenerator(clangTranslator.getLLVMModule(), configWalker);

	if (munmap(startFile, statbuf.st_size) < 0) {
		std::cout << "Can\'t unmap file\n";
		return 0;
	}
	return 0;
}
