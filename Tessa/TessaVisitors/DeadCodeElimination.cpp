#include "TessaVisitors.h"

namespace TessaVisitors {
	using namespace TessaVM;
	using namespace TessaInstructions;
	using namespace avmplus;

	/***
	 * Go through each instruction and add uses to each definition.
	 * Then, if an instruction has no uses, we can remove it from it's parent block.
	 */
	DeadCodeElimination::DeadCodeElimination(AvmCore* core, MMgc::GC* gc) {
		this->_core = core;
		this->_gc = gc;
	}

	void DeadCodeElimination::addValues(List<TessaValue*, LIST_GCObjects>* operands) {
		for (uint32_t i = 0; i < operands->size(); i++) {
			TessaValue* currentValue = operands->get(i);
			_liveValues.insert(currentValue);
		}
	}

	void DeadCodeElimination::addValue(TessaValue* value) {
		_liveValues.insert(value);
	}

	bool DeadCodeElimination::isAlive(TessaValue* value) {
		return _liveValues.count(value) != 0;
	}

	void DeadCodeElimination::visit(TessaInstruction* tessaInstruction) {
		TessaAssert(false);
	}

	void DeadCodeElimination::visit(FindPropertyInstruction* findPropertyInstruction) {
	}

	void DeadCodeElimination::visit(ConstantValueInstruction* constantValueInstruction) {
	}

	void DeadCodeElimination::visit(ArrayOfInstructions* arrayOfInstructions) {
	}

	void DeadCodeElimination::visit(NewObjectInstruction* newObjectInstruction) {
	}

	void DeadCodeElimination::visit(ConstructInstruction* constructInstruction) {
	}

	void DeadCodeElimination::visit(ConstructPropertyInstruction* constructPropertyInstruction) {
	}

	void DeadCodeElimination::visit(ConstructSuperInstruction* constructSuperInstruction) {
	}

	void DeadCodeElimination::visit(CallInstruction* callInstruction) {
	}

	void DeadCodeElimination::visit(CallVirtualInstruction* callVirtualInstruction) {
	}

	void DeadCodeElimination::visit(CallStaticInstruction* callStaticInstruction) {
	}

