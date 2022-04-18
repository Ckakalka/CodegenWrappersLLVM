
#include "CodeGenerator.h"

#include <iostream>
#include <llvm/IR/DebugInfo.h>
CodeGenerator::CodeGenerator(std::unique_ptr<llvm::Module> module, ConfigWalker &configWalker) {
	llvm::DebugInfoFinder Finder;
	Finder.processModule(*module);

	for (const llvm::DIType *type : Finder.types()) {
		if (llvm::DICompositeType::classof(type)) {
			const auto *cmpType = reinterpret_cast<const llvm::DICompositeType *>(type);
			if (cmpType->getTag() == llvm::dwarf::DW_TAG_structure_type ||
				cmpType->getTag() == llvm::dwarf::DW_TAG_class_type) {
				// Classes
				int fieldNumberInt = 0;
				configWalker.realMacroSubstitutions[MacroSubstitution::ClassName] = cmpType->getName().data();
				std::cout << "\n" << configWalker.realMacroSubstitutions[MacroSubstitution::ClassName] << "\n";
				llvm::DINodeArray elements = cmpType->getElements();
				for (auto el : elements) {
					const auto *element = reinterpret_cast<const llvm::DIType *>(el);
					if (llvm::DIType::classof(element)) {
						configWalker.realMacroSubstitutions[MacroSubstitution::FieldName] = element->getName().data();
						configWalker.realMacroSubstitutions[MacroSubstitution::FieldNumber] =
							std::to_string(fieldNumberInt).c_str();
						for (int i = 0; i < configWalker.blocks.size(); ++i) {
							configWalker.blocks[i].printStrings();
							std::cout << "\n";
						}
						++fieldNumberInt;
					}
				}
			}
		}
	}
}
