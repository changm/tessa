
#include "TessaInstructionheader.h"

/*
namespace TessaInstructions {
	LabelInstruction::LabelInstruction() {

	}

	LabelInstruction::LabelInstruction(BasicBlock* basicBlock, int labelId) {
		this->_labelId = labelId;
		this->_basicBlock = basicBlock;
	}

	int LabelInstruction::getLabelId() {
		return _labelId;
	}

	bool LabelInstruction::isLabel() {
		return true;
	}

	BasicBlock* LabelInstruction::getBasicBlock() {
		return _basicBlock;
	}

	void LabelInstruction::print() {
		char buffer[32];
		sprintf_s(buffer, 32, "Label %d:", _labelId);
		printf("%s\n", buffer);
	}

	void LabelInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	LabelInstruction* LabelInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap) {
		return new (gc) LabelInstruction();
	}
}
*/