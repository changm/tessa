#include "TessaInstructionheader.h"
#include "StateVector.h"
#include "BasicBlock.h"	// Cannot incude TessavM otherwise because TessaVM includes TessaInstructionHeader

namespace TessaInstructions {
	PhiInstruction::PhiInstruction(TessaVM::BasicBlock* insertInFront) :
		TessaInstruction() 
	{
		insertInFront->addPhiInstruction(this);
	}

	PhiInstruction::PhiInstruction(MMgc::GC* gc, TessaVM::BasicBlock* insertInFront) :
		TessaInstruction()
	{
		insertInFront->addPhiInstruction(this);
		_values = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc); 
		_from = new (gc) List<BasicBlock*, avmplus::LIST_GCObjects>(gc);
	}

	void PhiInstruction::addOperand(BasicBlock* comingFrom, TessaInstruction* phiValue) {
		TessaAssert(_from->size() == _values->size());
		_from->add(comingFrom);
		_values->add(phiValue);
	}

	void PhiInstruction::removeOperand(TessaVM::BasicBlock* incomingEdge) {
		TessaAssert(_from->size() == _values->size());
		int index = _from->indexOf(incomingEdge);

		_from->removeAt(index);
		_values->removeAt(index);

		TessaAssert(_from->size() == _values->size());
	}

	BasicBlock* PhiInstruction::getIncomingEdge(int index) {
		TessaAssert(_from->size() == _values->size());
		return _from->get(index);
	}

	void PhiInstruction::setOperand(TessaVM::BasicBlock* comingFrom, TessaInstruction* newOperand) {
		TessaAssert(_from->size() == _values->size());
		for (uint32_t i = 0; i < _from->size(); i++) {
			BasicBlock* currentBasicBlock = _from->get(i);
			if (currentBasicBlock == comingFrom) {
				return _values->set(i, newOperand);
			}
		}

		TessaAssertMessage(false, "Phi not tracking value\n");
	}

	TessaInstruction* PhiInstruction::getOperand(BasicBlock* comingFrom) {
		TessaAssert(_from->size() == _values->size());
		for (uint32_t i = 0; i < _from->size(); i++) {
			BasicBlock* currentBasicBlock = _from->get(i);
			if (currentBasicBlock == comingFrom) {
				return _values->get(i);
			}
		}

		TessaAssertMessage(false, "Phi not tracking value\n");
		return NULL;
	}

	bool PhiInstruction::isPhi() {
		return true;
	}

	void PhiInstruction::print() {
		TessaAssert(_from->size() == _values->size());
		std::string outputString = getPrintPrefix();
		outputString += " PhiInstruction [";
		char buffer[32];

		for (uint32_t i = 0; i < _from->size(); i++) {
			int basicBlockId = _from->get(i)->getBasicBlockId();
			_itoa(basicBlockId, buffer, 10);
			TessaInstruction* value = _values->get(i);

			outputString += " ( BB";
			outputString += buffer;
			outputString += ", ";
			outputString += value->getOperandString(); 
			outputString += " ) ";
		}

		outputString += "]";
		printf("%s Type (%s)\n", outputString.c_str(), getType()->toString().data());
	}

	void PhiInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	int	PhiInstruction::numberOfOperands() {
		int valueSize = _values->size();
		TessaAssert(valueSize == _from->size());
		TessaAssert(valueSize >= 0);
		return valueSize;
	}

	bool PhiInstruction::containsEdge(TessaVM::BasicBlock* incomingEdge) {
		return _from->contains(incomingEdge);
	}

	PhiInstruction*	PhiInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		PhiInstruction* clonedPhi = new (gc) PhiInstruction(gc, insertCloneAtEnd);
		/*
		for (uint32_t i = 0; i < _values->size(); i++) {
			TessaInstruction* clonedValue = (TessaInstruction*) originalToCloneMap->get(_values->get(i));
			BasicBlock* clonedBlock = (BasicBlock*) originalToCloneMap->get(_from->get(i));

			TessaAssert(clonedValue != NULL);
			TessaAssert(clonedBlock != NULL);

			clonedPhi->addOperand(clonedBlock, clonedValue);
		}
		*/
		clonedPhi->setType(this->getType());
		return clonedPhi;
	}

	List<TessaValue*, LIST_GCObjects>* PhiInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operandList = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		for (int i = 0; i < numberOfOperands(); i++) {
			operandList->add(_values->get(i));
		}

		return operandList;
	}
}