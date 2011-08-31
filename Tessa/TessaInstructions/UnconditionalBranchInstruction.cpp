#include "TessaInstructionheader.h"
#include "BasicBlock.h"	// Need to avoid circular dependencies with TessaVM;

namespace TessaInstructions {
	UnconditionalBranchInstruction::UnconditionalBranchInstruction(TessaVM::BasicBlock* branchTarget, BasicBlock* insertAtEnd) 
		: BranchInstruction(insertAtEnd) 
	{
		setBranchTarget(branchTarget); 

	}

	UnconditionalBranchInstruction::UnconditionalBranchInstruction(TessaVM::BasicBlock* insertAtEnd) 
		: BranchInstruction(insertAtEnd) 
	{
	}

	bool UnconditionalBranchInstruction::isConditionalBranch() {
		return false;
	}

	bool UnconditionalBranchInstruction::isUnconditionalBranch() {
		return true;
	}

	BasicBlock* UnconditionalBranchInstruction::getBranchTarget() {
		return _branchTarget;
	}

	void UnconditionalBranchInstruction::setBranchTarget(BasicBlock* branchTarget) {
		if (this->_branchTarget != NULL) {
			_branchTarget->removePredecessor(this);
		}

		TessaAssert(branchTarget != NULL);
		this->_branchTarget = branchTarget;
		branchTarget->addPredecessor(this);
	}

	void UnconditionalBranchInstruction::visit(TessaVisitorInterface *tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void UnconditionalBranchInstruction::print() {
		int targetBasicBlockId = _branchTarget->getBasicBlockId();
		//int targetBasicBlockId = 5;
		char buffer[64];
		VMPI_snprintf(buffer, sizeof(buffer), "%s UnconditionalBranch BB%d\n", 
			this->getPrintPrefix().c_str(), targetBasicBlockId);

		printf("%s", buffer);
	}

	UnconditionalBranchInstruction* UnconditionalBranchInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) { 
		BasicBlock* clonedBlock = (BasicBlock*) originalToCloneMap->get(_branchTarget);
		TessaAssert(clonedBlock != NULL);
		return new (gc) UnconditionalBranchInstruction(clonedBlock, insertCloneAtEnd);
	}

	List<TessaValue*, LIST_GCObjects>* UnconditionalBranchInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		return operandList;
	}
};