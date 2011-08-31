#include "TessaVM.h"

namespace TessaVM {
	BasicBlock::BasicBlock() {
		TessaAssert(false);
	}

	BasicBlock::BasicBlock(avmplus::AvmCore* core, int basicBlockNumber) {
		this->_core = core;
		this->_gc = core->gc;
		this->_terminatesWithReturn = false;

		_instructions = new (_gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(_gc);
		_phiInstructions = new (_gc) List<PhiInstruction*, avmplus::LIST_GCObjects>(_gc);
		_parameterInstructions = new (_gc) List<ParameterInstruction*, avmplus::LIST_GCObjects>(_gc);
		_predecessors = new (_gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(_gc);;
		this->_basicBlockId = basicBlockNumber;
	}

	void BasicBlock::addInstruction(TessaInstruction* tessaInstruction) {
		_instructions->add(tessaInstruction);
		tessaInstruction->setInBasicBlock(this);
		if (tessaInstruction->isParameter()) {
			_parameterInstructions->add((ParameterInstruction*)tessaInstruction);
		}
	}

	void BasicBlock::addInstructionBefore(TessaInstruction* instruction, TessaInstruction* insertBefore) {
		TessaAssert(_instructions->contains(insertBefore));
		int index = _instructions->indexOf(insertBefore);
		_instructions->insert(index, instruction);
		instruction->setInBasicBlock(this);
	}

	void BasicBlock::addInstructionAfter(TessaInstruction* instruction, TessaInstruction* insertAfter) {
		TessaAssertMessage(_instructions->contains(insertAfter), "Trying to add instruction after, but insertAfter does not exist");
		int index = _instructions->indexOf(insertAfter);
		_instructions->insert(index + 1, instruction);
		instruction->setInBasicBlock(this);
	}

	void BasicBlock::removeInstruction(TessaInstruction* instruction) {
		TessaAssert(_instructions->contains(instruction));
		int index = _instructions->indexOf(instruction);
		_instructions->removeAt(index);

		if (instruction->isParameter()) {
			ParameterInstruction* paramInstruction = reinterpret_cast<ParameterInstruction*>(instruction);
			TessaAssert(_parameterInstructions->contains(paramInstruction));
			int paramIndex = _parameterInstructions->indexOf(paramInstruction);
			_parameterInstructions->removeAt(paramIndex);
		}

		if (instruction->isPhi()) {
			PhiInstruction* phiInstruction = reinterpret_cast<PhiInstruction*>(instruction);
			TessaAssert(_phiInstructions->contains(phiInstruction));
			int phiIndex = _phiInstructions->indexOf(phiInstruction);
			_phiInstructions->removeAt(phiIndex);
		}
	}

	void BasicBlock::addPhiInstruction(PhiInstruction* phiInstruction) {
		_instructions->insert(0, phiInstruction);
		_phiInstructions->add(phiInstruction);

		phiInstruction->setInBasicBlock(this);
	}

	bool BasicBlock::isLoopHeader() {
		return _loopHeader;
	}

	void BasicBlock::setLoopHeader(bool loopHeader) {
		TessaAssert(!isSwitchHeader());
		this->_loopHeader = loopHeader;
	}

	bool BasicBlock::isSwitchHeader() {
		return _switchHeader;
	}

	void BasicBlock::setSwitchHeader(bool switchHeader) {
		TessaAssert(!isLoopHeader());
		this->_switchHeader = switchHeader;
	}

	void BasicBlock::printResults() {
		printf("\n\nBB%d:\n", getBasicBlockId());
		/*
		printf("\n\nBB%d:\n    Successors:", getBasicBlockId());

		std::string successorList;
		char buffer[32];

		List<BasicBlock*, avmplus::LIST_GCObjects>* successors = this->getSuccessors();
		for (uint32_t i = 0; i < successors->size(); i++) {
			successorList += "BB";
			int successorId = successors->get(i)->getBasicBlockId();
			_itoa(successorId, buffer, 10);

			successorList += buffer;
			successorList += " ";
		}

		printf("%s\n", successorList.c_str());

		std::string predecessorList;
		List<BasicBlock*, avmplus::LIST_GCObjects>* predecessors = this->getPredecessors();
		for (uint32_t i = 0; i < predecessors->size(); i++) {
			predecessorList += "BB";
			int predecessorId = predecessors->get(i)->getBasicBlockId();
			_itoa(predecessorId, buffer, 10);

			predecessorList += buffer;
			predecessorList += " ";
		}

		printf("    Predecessors: %s\n", predecessorList.c_str());
		*/

		for (uint32_t i = 0; i < _instructions->size(); i++) {
			TessaInstruction* currentInstruction = _instructions->get(i);
			currentInstruction->print();
		}
	}

	void BasicBlock::addPredecessor(TessaInstruction* predecessorTerminator) {
		TessaAssert(predecessorTerminator->isBlockTerminator() || predecessorTerminator->isCase());
		_predecessors->add(predecessorTerminator);
	}

	void BasicBlock::removePredecessor(TessaInstruction* predecessorInstruction) {
		BasicBlock* predecessorBlock = predecessorInstruction->getInBasicBlock();
		TessaAssert(isPredecessor(predecessorBlock));
		uint32_t index = _predecessors->indexOf(predecessorInstruction);
		_predecessors->removeAt(index);
	}

	bool BasicBlock::isSuccessor(BasicBlock* successor) {
		return this->getSuccessors()->contains(successor);
	}

	bool BasicBlock::isPredecessor(BasicBlock* predecessor) {
		return this->getPredecessors()->contains(predecessor);
	}

	int BasicBlock::getBasicBlockId() {
		return this->_basicBlockId;
	}

	List<TessaInstruction*, avmplus::LIST_GCObjects>* BasicBlock::getInstructions() {
		return _instructions;
	}

	TessaInstruction* BasicBlock::getLastInstruction() {
		int size = _instructions->size();
		TessaAssert(size > 0);
		return _instructions->get(size - 1);
	}

	int	BasicBlock::getInstructionIndex(TessaInstruction* instruction) {
		for (uint32_t i = 0; i < _instructions->size(); i++) {
			TessaInstruction* currentInstruction = _instructions->get(i);
			if (instruction == currentInstruction) {
				return i;
			}
		}

		AvmAssertMsg(false, "Instruction does not exist");
		return 0;
	}

	List<PhiInstruction*, avmplus::LIST_GCObjects>* BasicBlock::getPhiInstructions() {
		return _phiInstructions;
	}

	List<ParameterInstruction*, avmplus::LIST_GCObjects>* BasicBlock::getParameterInstructions() {
		return _parameterInstructions;
	}

	List<BasicBlock*, avmplus::LIST_GCObjects>* BasicBlock::getSuccessors() {
		List<BasicBlock*, avmplus::LIST_GCObjects>* successors = new (_gc) List<BasicBlock*, LIST_GCObjects>(_gc);
		if (_instructions->size() == 0) {
			return successors;
		}

		TessaInstruction* lastInstruction = this->getLastInstruction();
		if (lastInstruction->isUnconditionalBranch()) {
			UnconditionalBranchInstruction* unconditionalBranch = static_cast<UnconditionalBranchInstruction*>(lastInstruction);
			successors->add(unconditionalBranch->getBranchTarget());
		} else if (lastInstruction->isConditionalBranch()) {
			ConditionalBranchInstruction* conditionalBranch = static_cast<ConditionalBranchInstruction*>(lastInstruction);
			successors->add(conditionalBranch->getTrueTarget());
			successors->add(conditionalBranch->getFalseTarget());
		} else if (lastInstruction->isSwitch()) {
			SwitchInstruction* switchInstruction = static_cast<SwitchInstruction*>(lastInstruction);
			List<CaseInstruction*, LIST_GCObjects>* caseStatements = switchInstruction->getCaseInstructions();
			TessaAssert(switchInstruction->numberOfCases() == caseStatements->size());
			for (int i = 0; i < switchInstruction->numberOfCases(); i++) {
				CaseInstruction* caseInstruction = caseStatements->get(i);
				successors->add(caseInstruction->getTargetBlock());
			}
		} else {
			// The last instruction can be a parameter if there is only one basic block in the whole method
			TessaAssert(lastInstruction->isReturn() || lastInstruction->isParameter());
		}

		return successors;
	}

	List<BasicBlock*, avmplus::LIST_GCObjects>* BasicBlock::getPredecessors() {
		List<BasicBlock*, avmplus::LIST_GCObjects>* predecessorsAsBasicBlocks = new (_gc) List<BasicBlock*, avmplus::LIST_GCObjects>(_gc);
		for (uint32_t i = 0; i < _predecessors->size(); i++) {
			predecessorsAsBasicBlocks->add(_predecessors->get(i)->getInBasicBlock());
		}

		return predecessorsAsBasicBlocks;
	}
	
	StateVector* BasicBlock::getEndState() {
		return _endStateVector;
	}

	StateVector* BasicBlock::getBeginState() {
		return _beginStateVector;
	}

	void BasicBlock::setBeginState(StateVector* stateVector) {
		_beginStateVector = stateVector;
	}

	void BasicBlock::setEndState(StateVector* stateVector) {
		_endStateVector = stateVector;
	}

	void BasicBlock::cleanInstructions() {
		this->_instructions = new (_gc) List<TessaInstruction*, LIST_GCObjects>(_gc);
	}
}