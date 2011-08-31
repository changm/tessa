#include "TessaInstructionHeader.h"
#include "BasicBlock.h"		// To avoid circular dependencies with TessaVM

namespace TessaInstructions {
	CaseInstruction::CaseInstruction(TessaInstruction* caseValue, BasicBlock* targetBlock, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		setTargetBlock(targetBlock);
		this->_caseValue = caseValue;
	}

	bool CaseInstruction::isCase() {
		return true;
	}

	void CaseInstruction::print() {
		printf("%s Case Instruction\n", this->getPrintPrefix().c_str());
	}

	void CaseInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	SwitchInstruction* CaseInstruction::getSwitchInstruction() {
		return this->_switchInstruction;
	}

	void CaseInstruction::setSwitchInstruction(SwitchInstruction* switchInstruction) {
		this->_switchInstruction = switchInstruction;
	}

	BasicBlock* CaseInstruction::getTargetBlock() {
		TessaAssert(_targetBlock != NULL);
		return this->_targetBlock;
	}

	TessaInstruction* CaseInstruction::getCaseValue() {
		TessaAssert(_caseValue != NULL);
		return this->_caseValue;
	}

	void CaseInstruction::setTargetBlock(TessaVM::BasicBlock* caseTarget) {
		if (_targetBlock != NULL) {
			_targetBlock->removePredecessor(this);
		}

		this->_targetBlock = caseTarget;
		_targetBlock->addPredecessor(this);
	}

	List<TessaValue*, LIST_GCObjects>* CaseInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		// Can be null if this case is the default case
		if (this->_caseValue != NULL) {
			operandList->add(this->getCaseValue());
		}
		return operandList;
	}
}

