#include "TessaInstructionheader.h"
#include "BasicBlock.h"		// Needed to break circular dependency

namespace TessaInstructions {
	SwitchInstruction::SwitchInstruction(MMgc::GC* gc, TessaInstruction* switchValue, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		_caseInstructions = new (gc) List<CaseInstruction*, LIST_GCObjects>(gc);
		this->_switchValue = switchValue;
	}

	void SwitchInstruction::print() {
		std::string caseStrings = "";
		char caseBuffer[16];

		for (int i = 0; i < numberOfCases(); i++) {
			CaseInstruction* caseInstruction = getCase(i);
			BasicBlock* caseBlock = caseInstruction->getTargetBlock();
			_itoa(caseBlock->getBasicBlockId(), caseBuffer, 10);

			caseStrings += "BB";
			caseStrings += caseBuffer;
			caseStrings += " ";
		}

		// Don't need a space before the last ] because caseStrings adds one
		printf("%s Switch %s [ %s]\n", this->getPrintPrefix().c_str(), _switchValue->getOperandString().data(), caseStrings.c_str());
	}

	void SwitchInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool SwitchInstruction::isSwitch() {
		return true;
	}

	List<CaseInstruction*, LIST_GCObjects>* SwitchInstruction::getCaseInstructions() {
		return _caseInstructions;
	}

	CaseInstruction* SwitchInstruction::getDefaultCase() {
		TessaAssert(_caseInstructions->size() > 0);
		// Default case is ALWAYS the last one
		return _caseInstructions->get(_caseInstructions->size() - 1);
	}

	TessaInstruction* SwitchInstruction::getSwitchValue() {
		return this->_switchValue;
	}

	void SwitchInstruction::addCaseInstruction(CaseInstruction* caseInstruction) {
		TessaAssert(!_caseInstructions->contains(caseInstruction));
		_caseInstructions->add(caseInstruction);
	}

	void SwitchInstruction::setDefaultCase(CaseInstruction* caseInstruction) {
		TessaAssert(!_caseInstructions->contains(caseInstruction));
		_caseInstructions->add(caseInstruction);
	}

	/***
	 * Returns the number of case statements in the switch, INCLUDING the default
	 */
	int SwitchInstruction::numberOfCases() {
		return _caseInstructions->size();
	}

	CaseInstruction* SwitchInstruction::getCase(int index) {
		return _caseInstructions->get(index);
	}

	List<TessaValue*, LIST_GCObjects>* SwitchInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		for (int i = 0; i < numberOfCases(); i++) {
			operandList->add(getCase(i));
		}

		operandList->add(_switchValue);
		return operandList;
	}
}