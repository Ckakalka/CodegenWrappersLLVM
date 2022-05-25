#set (search_paths
#        $ENV{PATH_TO_LLVM}
#        $ENV{PATH_TO_LLVM}/lib/cmake
#        $ENV{PATH_TO_LLVM}/lib/cmake/llvm
#        $ENV{PATH_TO_LLVM}/lib/cmake/clang
#        $ENV{PATH_TO_LLVM}/share/clang/cmake/
#        $ENV{PATH_TO_LLVM}/share/llvm/cmake/
#        )
#
#find_package(LLVM REQUIRED CONFIG
#        PATHS ${search_paths}
#        NO_DEFAULT_PATH)
#
#
#
#find_package(Clang REQUIRED CONFIG
#        PATHS ${search_paths}
#        NO_DEFAULT_PATH)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "LLVM_INCLUDE_DIRS=${LLVM_INCLUDE_DIRS}")
#message(STATUS "PATH_TO_LLVM=$ENV{PATH_TO_LLVM}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(Program main.cpp ConfigWalker/ConfigWalker.cpp ConfigWalker/ConfigWalker.h ConfigWalker/Block.cpp ConfigWalker/Block.h ConfigWalker/MacroSubstitution.cpp ConfigWalker/MacroSubstitution.h Translator/ClangLLVMIRTranslator.cpp Translator/ClangLLVMIRTranslator.h CodeGenerator/CodeGenerator.cpp CodeGenerator/CodeGenerator.h utils.h utils.cpp FileMapper/FileMapper.cpp FileMapper/FileMapper.h Translator/Translator.h ConfigWalker/ConfigFormatException.cpp ConfigWalker/ConfigFormatException.h Translator/TranslatorException.cpp Translator/TranslatorException.h)

#llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(Program
        clangFrontend
        clangSerialization
        clangDriver
        clangParse
        clangSema
        clangAnalysis
        clangAST
        clangBasic
        clangEdit
        clangLex
        clangTooling
        clangCodeGen
        )

#target_link_libraries(Program
#        PRIVATE
#        clangAST
#        clangBasic
#        clangDriver
#        clangFrontend
#        clangRewriteFrontend
#        clangSerialization
#        clangStaticAnalyzerFrontend
#        clangTooling
#        clangToolingSyntax
#        )