	void DeadCodeElimination::visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction) {
	}

	void DeadCodeElimination::visit(CallInterfaceInstruction* callInterfaceInstruction) {
	}

	void DeadCodeElimination::visit(CallSuperInstruction* callSuperInstruction) {
	}

	void DeadCodeElimination::visit(CallPropertyInstruction* callPropertyInstruction) {
	}

	void DeadCodeElimination::visit(ReturnInstruction* returnInstruction) {
	}

	void DeadCodeElimination::visit(BinaryInstruction* binaryInstruction) {
	}

	void DeadCodeElimination::visit(ConditionInstruction* conditionInstruction) {
	}

	void DeadCodeElimination::visit(UnaryInstruction* unaryInstruction) {
	}

	void DeadCodeElimination::visit(ConditionalBranchInstruction* conditionalBranchInstruction) {
	}

	void DeadCodeElimination::visit(UnconditionalBranchInstruction* unconditionalBranchInstruction) {
	}

	void DeadCodeElimination::visit(PhiInstruction* phiInstruction) {
	}

	void DeadCodeElimination::visit(ParameterInstruction* parameterInstruction) {
	}

	void DeadCodeElimination::visit(ConvertInstruction* convertInstruction) {
	}

	void DeadCodeElimination::visit(CoerceInstruction* coerceInstruction) {
	}

	void DeadCodeElimination::visit(GetPropertyInstruction* getPropertyInstruction) {
	}

	void DeadCodeElimination::visit(SetPropertyInstruction* setPropertyInstruction) {
	}

	void DeadCodeElimination::visit(InitPropertyInstruction* initPropertyInstruction) {
	}

	void DeadCodeElimination::visit(GetSlotInstruction* getSlotInstruction) {
	}

	void DeadCodeElimination::visit(SetSlotInstruction* setSlotInstruction) {
	}

	void DeadCodeElimination::visit(NewArrayInstruction* newArrayInstruction) {
	}

	void DeadCodeElimination::visit(NextNameInstruction* nextNameInstruction) {
	}

	void DeadCodeElimination::visit(PushScopeInstruction* pushScopeInstruction) {
	}

	void DeadCodeElimination::visit(PopScopeInstruction* popScopeInstruction) {
	}

	void DeadCodeElimination::visit(WithInstruction* withInstruction) {
		AvmAssert(false);
	}

	void DeadCodeElimination::visit(TypeOfInstruction* typeOfInstruction) {
	}

	void DeadCodeElimination::visit(GetScopeObjectInstruction* getScopeObjectInstruction) {
	}

	void DeadCodeElimination::visit(GetGlobalScopeInstruction* getGlobalScopeInstruction) {
	}

	void DeadCodeElimination::visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction) {
		AvmAssert(false);
	}

	void DeadCodeElimination::visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction) {
		AvmAssert(false);
	}

	void DeadCodeElimination::visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesRegisterInstruction) {
		AvmAssert(false);
	}

	void DeadCodeElimination::visit(SwitchInstruction* switchInstruction) {
	}

	void DeadCodeElimination::visit(CaseInstruction* caseInstruction) {
	}

	void DeadCodeElimination::visit(NewActivationInstruction* newActivationInstruction) {
	}

	void DeadCodeElimination::visit(InlineBeginInstruction* inlineBeginInstruction) {
	}

	bool DeadCodeElimination::isFunctionParameter(TessaInstruction* currentInstruction, BasicBlock* basicBlock) {
		if (currentInstruction->isParameter()) {
			return (basicBlock->getBasicBlockId() == 0) && (basicBlock->getPredecessors()->size() == 0);
		}

		return false;
	}

	List<TessaInstruction*, avmplus::LIST_GCObjects>* DeadCodeElimination::cloneInstructionList(List<TessaInstruction*, avmplus::LIST_GCObjects>* instructionsToClone) {
		List<TessaInstruction*, avmplus::LIST_GCObjects>* clonedList = new (_gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(_gc);
		for (uint32_t i = 0; i < instructionsToClone->size(); i++) {
			TessaInstruction* originalInstruction = instructionsToClone->get(i);
			clonedList->add(originalInstruction);
		}

		AvmAssert(clonedList->size() == instructionsToClone->size());
		return clonedList;
	}

	void DeadCodeElimination::addOperands() {
		int currentCount = _liveValues.size();
		int oldCount = 0;
		while (currentCount != oldCount) {
			oldCount = currentCount;

			for (set<TessaValue*>::iterator iter = _liveValues.begin(); iter != _liveValues.end(); iter++) {
				TessaValue* liveValue = *iter;
				if (liveValue->isInstruction()) {
					TessaInstruction* liveInstruction = liveValue->toInstruction();
					addValues(liveInstruction->getOperands(_gc));
				}
			}

			currentCount = _liveValues.size();
		}
	}

	void DeadCodeElimination::markUsedCode(ASFunction* function) {
		List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks = function->getBasicBlocksInReversePostOrder();

		for (int basicBlockIndex = basicBlocks->size() - 1; basicBlockIndex >= 0; basicBlockIndex--) {
			BasicBlock* basicBlock = basicBlocks->get(basicBlockIndex);
			List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = basicBlock->getInstructions();
			for (int instructionIndex = instructions->size() - 1; instructionIndex >= 0; instructionIndex--) {
				TessaInstruction* currentInstruction = instructions->get(instructionIndex);
				if (currentInstruction->hasSideEffect()) { 
					addValue(currentInstruction);
				}
			}
		}
	}

	bool DeadCodeElimination::removeDeadInstructions(ASFunction* function) {
		List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks = function->getBasicBlocks();

		for (uint32_t basicBlockIndex = 0; basicBlockIndex < basicBlocks->size(); basicBlockIndex++) {
			BasicBlock* basicBlock = basicBlocks->get(basicBlockIndex);
			List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = basicBlock->getInstructions();
			List<TessaInstruction*, avmplus::LIST_GCObjects>* clonedList = cloneInstructionList(instructions);
			int instructionSize = clonedList->size();

			// Have to clone since we are modifying the original instruction list
			for (uint32_t instructionIndex = 0; instructionIndex < instructionSize; instructionIndex++) {
				AvmAssert(clonedList->size() == instructionSize);
				TessaInstruction* currentInstruction = clonedList->get(instructionIndex);
				if (!isAlive(currentInstruction) && !isFunctionParameter(currentInstruction, basicBlock)) {
					basicBlock->removeInstruction(currentInstruction);
				}
			}
		}
		return true;
	}

	void DeadCodeElimination::eliminateDeadCode(ASFunction* function) {
#ifdef DEBUG
		if (_core->config.tessaVerbose) {
			printf("\n\n==== Dead Code Elimination ====\n");
		}
#endif

		markUsedCode(function);
		addOperands();
		removeDeadInstructions(function);

#ifdef DEBUG
		if (_core->config.tessaVerbose) {
			function->printResults();
		}
#endif
	}


}