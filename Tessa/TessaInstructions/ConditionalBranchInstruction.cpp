#include "TessaInstructionHeader.h"
#include "BasicBlock.h"	// Have to include basic block here to avoid circular dependences. TessaVM relies on TEssaInstructions

namespace TessaInstructions {
	ConditionalBranchInstruction::ConditionalBranchInstruction(TessaInstruction* branchCondition, BasicBlock* trueTarget, BasicBlock* falseTarget, TessaVM::BasicBlock* insertAtEnd) 
		: BranchInstruction(insertAtEnd) 
	{
		setBranchCondition(branchCondition);
		setTrueTarget(trueTarget);
		setFalseTarget(falseTarget);
	}

	ConditionalBranchInstruction::ConditionalBranchInstruction(TessaInstruction* branchCondition, TessaVM::BasicBlock* insertAtEnd) 
		: BranchInstruction(insertAtEnd) 
	{
		setBranchCondition(branchCondition);
	}

	void ConditionalBranchInstruction::setBranchCondition(TessaInstruction* branchCondition) {
		TessaAssert(branchCondition->isCoerce() || branchCondition->isCondition());
		this->_branchCondition = branchCondition;
	}

	TessaInstruction* ConditionalBranchInstruction::getBranchCondition() {
		return this->_branchCondition;
	}

	bool ConditionalBranchInstruction::isConditionalBranch() {
		return true;
	}

	bool ConditionalBranchInstruction::isUnconditionalBranch() {
		return false;
	}

	void ConditionalBranchInstruction::setTrueTarget(BasicBlock* trueTarget) {
		if (_trueTarget != NULL) {
			_trueTarget->removePredecessor(this);
		}

		TessaAssert(trueTarget != NULL);
		this->_trueTarget = trueTarget;
		trueTarget->addPredecessor(this);
	}

	void ConditionalBranchInstruction::setFalseTarget(BasicBlock* falseTarget) {
		if (this->_falseTarget != NULL) {
			_falseTarget->removePredecessor(this);
		}

		TessaAssert(falseTarget != NULL);
		this->_falseTarget = falseTarget;
		falseTarget->addPredecessor(this);
	}

	BasicBlock* ConditionalBranchInstruction::getTrueTarget() {
		TessaAssert(_trueTarget != NULL);
		return _trueTarget;
	}

	BasicBlock* ConditionalBranchInstruction::getFalseTarget() {
		TessaAssert(_falseTarget != NULL);
		return _falseTarget;
	}

	void ConditionalBranchInstruction::print() {
		char buffer[512];
		VMPI_snprintf(buffer, sizeof(buffer), "%s ConditionalBranchInstruction %s [ BB%d , BB%d ]\n", getPrintPrefix().c_str(), 
			_branchCondition->getOperandString().c_str(), _trueTarget->getBasicBlockId(), _falseTarget->getBasicBlockId());

		printf("%s", buffer);
	}

	void ConditionalBranchInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	ConditionalBranchInstruction* ConditionalBranchInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		BasicBlock* trueBlock = (BasicBlock*) originalToCloneMap->get(_trueTarget);
		BasicBlock* falseBlock = (BasicBlock*) originalToCloneMap->get(_falseTarget);
		TessaInstruction* newBranch = (TessaInstruction*) originalToCloneMap->get(_branchCondition);

		TessaAssert(trueBlock != NULL);
		TessaAssert(falseBlock != NULL);
		TessaAssert(newBranch != NULL);

		return new (gc) ConditionalBranchInstruction(newBranch, trueBlock, falseBlock, insertCloneAtEnd);
	}

	List<TessaValue*, LIST_GCObjects>* ConditionalBranchInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getBranchCondition());
		return operandList;
	}
};
	