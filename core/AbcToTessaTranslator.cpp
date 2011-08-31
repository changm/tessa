using namespace TessaInstructions;

#include "avmplus.h"
#include "AbcToTessaTranslator.h"
#include "TessaVisitors.h"
#include "TessaTypeHeader.h"

// Needed for the jit call/set/get cache builders
//#include "CodegenLIR.h"

/***
 * The Verifier already does the abstract interpretation. 
 * So the state object already has what is on the stack, and scope stack.
 *
 * Methodology to translating ABC -> Tessa
 * 1) Go through the ABC, and recreate the control flow graph. Insert information such as whether a basic block is a loop header
 * or a control flow merge point etc. First pass.
 * 2) Second pass - actually go through the ABC again, this time with the verifier, and emit the actual Tessa instructions
 * 
 * We need to do the first pass so we know where to put in Phi instructions. We can't rely on the Verifier since we sitll need to recreate the CFG
 * with Tessa objects instead of core objects.
 */
namespace avmplus {
	AbcToTessaTranslator::AbcToTessaTranslator(AvmCore* core, MethodInfo *methodInfo, Verifier* verifier) :
		_poolObject(methodInfo->pool()),
		_abcPcToLabel(64),
		_stateToOperandStack(64),
		_call_cache_builder(_cacheAllocator, *initCodeMgr(_poolObject)),
		_get_cache_builder(_cacheAllocator, *_poolObject->codeMgr),
		_set_cache_builder(_cacheAllocator, *_poolObject->codeMgr)
	{
		this->verifier = verifier;
		this->_methodInfo = methodInfo;
		this->core = core;
		this->gc = core->gc;
		this->_tessaInstructionCreator = new (gc) TessaCreationApi(core, gc);

		TypeFactory::initInstance(gc);
		_typeFactory = TypeFactory::getInstance();

		this->_jumpTargetToLabels = new (gc) GCSortedMap<int, List<BranchInstruction*, LIST_GCObjects>*, LIST_GCObjects> (gc);
		this->_pcLabelMap = new (gc) GCSortedMap<int, BasicBlock*, LIST_GCObjects> (gc);
		this->_operandStack = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc);
		this->_returnValues = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc);
		this->_numberOfLocals = verifier->local_count;
	}

	CodeMgr* AbcToTessaTranslator::initCodeMgr(PoolObject *pool) {
	    if (!pool->codeMgr) {
	        CodeMgr *mgr = mmfx_new( CodeMgr() );
	        pool->codeMgr = mgr;
	#ifdef NJ_VERBOSE
	        mgr->log.core = pool->core;
	        mgr->log.lcbits = pool->verbose_vb;
	#endif
	    }
	    return pool->codeMgr;
    }

	AbcToTessaTranslator::~AbcToTessaTranslator() {
		TessaAssert(_operandStack->isEmpty());
		_finddef_cache_builder.cleanup();
	}

	void AbcToTessaTranslator::printResults() {
		if (core->config.tessaVerbose) {
			this->_tessaInstructionCreator->printResults();
		}
	}

	/***
	 * The notion of an operand stack is kind of a misnomer. It's more like an array where
	 * any value can be modified. This is required to conform to the verifier interface.
	 * However, teh AVM2 spec has this idea of an operand stack. We want to conform to the
	 * AVM2 spec rather than the verifier interface, and therefore call it an operand stack.
	 */
	void AbcToTessaTranslator::modifyStackValue(uint32_t index, TessaInstruction* value) {
		TessaAssert(index >= 0 && index < _operandStack->size());
		_operandStack->set(_operandStack->size() - index - 1, value);
	}

	void AbcToTessaTranslator::pushInstruction(TessaInstruction* instruction) {
		_operandStack->add(instruction);
	}

	TessaInstruction* AbcToTessaTranslator::popInstruction() {
		return _operandStack->removeLast();
	}

	TessaInstruction* AbcToTessaTranslator::peekInstruction(uint32_t index) {
		TessaAssert(index < _operandStack->size());
		// This is a stack so index is from the units from the TOP of the stack
		return _operandStack->get(_operandStack->size() - index - 1);
	}

	bool AbcToTessaTranslator::hasState(const FrameState* state) {
		return _stateToOperandStack.get(state) != NULL;
	}

	/***
	 * Call save state PRIOR to switching to a new basic block
	 */
	void AbcToTessaTranslator::saveState(const FrameState* state) {
		/***
		 * We are now at a branch point. If the stack depth is greater than zero, which means something is on the operand stack,
		 * Tell the current state vector to save this extra data.
		 * Also weird is that the state->stackDepth represents what happens BEFORE the current instruction.
		 * So we have cases like a branch where the state->stackDepth represents teh two items to compare in a conditional branch
		 * However, after the instruction executes, nothing should be on the stack. We want to save the state AFTER
		 * the execution of the instruction
		 */
		if (state->stackDepth > 0) {
			int currentStackHeight = this->getStackSize();
			int numberOfLocals = state->verifier->local_count;

			/***
			 * Stack slot is counting from the operand stack.
			 * We track the stack exactly. Ala stackSlot[0] is the bottom most item on the _operandStack
			 */
			for (int stackSlot = 0; stackSlot < currentStackHeight; stackSlot++) {
				/***
				 * This loop is counts top down, starting at the top of the stack and working our way down
				 * PeekInstruction counts the index from the top of the stack. peekInstruction(0) returns whats on top of the stack
				 * Therefore, stackSlot is the opposite from peekInstruction(). 
				 * Eg. StackSlot[0], is the bottom most item on the operand stack. peekInstruction(0) returns top of stack
				 * Subtle bug here
				 */
				TessaInstruction* stackReference = peekInstruction(getStackSize() - stackSlot - 1);	
				_tessaInstructionCreator->trackExtraVariableInEndState(stackSlot, stackReference, numberOfLocals);
			}
			//TessaAssert(getStackSize() == 0);
		} else {
			/**
			 * Otherwise we're already tracking all the local variables and nothing needs to be done at the merge point
			 */
			TessaAssert(state->stackDepth == 0);
		}
	}

	/***
	 * The verifier doesn't call writeBlockStart if the ABC falls through to this block (In the case of conditionals)
	 * Therefore, we have to recreate the stack for the fall through as if writeBlockStart was called.
	 */
	void AbcToTessaTranslator::loadFallThroughState(const FrameState* state) {
		int currentStackSize = getStackSize();
		int stateStackSize = state->stackDepth;
		if (currentStackSize != 0) {
			/***
			 * insert parameter instructions for stack slots which will get turned into phis later on
			 * We also have to erase the operand stack and start from scratch since the operand stack
			 * will have values that exist in a different basic block. Those values are saved
			 * by a state vector 
			 */
			_operandStack = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc);	
			for (int stackSlot = 0; stackSlot < currentStackSize; stackSlot++) {
				TessaInstruction* defaultValue = NULL;
				ParameterInstruction* stackParameter = new (gc) ParameterInstruction(defaultValue, _currentBasicBlock); 
				Value value = state->peek(state->sp() - stackSlot);
				pushInstruction(stackParameter);
				_tessaInstructionCreator->trackExtraVariableInEntryState(stackSlot, stackParameter, state->verifier->local_count);
			}
		} 
	}

	/***
	 * This is called when writeBlockStart is called 
	 */
	void AbcToTessaTranslator::loadState(const FrameState* state) {
		_operandStack = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc);	

		if (state->stackDepth > 0) {
			/***
			 * insert parameter instructions for stack slots which will get turned into phis later on
			 * We also have to erase the operand stack and start from scratch since the operand stack
			 * will have values that exist in a different basic block. Those values are saved
			 * by a state vector 
			 */
			for (int stackSlot = 0; stackSlot < state->stackDepth; stackSlot++) {
				// This means something is on the stack. Create a parameter instruction for it, which becomes a phi later
				// This is probably a ternary operator
				TessaInstruction* defaultValue = NULL;
				ParameterInstruction* stackParameter = new (gc) ParameterInstruction(defaultValue, _currentBasicBlock);
				pushInstruction(stackParameter);

				// state->peek counts from the top of the stack
				Type* parameterType = getType(state->peek(state->stackDepth - stackSlot).traits);
				_tessaInstructionCreator->trackExtraVariableInEntryState(stackSlot, stackParameter, state->verifier->local_count);
			}
		} else {
			/***
			 * We're already tracking local vars so we don't have to do anything here
			 */
			TessaAssert(getStackSize() == 0);
			TessaAssert(state->stackDepth == 0);
		}
	}

	int AbcToTessaTranslator::getStackSize() {
		return _operandStack->size();
	}

	ASFunction* AbcToTessaTranslator::getMethod() {
		return _tessaInstructionCreator->getFunction();
	}

	bool AbcToTessaTranslator::isInCurrentBasicBlock(TessaInstruction* instruction) {
		TessaAssert(_currentBasicBlock != NULL);
		BasicBlock* instructionBasicBlock = instruction->getInBasicBlock();
		return instructionBasicBlock == _currentBasicBlock;
	}

	bool AbcToTessaTranslator::isForwardBranch(int currentPc, int targetPc) {
		TessaAssert(currentPc != targetPc);
		return currentPc < targetPc;
	}

	bool AbcToTessaTranslator::isBackwardsBranch(int currentPc, int targetPc) {
		TessaAssert(currentPc != targetPc);
		return currentPc > targetPc;
	}

	bool AbcToTessaTranslator::isInteger(int atom) {
		return (((atom) & 7) == kIntptrType);
	}

	void AbcToTessaTranslator::printOpcode(const avmplus::byte* pc, AbcOpcode opcode, int offset) {
		/*
#ifdef DEBUG
		if (core->config.tessaVerbose) {
			core->formatOpcode(core->console, pc, opcode, offset, _poolObject);
			printf("\n");
		}
#else
		(void)pc;
		(void)opcode;
		(void)offset;

#endif
		*/
		(void)pc;
		(void)opcode;
		(void)offset;
	}

	/***
	 * Emit an explicit unconditional branch for control flow that falls through in ABC
	 */
	void AbcToTessaTranslator::emitUnconditionalBranchForFallThrough(int abcPc, BasicBlock* targetBlock, const FrameState* state) {
		TessaInstruction* lastInstruction = _currentBasicBlock->getLastInstruction();
		if ((lastInstruction->isBlockTerminator()) || (targetBlock == _currentBasicBlock)) {
			return;
		}

		List<BranchInstruction*, avmplus::LIST_GCObjects> *branchesToTarget = getBranchesToPc(abcPc);
		for (uint32_t i = 0; i < branchesToTarget->size(); i++) {
			BranchInstruction* branchInstruction = branchesToTarget->get(i);
			if (branchInstruction->getInBasicBlock() == _currentBasicBlock) {
				return;
			}
		}

		saveState(state);
		UnconditionalBranchInstruction* explicitFallThroughBranch = new (gc) UnconditionalBranchInstruction(_currentBasicBlock);
		branchesToTarget->add(explicitFallThroughBranch);
	}

	/***
	 * If the pc is has a label associated with it, we switch TessaCreationApi to
	 * start parsing that given basic block
	 */
	void AbcToTessaTranslator::startParsingBasicBlock(BasicBlock* targetBasicBlock, const FrameState* state) {
		_tessaInstructionCreator->switchToBasicBlock(targetBasicBlock);
		_currentBasicBlock = targetBasicBlock;
		setTypesOfLocals(_currentBasicBlock, state);
	}

	List<BranchInstruction*, avmplus::LIST_GCObjects>* AbcToTessaTranslator::getBranchesToPc(int abcPc) {
		int index = abcPc;
		if (_jumpTargetToLabels->get(index) == NULL) {
			_jumpTargetToLabels->put(index,  new (gc) List<BranchInstruction*, avmplus::LIST_GCObjects>(gc));
		}

		return (List<BranchInstruction*, avmplus::LIST_GCObjects>*)_jumpTargetToLabels->get(index);
	}

	void AbcToTessaTranslator::addBranchToPc(int abcPc, BranchInstruction* branchToTarget) {
		List<BranchInstruction*, avmplus::LIST_GCObjects> *branchesToTarget = getBranchesToPc(abcPc);
		branchesToTarget->add(branchToTarget);
	}

	BasicBlock* AbcToTessaTranslator::getBasicBlockAtPc(int abcPc) {
		int index = abcPc;
		if (!_pcLabelMap->containsKey(abcPc)) {
			_pcLabelMap->put(abcPc, _tessaInstructionCreator->createNewBasicBlock());
		}

		BasicBlock* basicBlock = _pcLabelMap->get(abcPc);
		return basicBlock;
	}

	ConstantFloat* AbcToTessaTranslator::createConstantDouble(double value) {
		return new (gc) ConstantFloat(value);
	}

	ConstantInt* AbcToTessaTranslator::createConstantInteger(int value) {
		bool isSigned = true;
		return new (gc) ConstantInt(value, isSigned);
	}

	ConstantString* AbcToTessaTranslator::createConstantString(Stringp value) {
		return new (gc) ConstantString(value);
	}

	ConstantUndefined* AbcToTessaTranslator::createConstantUndefined() {
		return new (gc) ConstantUndefined();
	}

	ConstantNull* AbcToTessaTranslator::createConstantNull() {
		return new (gc) ConstantNull();
	}

	ConstantBool* AbcToTessaTranslator::createConstantBoolean(bool value) {
		return new (gc) ConstantBool(value);
	}

	ConstantValueInstruction* AbcToTessaTranslator::createConstantValueInstruction(ConstantValue* constantValue, TessaVM::BasicBlock* insertAtEnd) {
		ConstantValueInstruction* constantValueInstruction = new (gc) ConstantValueInstruction(constantValue, insertAtEnd);
		if (constantValue->isInteger()) {
			constantValueInstruction->setType(_typeFactory->integerType());
		} else if (constantValue->isNumber()) {
			constantValueInstruction->setType(_typeFactory->numberType());
		} else if (constantValue->isString()) {
			constantValueInstruction->setType(_typeFactory->stringType());
		} else if (constantValue->isNull()) {
			constantValueInstruction->setType(_typeFactory->nullType());
		} else if (constantValue->isUndefined()) {
			constantValueInstruction->setType(_typeFactory->undefinedType());
		} else if (constantValue->isBoolean()) {
			constantValueInstruction->setType(_typeFactory->boolType());
		} else {
			TessaAssert(false);
		}
		return constantValueInstruction;
	}

	bool AbcToTessaTranslator::canPrecomputeBinaryInstruction(TessaInstruction* leftOperand, TessaInstruction* rightOperand) {
		return (leftOperand->isInteger() && rightOperand->isInteger() 
			&& leftOperand->isConstantValue() && rightOperand->isConstantValue());
	}

	ConstantValueInstruction*	AbcToTessaTranslator::precomputeBinaryValue(TessaBinaryOp opcode, TessaInstruction* leftOperand, TessaInstruction* rightOperand) {
		TessaAssert(leftOperand->isConstantValue());
		TessaAssert(rightOperand->isConstantValue());

		ConstantValueInstruction* leftConstant = static_cast<ConstantValueInstruction*>(leftOperand);
		ConstantValueInstruction* rightConstant = static_cast<ConstantValueInstruction*>(rightOperand);

		ConstantValue* leftValue = leftConstant->getConstantValue();
		ConstantValue* rightValue = rightConstant->getConstantValue();

		int leftIntValue = static_cast<ConstantInt*>(leftValue)->getValue();
		int rightIntValue = static_cast<ConstantInt*>(rightValue)->getValue();
		int resultValue = 0;

		switch (opcode) {
			case ADD:
				resultValue = leftIntValue + rightIntValue;
				break;
			case SUBTRACT:
				resultValue = leftIntValue - rightIntValue;
				break;
			case BITWISE_XOR:
				resultValue = leftIntValue ^ rightIntValue;
				break;
			case BITWISE_LSH: 
				resultValue = leftIntValue << rightIntValue;
				break;
			case BITWISE_RSH:
				resultValue = leftIntValue >> rightIntValue;
				break;
			case BITWISE_URSH:
				resultValue = (uint32_t(leftIntValue) >> int32_t(rightIntValue));
				break;
			case BITWISE_OR:
				resultValue = leftIntValue | rightIntValue;
				break;
			case BITWISE_AND:
				resultValue = leftIntValue & rightIntValue;
				break;
			default:
				TessaAssert(false);
				break;
		}

		bool isSigned = false;
		ConstantInt* constantResult = new (gc) ConstantInt(resultValue, isSigned);
		ConstantValueInstruction* constantValue = new (gc) ConstantValueInstruction(constantResult, _currentBasicBlock);
		constantValue->setType(_typeFactory->integerType());
		return constantValue;
	}

	void AbcToTessaTranslator::emitConvertToString(const FrameState* state, int sp) {
		const Value& value = state->value(sp);
        Traits* valueTraits = value.traits;
        Traits* stringType = STRING_TYPE;
		TessaInstruction* instructionToConvert = popInstruction();
		pushInstruction(new (gc) ConvertInstruction(_typeFactory->stringType(), instructionToConvert, _currentBasicBlock));

		/*
        if (valueTraits != stringType || !value.notNull) {
            if (valueTraits&& (value.notNull || valueTraits->isNumeric() || valueTraits == BOOLEAN_TYPE)) {
                // convert is the same as coerce
				pushInstruction(new (gc) CoerceInstruction(_typeFactory->stringType(), instructionToConvert, _currentBasicBlock));
            } else {
                // explicitly convert to string
				AvmAssert(false);
                //return callIns(FUNCTIONID(string), 2, coreAddr, loadAtomRep(index));
            }
        } else {
            // already String*
            //return localGetp(index);
			AvmAssert(false);
        }
		*/
	}

	TessaInstruction* AbcToTessaTranslator::emitBinaryInstruction(AbcOpcode abcOpcode, Traits* resultTraits) {
		TessaInstruction* rightOperand = popInstruction();
		TessaInstruction* leftOperand = popInstruction();
		TessaBinaryOp op = BinaryInstruction::getBinaryOpcodeFromAbcOpcode(abcOpcode); 
		TessaInstruction* result;

		if (canPrecomputeBinaryInstruction(leftOperand, rightOperand)) {
			result = precomputeBinaryValue(op, leftOperand, rightOperand);
		} else {
			BinaryInstruction* binaryInstruction = new (gc) BinaryInstruction(op, leftOperand, rightOperand, _currentBasicBlock);
			Type* resultType = getType(resultTraits);
			binaryInstruction->setType(resultType);
			result = binaryInstruction;
		}

		pushInstruction(result);
		return result;
	}

	void AbcToTessaTranslator::emitUnaryInstruction(AbcOpcode abcOpcode) {
		TessaInstruction* operand = popInstruction();
		TessaUnaryOp op = UnaryInstruction::getUnaryOpcodeFromAbcOpcode(abcOpcode); 
		UnaryInstruction* unaryInstruction = new (gc) UnaryInstruction(op, operand, _currentBasicBlock);
		switch (unaryInstruction->getOpcode()) {
			case BITWISE_NOT:
				unaryInstruction->setType(_typeFactory->integerType());
				break;
			case NEGATE:
				unaryInstruction->setType(_typeFactory->numberType());
				break;
			case NOT:
				unaryInstruction->setType(_typeFactory->boolType());
				break;
		}
		pushInstruction(unaryInstruction);
	}

	void AbcToTessaTranslator::emitReturnInstruction(AbcOpcode abcOpcode) {
		TessaInstruction* returnValue;
		if (abcOpcode == OP_returnvoid) {
			returnValue = createConstantValueInstruction(createConstantUndefined(), _currentBasicBlock);
		} else {
			returnValue = popInstruction();
		}

		if (_hasMultipleReturnValues) {
			_returnValues->add(returnValue);
			UnconditionalBranchInstruction* jumpToReturnBlock = new (gc) UnconditionalBranchInstruction(_returnBlock, _currentBasicBlock);
		} else {
			TessaTypes::Type* returnType = getType(_methodInfo->getMethodSignature()->returnTraits());
			ReturnInstruction* returnInstruction = new (gc) ReturnInstruction(returnValue, _currentBasicBlock);
			returnInstruction->setType(returnType);
		}
	}

	void AbcToTessaTranslator::emitBooleanBranch(AbcOpcode abcOpcode, int trueTargetPc, int falseTargetPc, const FrameState* state) {
		TessaInstruction* operand = popInstruction();
		TessaInstruction* equalityOperand = NULL;

		switch (abcOpcode) {
			case OP_iffalse:
				equalityOperand = createConstantValueInstruction(createConstantBoolean(false), _currentBasicBlock); 
				break;
			case OP_iftrue:
				equalityOperand = createConstantValueInstruction(createConstantBoolean(true), _currentBasicBlock); 
				break;
			default:
				TessaAssertMessage(false, "Should never get here");
		}

		TessaAssert(equalityOperand != NULL);
		emitConditionalBranch(abcOpcode, operand, equalityOperand, trueTargetPc, falseTargetPc, state);
	}

	void AbcToTessaTranslator::emitCondition(AbcOpcode abcOpcode) {
		TessaInstruction* rightOperand = popInstruction();
		TessaInstruction* leftOperand = popInstruction();
		TessaBinaryOp opcode = BinaryInstruction::getBinaryOpcodeFromAbcOpcode(abcOpcode);
		TessaInstruction* conditionInstruction = new (gc) ConditionInstruction(opcode, leftOperand, rightOperand, _currentBasicBlock);
		conditionInstruction->setType(_typeFactory->boolType());
		pushInstruction(conditionInstruction);
	}

	void AbcToTessaTranslator::emitConditionalBranch(AbcOpcode abcOpcode, int trueTargetPc, int falseTargetPc, const FrameState* state) {
		TessaInstruction* rightOperand = popInstruction();
		TessaInstruction* leftOperand = popInstruction();
		emitConditionalBranch(abcOpcode, leftOperand, rightOperand, trueTargetPc, falseTargetPc, state);
	}

	bool AbcToTessaTranslator::canOptimizeNullCheck(const FrameState* state) {
		int sp = state->sp();
		Traits* leftOperand = state->value(sp).traits;
		Traits* rightOperand = state->value(sp - 1).traits;
		
        // If we have null and a type that is derived from an Object (but not Object or XML)
        // we can optimize our equal comparison down to a simple ptr comparison. This also
        // works when both types are derived Object types.
        return (((leftOperand == NULL_TYPE) && (rightOperand && !rightOperand->notDerivedObjectOrXML())) ||
            ((rightOperand == NULL_TYPE) && (leftOperand && !leftOperand->notDerivedObjectOrXML())) ||
            ((rightOperand && !rightOperand->notDerivedObjectOrXML()) && (leftOperand && !leftOperand->notDerivedObjectOrXML())));
	}

	void AbcToTessaTranslator::emitConditionalBranch(AbcOpcode abcOpcode, TessaInstruction* leftOperand, TessaInstruction* rightOperand, int trueTargetPc, int falseTargetPc, const FrameState* state) {
		TessaAssert(trueTargetPc != falseTargetPc);
		TessaAssert(rightOperand != NULL);
		TessaAssert(leftOperand != NULL);

		TessaBinaryOp op = BinaryInstruction::getBinaryOpcodeFromAbcOpcode(abcOpcode); 
		ConditionInstruction* conditionInstruction = new (gc) ConditionInstruction(op, leftOperand, rightOperand, _currentBasicBlock); 
		bool forceOptimizeNullPointerComparison = canOptimizeNullCheck(state);
		conditionInstruction->forceOptimizeNullPointerCheck = forceOptimizeNullPointerComparison;
		ConditionalBranchInstruction* conditionalBranch = new (gc) ConditionalBranchInstruction(conditionInstruction, _currentBasicBlock); 

		// Have to create these AFTER we create the conditionalBranch otherwise the conditional branch will be inserted into the new basic block
		BasicBlock* trueBasicBlock = getBasicBlockAtPc(trueTargetPc); 
		BasicBlock* falseBasicBlock = getBasicBlockAtPc(falseTargetPc); 
				
		conditionalBranch->setTrueTarget(trueBasicBlock);
		conditionalBranch->setFalseTarget(falseBasicBlock);

		addBranchToPc(trueTargetPc, conditionalBranch);
		addBranchToPc(falseTargetPc, conditionalBranch);

		// Always fall through to the false branch
		saveState(state);
		startParsingBasicBlock(falseBasicBlock, state);
		loadFallThroughState(state);
	}

	void AbcToTessaTranslator::emitUnconditionalBranch(int currentPc, int targetPc, const FrameState* state) {
		saveState(state);
		UnconditionalBranchInstruction* unconditionalBranch = new (gc) UnconditionalBranchInstruction(_currentBasicBlock);

		if (isForwardBranch(currentPc, targetPc)) {
			addBranchToPc(targetPc, unconditionalBranch);
		} else {
			TessaAssert(false);
		}
	}

	bool AbcToTessaTranslator::needCoerceObject(Traits* result, Traits* in) {
        if (result == NULL)
        {
			return false;
        }
        else if (result == OBJECT_TYPE)
        {
			return false;
        }
        else if (!result->isMachineType() && in == NULL_TYPE)
        {
			return false;
        }
        else if (result == NUMBER_TYPE)
        {
			return false;
        }
        else if (result == INT_TYPE)
        {
			return false;
        }
        else if (result == UINT_TYPE)
        {
			return false;
        }
        else if (result == BOOLEAN_TYPE)
        {
			return false;
        }
        else if (result == STRING_TYPE)
        {
			return false;
        }
        else if (in && !in->isMachineType() && !result->isMachineType()
               && in != STRING_TYPE && in != NAMESPACE_TYPE)
        {
			return false;
        }
        else if (!result->isMachineType() && result != NAMESPACE_TYPE)
        {
			return true;
        }
        else if (result == NAMESPACE_TYPE && in == NAMESPACE_TYPE)
        {
			return false;
        }
        else
        {
			return false;
        }
	}

	void AbcToTessaTranslator::emitConvertOrCoerceInstruction(AbcOpcode abcOpcode, int multinameIndex, Traits* resultTraits, Traits* instructionTraits) {
		TessaInstruction* instructionToConvert = popInstruction();
		Type* resultType = getType(resultTraits);
		if (resultType == instructionToConvert->getType()) {
			pushInstruction(instructionToConvert);
			return;
		}

		TessaInstruction* result = NULL;

		switch (abcOpcode) {
		case OP_convert_b:
			result = new (gc) ConvertInstruction(_typeFactory->boolType(), instructionToConvert, _currentBasicBlock); 
			break;
		case OP_coerce:
		{
			// Coerce just leaves it alone?
			const Multiname* multinameToConvert = this->_poolObject->precomputedMultiname(multinameIndex);
			Type* tessaType = getType(resultTraits);
			bool needCoerce = needCoerceObject(resultTraits, instructionTraits);
			CoerceInstruction* coerceInstruction = new (gc) CoerceInstruction(multinameToConvert, instructionToConvert, tessaType, _currentBasicBlock); 
			coerceInstruction->useCoerceObjAtom = needCoerce;
			coerceInstruction->resultTraits = resultTraits;
			result = coerceInstruction;
			break;
		}
        case OP_coerce_b:
			result = new (gc) CoerceInstruction(_typeFactory->boolType(), instructionToConvert, _currentBasicBlock); 
			break;
		case OP_coerce_a:
			result = new (gc) CoerceInstruction(_typeFactory->anyType(), instructionToConvert, _currentBasicBlock);
			break;

		case OP_convert_d:
			result = new (gc) ConvertInstruction(_typeFactory->numberType(), instructionToConvert, _currentBasicBlock);
			break;
        case OP_convert_u:
			result = new (gc) ConvertInstruction(_typeFactory->uintType(), instructionToConvert, _currentBasicBlock);
			break;
		case OP_coerce_s:
			result = new (gc) CoerceInstruction(_typeFactory->stringType(), instructionToConvert, _currentBasicBlock);
			break;
		case OP_coerce_o:
			result = new (gc) CoerceInstruction(_typeFactory->objectType(), instructionToConvert, _currentBasicBlock);
			break;
        case OP_convert_i:
			result = new (gc) ConvertInstruction (_typeFactory->integerType(), instructionToConvert, _currentBasicBlock);
			break;
        case OP_coerce_i:
			result = new (gc) CoerceInstruction(_typeFactory->integerType(), instructionToConvert, _currentBasicBlock);
			break;
        case OP_coerce_u:
			result = new (gc) CoerceInstruction(_typeFactory->uintType(), instructionToConvert, _currentBasicBlock);
			break;
        case OP_coerce_d:
			result = new (gc) CoerceInstruction(_typeFactory->numberType(), instructionToConvert, _currentBasicBlock);
			break;
		default:
			TessaAssert(false);
		}

		TessaAssert(result != NULL);
		pushInstruction(result);
	}

	Type* AbcToTessaTranslator::getType(Traits* traitsType) {
		avmplus::BuiltinType builtinType = Traits::getBuiltinType(traitsType);

		switch (builtinType) 
		{
        case BUILTIN_number:
			return _typeFactory->numberType();
        case BUILTIN_any:
			return _typeFactory->anyType();
        case BUILTIN_object:
			return _typeFactory->objectType();
        case BUILTIN_boolean:
			return _typeFactory->boolType();
		case BUILTIN_string:
			return _typeFactory->stringType();
		case BUILTIN_int:
			return _typeFactory->integerType();
		case BUILTIN_uint:
			return _typeFactory->uintType();
		case BUILTIN_array:
			return _typeFactory->anyArrayType();
		case BUILTIN_vector:
			return _typeFactory->anyVectorType();
		case BUILTIN_vectordouble:
			return _typeFactory->numberVectorType();
		case BUILTIN_vectorint:
			return _typeFactory->intVectorType();
		case BUILTIN_vectorobj:
			return _typeFactory->objectVectorType();
        case BUILTIN_vectoruint:
			return _typeFactory->uintVectorType();
        case BUILTIN_namespace:
		{
			TessaAssert(false);
		}
        default:
		{
			if (isScriptObject(traitsType)) {
				return _typeFactory->scriptObjectType();
			} else {
				return _typeFactory->anyType();
			}
		}
		} // End switch
	}

	/*
	int AbcToTessaTranslator::getTessaType(Traits* type) {
		avmplus::BuiltinType builtinType = Traits::getBuiltinType(type);
		switch (builtinType) 
		{
        case BUILTIN_number:
			return _typeFactory->numberType();
        case BUILTIN_any:
			return TESSA_ANY;
        case BUILTIN_object:
			return _typeFactory->objectType();
        case BUILTIN_void:
            return TESSA_VOID;  // value already represented as Atom
        case BUILTIN_int:
			return _typeFactory->integerType();
        case BUILTIN_uint:
			return _typeFactory->uintType();
        case BUILTIN_boolean:
			return _typeFactory->boolType();
		case BUILTIN_vector:
			return VECTOR;
        case BUILTIN_string:
			return _typeFactory->stringType()();
		case BUILTIN_vectordouble:
			return &TessaTypes::numberVectorType;
		case BUILTIN_vectorint:
			return &TessaTypes::intVectorType;
		case BUILTIN_vectorobj:
			return &TessaTypes::objectVectorType;
        case BUILTIN_vectoruint:
			return &TessaTypes::uintVectorType;
        case BUILTIN_namespace:
		{
			TessaAssert(false);
		}
		case BUILTIN_array:
			return &TessaTypes::anyArrayType;

        default:
		{
			if (isScriptObject(type)) {
				return _typeFactory->scriptObjectType();
			} else {
				return TESSA_ANY;
			}
		}
        }
	}
	*/

	Traits* AbcToTessaTranslator::getSlotTraits(Traits* objectTraits, int slotIndex) {
		return objectTraits ? objectTraits->getTraitsBindings()->getSlotTraits(slotIndex) : NULL;
	}

	bool AbcToTessaTranslator::isScriptObject(Traits* traits) {
		return (traits && !traits->isMachineType() && traits != STRING_TYPE && traits != NAMESPACE_TYPE && traits != NULL_TYPE);
	}

	GetSlotInstruction* AbcToTessaTranslator::emitGetSlot(int slot, Traits* objectTraits, Traits* slotTraits) {
		TessaInstruction*	receiverInstruction = popInstruction();

		const TraitsBindingsp traitsBinding = objectTraits->getTraitsBindings();
        int offset = traitsBinding->getSlotOffset(slot);
		Type* tessaSlotType = getType(slotTraits);
		GetSlotInstruction* getSlotInstruction = new (gc) GetSlotInstruction(slot, offset, receiverInstruction, _currentBasicBlock);
		getSlotInstruction->setType(tessaSlotType);
		pushInstruction(getSlotInstruction);

		return getSlotInstruction;
	}

	void AbcToTessaTranslator::emitSetSlot(int slot, Traits* objectTraits, Traits* slotTraits) {
		const TraitsBindingsp traitsBinding = objectTraits->getTraitsBindings();
        int offset = traitsBinding->getSlotOffset(slot); 
		Type* tessaSlotType = getType(slotTraits);

		TessaInstruction* valueToSet = popInstruction();
		TessaInstruction* receiverObject = popInstruction();
		SetSlotInstruction* setSlotInstruction = new (gc) SetSlotInstruction(slot, offset, receiverObject, valueToSet, slotTraits, _currentBasicBlock);
		setSlotInstruction->setType(tessaSlotType);
	}

	bool AbcToTessaTranslator::isIntegerIndex(const Multiname* propertyName, Traits* indexTraits) {
		bool attribute = propertyName->isAttr();
		bool maybeIntegerIndex = !attribute && propertyName->isRtname() && propertyName->containsAnyPublicNamespace();
		return maybeIntegerIndex && (indexTraits == INT_TYPE);
	}

	bool AbcToTessaTranslator::isUnsignedIntegerIndex(const Multiname* propertyName, Traits* indexTraits) {
		bool attribute = propertyName->isAttr();
		bool maybeIntegerIndex = !attribute && propertyName->isRtname() && propertyName->containsAnyPublicNamespace();
		return maybeIntegerIndex && (indexTraits == UINT_TYPE);
	}

	bool AbcToTessaTranslator::canUsePropertyCache(const Multiname* propertyName) {
		return !propertyName->isRuntime();
	}

	bool AbcToTessaTranslator::isArrayObject(Traits* objectTraits) {
		return objectTraits == ARRAY_TYPE;
	}

	bool AbcToTessaTranslator::isObjectVector(Traits* objectTraits) {
		return (objectTraits != NULL) && (objectTraits->subtypeof(VECTOROBJ_TYPE));
	}

	bool AbcToTessaTranslator::isIntVector(Traits* objectTraits) {
		return objectTraits == VECTORINT_TYPE;
	}

	bool AbcToTessaTranslator::isUIntVector(Traits* objectTraits) {
		return objectTraits == VECTORUINT_TYPE;
	}

	bool AbcToTessaTranslator::isDoubleVector(Traits* objectTraits) {
		return objectTraits == VECTORDOUBLE_TYPE;
	}

	bool AbcToTessaTranslator::isIntegerType(Traits* traits) {
		return traits== INT_TYPE;
	}

	bool AbcToTessaTranslator::isNumberType(Traits* traits) {
		return traits == NUMBER_TYPE;
	}

	bool AbcToTessaTranslator::isUnsignedIntegerType(Traits* traits) {
		return traits == UINT_TYPE;
	}

	Type* AbcToTessaTranslator::getIndexType(const Multiname* propertyName, Traits* indexTraits) {
		bool maybeIntegerIndex = isIntegerIndex(propertyName, indexTraits);
		if (maybeIntegerIndex && (indexTraits == INT_TYPE)) {
			return _typeFactory->integerType();
		} else if (maybeIntegerIndex && (indexTraits == UINT_TYPE)) {
			return _typeFactory->uintType();
		} else {
			return _typeFactory->anyType();
		}
	}

	/***
	 * We couldn't quite early bind, although we still can a little bit here
	 */
	void AbcToTessaTranslator::emitGetPropertySlow(const Multiname* multiname, const FrameState* state, Binding* binding, Traits* indexTraits, Traits* objectTraits, Traits* resultTraits) {
		(void)binding;
		(void)state;
		GetPropertyInstruction* getPropertyInstruction;

		if (canUsePropertyCache(multiname)) {
			TessaInstruction* receiverObject = popInstruction();
			TessaInstruction* propertyKey = NULL;
			getPropertyInstruction = new (gc) GetPropertyInstruction(receiverObject, propertyKey, multiname, indexTraits, _currentBasicBlock);

			GetCache* cacheSlot = _get_cache_builder.allocateCacheSlot(multiname);
			getPropertyInstruction->getCacheSlot = cacheSlot;
			getPropertyInstruction->getCacheGetHandlerOffset = offsetof(GetCache, get_handler);
			getPropertyInstruction->usePropertyCache = true;
		} else {
			TessaInstruction* propertyKey = popInstruction();
			TessaInstruction* receiverObject = popInstruction();
			getPropertyInstruction = new (gc) GetPropertyInstruction(receiverObject, propertyKey, multiname, indexTraits, _currentBasicBlock);	
		}

		getPropertyInstruction->indexType = getIndexType(multiname, indexTraits);
		getPropertyInstruction->objectType = getType(objectTraits);
		Type* getPropertyResultType = getType(resultTraits);
		getPropertyInstruction->setType(getPropertyResultType);

		getPropertyInstruction->objectTraits = objectTraits;
		getPropertyInstruction->resultTraits = resultTraits;
		TessaAssert(getPropertyInstruction != NULL);
		pushInstruction(getPropertyInstruction);
	}

	void AbcToTessaTranslator::emitSetPropertySlow(const Multiname* multiname, const FrameState* state, Binding *binding, Traits* objectTraits) {
		(void)binding;
		int sp = state->sp();

		TessaInstruction* valueToSet = popInstruction();
		TessaInstruction* propertyKey = NULL;
		TessaInstruction* receiverObject = NULL;

		Traits* valueTraits = state->value(sp).traits;
		Traits* indexTraits = state->value(sp - 1).traits;
		
		if (multiname->isRuntime()) {
			propertyKey = popInstruction();
			receiverObject = popInstruction();
		} else {
			receiverObject = popInstruction();
		}

		SetPropertyInstruction* setPropertyInstruction = new (gc) SetPropertyInstruction(receiverObject, propertyKey, valueToSet, multiname, indexTraits, _currentBasicBlock);
		setPropertyInstruction->objectTraits = objectTraits;
		setPropertyInstruction->valueTraits = valueTraits;

		if (canUsePropertyCache(multiname)) {
			SetCache* cacheSlot = _set_cache_builder.allocateCacheSlot(multiname);
			setPropertyInstruction->setCacheSlot = cacheSlot;
			setPropertyInstruction->setCacheHandlerOffset = offsetof(SetCache, set_handler);
			setPropertyInstruction->usePropertyCache = true;
		}
	}

	void AbcToTessaTranslator::emitInitProperty(const Multiname* multiname, const FrameState* state, TessaInstruction* namespaceInstruction, Traits* objectTraits) {
		TessaInstruction* valueToInit = popInstruction();
		TessaInstruction* receiverObject = NULL;
		int sp = state->sp();

		Traits* valueTraits = state->value(sp).traits;
		Traits* indexTraits = state->value(sp - 1).traits;

		if (multiname->isRtns() || multiname->isRtname()) {
			namespaceInstruction = popInstruction();	
			receiverObject = popInstruction();
		} else {
			receiverObject = popInstruction();
		}

		InitPropertyInstruction* initPropertyInstruction = new (gc) InitPropertyInstruction(receiverObject, namespaceInstruction, valueToInit, multiname, indexTraits, _currentBasicBlock);
		initPropertyInstruction->objectTraits = objectTraits;
		initPropertyInstruction->valueTraits = valueTraits;
	}

	/***
	 * Takes the number of arguments +1 and pops those items off the stack
	 * IT then creates an ArrayOfInstructions* and puts those arguments in the correct order in the new instruction
	 * The operand stack has a side effect that the argCount + 1 values are popped off
	 * The +1 is for the receiverObject
	 */
	ArrayOfInstructions* AbcToTessaTranslator::createMethodArguments(TessaInstruction* receiverObject, uint32_t argCount) {
		ArrayOfInstructions* arguments = new (gc) ArrayOfInstructions(gc, _currentBasicBlock);
		TessaAssert(receiverObject != NULL);
		// First argument is ALWAYS receiver, or "this", then the real arguments
		arguments->addInstruction(receiverObject);

		// Arguments then proceed in reverse order. Everything in the VM expects arguments to be:
		//    0      1       2       3
		//  "this"  last   middle   first
		for (int i = argCount; i > 0; i--) { // Start at arg count because the 0th item is "this"
			arguments->setInstruction(i, popInstruction());
		}

		TessaInstruction* currentBaseInstruction = popInstruction();	// Pop off the base instruction
		TessaAssert(currentBaseInstruction == receiverObject);
		return arguments;
	}

	void AbcToTessaTranslator::emitCallInstruction(uint32_t argCount, uintptr_t disp_id, MethodInfo* methodInfo, Traits* resultsTraits) {
		TessaInstruction* receiverObject = this->peekInstruction(argCount);	
		ArrayOfInstructions* callArguments = this->createMethodArguments(receiverObject, argCount);
		Type* returnType = getType(resultsTraits);

		LoadVirtualMethodInstruction* loadVirtualMethod = new (gc) LoadVirtualMethodInstruction(gc, receiverObject, disp_id, _currentBasicBlock);
		CallVirtualInstruction* callInstruction = new (gc) CallVirtualInstruction(receiverObject, loadVirtualMethod, resultsTraits, callArguments, disp_id, methodInfo, _currentBasicBlock);
		callInstruction->setType(returnType);
		pushInstruction(callInstruction);
	}

	void AbcToTessaTranslator::emitCallDynamicMethod(uint32_t argCount, Traits* resultTraits) {
		TessaInstruction* receiverObject = this->peekInstruction(argCount);	
		ArrayOfInstructions* callArguments = this->createMethodArguments(receiverObject, argCount);
		TessaInstruction* functionObject = popInstruction();
		CallInstruction* callInstruction = new (gc) CallInstruction(functionObject, receiverObject, resultTraits, callArguments, _currentBasicBlock);
		pushInstruction(callInstruction);
	}

	void AbcToTessaTranslator::emitConstructSuper(uint32_t argCount, MethodInfo* methodInfo, Traits* receiverType) {
		TessaInstruction* receiverObject = this->peekInstruction(argCount);
		ArrayOfInstructions* constructArguments = this->createMethodArguments(receiverObject, argCount);
		TessaInstruction* constructSuperInstruction = new (gc) ConstructSuperInstruction(receiverObject, receiverType, constructArguments, methodInfo, _currentBasicBlock); 
		// Construct super doesn't return anything on the stack
	}

	void AbcToTessaTranslator::emitNewObjectInstruction(uint32_t argCount) {
		ArrayOfInstructions* objectProperties = new (gc) ArrayOfInstructions(gc, _currentBasicBlock); 
		TessaAssert((argCount % 2) == 0);

		// Arguments then proceed in reverse order. Everything in the VM expects arguments to be:
		for (int i = argCount - 1; i >= 0; i--) { // Start at arg count because the 0th item is "this"
			objectProperties->setInstruction(i, popInstruction());
		}

		TessaInstruction* newObjectInstruction = new (gc) NewObjectInstruction(objectProperties, _currentBasicBlock); 
		newObjectInstruction->setType(_typeFactory->scriptObjectType());
		pushInstruction(newObjectInstruction);
	}

	void AbcToTessaTranslator::emitConstruct(uint32_t argCount, Traits* classTraits) {
		TessaInstruction* receiverObject = this->peekInstruction(argCount);

		ArrayOfInstructions* callArguments = this->createMethodArguments(receiverObject, argCount);
		ConstructInstruction* constructInstruction =  new (gc) ConstructInstruction(receiverObject, classTraits, callArguments, NULL, _currentBasicBlock); 
		constructInstruction->setType(_typeFactory->objectType());

		TessaAssert(constructInstruction != NULL);
		pushInstruction(constructInstruction);
	}

	void AbcToTessaTranslator::emitConstructProperty(uint32_t argCount, const Multiname* multiname, TessaInstruction* receiverInstruction, const FrameState* state, Traits* receiverType) {
		const Value& avmObject = state->peek(argCount + 1); 
		Toplevel *toplevel = state->verifier->getToplevel();
        Binding binding = toplevel->getBinding(avmObject.traits, multiname);
		ArrayOfInstructions* constructArguments = createMethodArguments(receiverInstruction, argCount);

		ConstructPropertyInstruction* constructPropertyInstruction = new (gc) ConstructPropertyInstruction(receiverInstruction, receiverType, constructArguments, multiname, _currentBasicBlock); 
		if (AvmCore::isSlotBinding(binding)) {
			// Retain enough info to early bind
			int slotId = AvmCore::bindingToSlotId(binding);
            int constructorIndex = state->sp() - argCount;
			constructPropertyInstruction->setSlotId(slotId);
			constructPropertyInstruction->setConstructorIndex(constructorIndex);
			constructPropertyInstruction->setObjectTraits(avmObject.traits);

			if (receiverType != NULL) {
				Type* constructResult = getType(receiverType->itraits);
				constructPropertyInstruction->setType(constructResult);
			}
		} 

		pushInstruction(constructPropertyInstruction);
	}

	CallPropertyInstruction* AbcToTessaTranslator::emitSlotBoundCallProperty(GetSlotInstruction* functionValue, uint32_t argCount, const Multiname* propertyName, Traits* receiverType) {
		TessaInstruction* receiverInstruction = this->peekInstruction(argCount);	
		ArrayOfInstructions* callArguments = this->createMethodArguments(receiverInstruction, argCount);
		CallPropertyInstruction* callPropertyInstruction = new (gc) CallPropertyInstruction(functionValue, receiverInstruction, receiverType, callArguments, propertyName, _currentBasicBlock); 
		pushInstruction(callPropertyInstruction);

		return callPropertyInstruction;
	}

	void AbcToTessaTranslator::emitCallProperty(uint32_t argCount, const Multiname* propertyName, Traits* receiverType, CallCache* cacheSlot) {
		TessaInstruction* receiverInstruction = this->peekInstruction(argCount);	
		ArrayOfInstructions* callArguments = this->createMethodArguments(receiverInstruction, argCount);
		CallPropertyInstruction* callPropertyInstruction = new (gc) CallPropertyInstruction(receiverInstruction, receiverType, callArguments, propertyName, _currentBasicBlock); 
		callPropertyInstruction->cacheSlot = cacheSlot;
		callPropertyInstruction->cacheHandlerOffset = offsetof(CallCache, call_handler);
		pushInstruction(callPropertyInstruction);
	}

	void AbcToTessaTranslator::emitCallSuper(uint32_t multinameIndex, uint32_t argCount, Traits* baseType, const FrameState* state, Traits* resultTraits) {
		const TraitsBindingsp baseTraitsBinding = baseType->getTraitsBindings();
        const Multiname *multiname = _poolObject->precomputedMultiname(multinameIndex);
        Toplevel* toplevel = state->verifier->getToplevel();
        Binding binding = toplevel->getBinding(baseType, multiname);

		TessaInstruction* receiverInstruction = this->peekInstruction(argCount);	
		ArrayOfInstructions* callArguments = this->createMethodArguments(receiverInstruction, argCount);
		CallSuperInstruction* callSuperInstruction;

		if (AvmCore::isMethodBinding(binding)) {
            int methodId = AvmCore::bindingToMethodId(binding);
            MethodInfo* methodInfo  = baseTraitsBinding->getMethod(methodId);
			callSuperInstruction = new (gc) CallSuperInstruction(receiverInstruction, resultTraits, callArguments, methodId, methodInfo, multiname, _currentBasicBlock); 
        }
        else {
			callSuperInstruction = new (gc) CallSuperInstruction(receiverInstruction, resultTraits, callArguments, multiname, _currentBasicBlock);
        }

		pushInstruction(callSuperInstruction);
	}

	void AbcToTessaTranslator::emitNewArray(uint32_t numberOfElements) {
		List<TessaInstruction*, avmplus::LIST_GCObjects> *arrayElements = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc);

		TessaInstruction** arrayArguments = new TessaInstruction*[numberOfElements];
		for (int32_t i = numberOfElements - 1; i >= 0; i--) {
			arrayArguments[i] = popInstruction();
		}
		/***
		 * Arguments are in reverse order on the stack
		 */
		for (uint32_t i = 0; i < numberOfElements; i++) {
			arrayElements->add(arrayArguments[i]);
		}

		delete arrayArguments;
		NewArrayInstruction* newArray = new (gc) NewArrayInstruction(arrayElements, _currentBasicBlock); 
		newArray->setType(_typeFactory->anyArrayType()); 
		pushInstruction(newArray);
	}

	void AbcToTessaTranslator::emitNextNameInstruction() {
		TessaInstruction* registerFile = popInstruction();
		TessaInstruction* receiverObject = popInstruction();
		pushInstruction(_tessaInstructionCreator->createNextNameInstruction(receiverObject, registerFile));
	}

	void AbcToTessaTranslator::emitHasNextTwo(int objectIndex, int registerIndex) {
		TessaInstruction* objectReference = _tessaInstructionCreator->getLocalVariable(objectIndex);
		TessaInstruction* registerReference = _tessaInstructionCreator->getLocalVariable(registerIndex);
		
		HasMorePropertiesObjectInstruction*		hasMorePropertiesObject = _tessaInstructionCreator->createHasMorePropertiesObjectInstruction(objectReference);
		HasMorePropertiesRegisterInstruction*	hasMorePropertiesRegister = _tessaInstructionCreator->createHasMorePropertiesRegisterInstruction(registerReference);
		HasMorePropertiesInstruction*			hasMorePropertiesInstruction = _tessaInstructionCreator->createHasMorePropertiesInstruction(hasMorePropertiesObject, hasMorePropertiesRegister);

		/***
		 * The register is set incorrectly. The local variables will refer to the has more properties register
		 * rather than the real value of the new register
		 */
		_tessaInstructionCreator->setLocalVariable(objectIndex, hasMorePropertiesObject);
		_tessaInstructionCreator->setLocalVariable(registerIndex, hasMorePropertiesRegister);
		pushInstruction(hasMorePropertiesInstruction);
	}

	void AbcToTessaTranslator::emitTypeOfInstruction(AbcOpcode abcOpcode) {
		TessaAssert(abcOpcode == OP_istypelate);
		(void) abcOpcode;
		TessaInstruction* typeToCompare = popInstruction();
		TessaInstruction* valueToCheck = popInstruction();
		pushInstruction(new (gc) TypeOfInstruction(valueToCheck, typeToCompare, _currentBasicBlock)); 
	}

	/***
	 * There is no "this" pointer in OP_newfunction.
	 * Model this as a call with one argument.
	 * The one argument is the integer of the method id
	 */
	void AbcToTessaTranslator::emitNewFunction(uint32_t methodId) {
		TessaInstruction* methodIdInstruction = createConstantValueInstruction(createConstantInteger(methodId), _currentBasicBlock); 
		ArrayOfInstructions* callArguments = new (gc) ArrayOfInstructions(gc, _currentBasicBlock); 
		callArguments->addInstruction(methodIdInstruction);

		CallInstruction* newFunctionInstruction = new (gc) CallInstruction(NULL, NULL, NULL, callArguments, _currentBasicBlock); 
		newFunctionInstruction->setAbcOpcode(OP_newfunction);
		newFunctionInstruction->setType(_typeFactory->scriptObjectType());	// New function returns ClassClosure*
		pushInstruction(newFunctionInstruction);
	}

	void AbcToTessaTranslator::emitAbcOpcodeCall(AbcOpcode opcode, uint32_t argCount) {
		TessaInstruction* factoryObject = this->peekInstruction(argCount);
		ArrayOfInstructions* callArguments = this->createMethodArguments(factoryObject, argCount);
		CallInstruction* callInstruction = new (gc) CallInstruction(factoryObject, NULL, NULL, callArguments, _currentBasicBlock);
		callInstruction->setAbcOpcode(opcode);
		pushInstruction(callInstruction);
	}

	void AbcToTessaTranslator::emitGetScopeObject(int32_t index, bool isOuterScope) {
		GetScopeObjectInstruction* getScopeInstruction = new (gc) GetScopeObjectInstruction(index, isOuterScope, _currentBasicBlock); 
		getScopeInstruction->setType(_typeFactory->scriptObjectType());
		pushInstruction(getScopeInstruction);
	}

	void AbcToTessaTranslator::emitKill(int32_t index) {
		if (index < _numberOfLocals) {
			TessaInstruction* newReference = createConstantValueInstruction(createConstantUndefined(), _currentBasicBlock);
			newReference->setType(_typeFactory->undefinedType());
			_tessaInstructionCreator->setLocalVariable(index, newReference);
		} else {
			TessaAssert(false);
		}
	}

	void AbcToTessaTranslator::emitSwitch(int32_t numberOfCases, int defaultPc, int* caseTargets) {
		TessaInstruction* switchValue = popInstruction();
		List<CaseInstruction*, avmplus::LIST_GCObjects> caseInstructions(gc, numberOfCases); 

		for (int32_t caseIndex = 0; caseIndex < numberOfCases; caseIndex++) {
			BasicBlock* caseBasicBlock = getBasicBlockAtPc(caseTargets[caseIndex]);
			TessaInstruction* caseValue = createConstantValueInstruction(createConstantInteger(caseIndex), _currentBasicBlock); 
			CaseInstruction* switchCase = new (gc) CaseInstruction(caseValue, caseBasicBlock, _currentBasicBlock); 
			caseInstructions.add(switchCase);
		}
		TessaAssert(caseInstructions.size() == numberOfCases);

		// The default case is ALWAYS last
		BasicBlock* defaultBasicBlock = getBasicBlockAtPc(defaultPc);
		CaseInstruction* defaultCase = new (gc) CaseInstruction(NULL, defaultBasicBlock, _currentBasicBlock); 

		/*** 
		 * Create the switch at the end since switch terminates a basic block
		 */
		SwitchInstruction* switchInstruction = new (gc) SwitchInstruction(gc, switchValue, _currentBasicBlock); 
		for (int32_t caseIndex = 0; caseIndex < numberOfCases; caseIndex++) {
			CaseInstruction* caseInstruction = caseInstructions.get(caseIndex);
			caseInstruction->setSwitchInstruction(switchInstruction);
			switchInstruction->addCaseInstruction(caseInstruction);
		}

		// Default case is ALWAYS last
		defaultCase->setSwitchInstruction(switchInstruction);
		switchInstruction->setDefaultCase(defaultCase);
	}

	/***
	 * Copied from verifier->parseBodyHeader()
	 */
	void AbcToTessaTranslator::parseBodyHeader(const uint8_t* startPosition) {
		const byte* pos = startPosition;
        AvmCore::skipU32(pos, 4);
        _abcCodeLength = AvmCore::readU32(pos);
        _abcStartPosition = pos;
	}

	/***
	 * Go through the whole ABC for a method and recreate teh control flow graph
	 */
	void AbcToTessaTranslator::switchBasicBlocksIfPcIsNewBasicBlock(int abcPc) {
		BasicBlock* oldBasicBlock = _currentBasicBlock;

		/***
		 * We don't want to actually create a basic block at the PC, which is what getBasicBlockAtPc does.
		 * So check ourselves to see if it already exists
		 */
		if (_pcLabelMap->containsKey(abcPc)) { 
			_currentBasicBlock = getBasicBlockAtPc(abcPc);
		}
	}

	void AbcToTessaTranslator::recreateControlFlowGraph(const uint8_t* methodInfoCodeStart) {
        uint32_t imm30 = 0, imm30b = 0;
        int32_t imm8 = 0, imm24 = 0;
		parseBodyHeader(methodInfoCodeStart);
		const byte* codeEnd = _abcStartPosition + _abcCodeLength;

		char methodName[512];

#ifdef DEBUG
		avmplus::Stringp methodNameStringP = _methodInfo->getMethodName();
		StUTF8String cString(methodNameStringP);
		VMPI_snprintf(methodName, sizeof(methodName), "%s", cString.c_str());
#else
		VMPI_snprintf(methodName, sizeof(methodName), "");
#endif

		_currentBasicBlock = _tessaInstructionCreator->createNewFunction(_methodInfo, methodName, verifier->local_count, verifier->max_scope);
		_pcLabelMap->put(0, _currentBasicBlock);
		int returnCount = 0;

		if (core->config.tessaVerbose) {
			printf("Tessa creating control flow\n");
		}

		for (const avmplus::byte *pc = _abcStartPosition, *nextpc = pc; pc < codeEnd; pc = nextpc) {
			AvmCore::readOperands(nextpc, imm30, imm24, imm30b, imm8);
			AbcOpcode opcode = (AbcOpcode) *pc;

#ifdef DEBUG
			int offset = int(pc - _abcStartPosition);
			printOpcode(pc, opcode, offset);
#endif

			int intPc = pc - _abcStartPosition;
			switchBasicBlocksIfPcIsNewBasicBlock(intPc);

			/***
			 * Only care about branch instructions
			 */
			switch (opcode) {
            case OP_iflt:
            case OP_ifle:
            case OP_ifgt:
            case OP_ifge:
            case OP_ifeq:
            case OP_ifstricteq:
            case OP_iffalse:
			case OP_ifnlt:
            case OP_ifnle:
            case OP_ifngt:
			case OP_ifne:
            case OP_ifnge:
			case OP_ifstrictne:
			case OP_iftrue:
			{
				int32_t offset = imm24;
				int falseTargetPc = int(pc + 4 + offset - _abcStartPosition);	// 4 is the size of a avmplus::byte
				int trueTargetPc = int(pc + 4 - _abcStartPosition);	// True always falls through

				BasicBlock* trueBasicBlock = getBasicBlockAtPc(trueTargetPc);
				BasicBlock* falseBasicBlock = getBasicBlockAtPc(falseTargetPc);

				// We fall through to the true
				_currentBasicBlock = trueBasicBlock;
                break;
			}
			
            case OP_jump:
			{
				int32_t offset = (int32_t) imm24;
				const avmplus::byte * byteTargetPc = nextpc + imm24;
				int targetPc = int(byteTargetPc - _abcStartPosition);

				BasicBlock* jumpTarget = getBasicBlockAtPc(targetPc);
				AbcOpcode nextOpcode = (AbcOpcode)*nextpc; 

				_currentBasicBlock = getBasicBlockAtPc(nextpc - _abcStartPosition);
				break;
			}

            case OP_lookupswitch:
			{
				int currentPc = int(pc - _abcStartPosition);
				int defaultCasePc = int(pc - _abcStartPosition + imm24);
				int numberOfCases = imm30b;

                // Compute address of jump table
				const byte* jumpTablePc = pc + 4; 
                AvmCore::readU32(jumpTablePc);  // skip count

				for (int currentCase = 0; currentCase < numberOfCases; currentCase++) {
					int targetPc = currentPc + AvmCore::readS24(jumpTablePc + (3 * currentCase));	// Index found from CodegenLIR
					BasicBlock* targetBlock = this->getBasicBlockAtPc(targetPc);
				}

				BasicBlock* caseDefaultBlock = getBasicBlockAtPc(defaultCasePc);
				break;
			}
			case OP_abs_jump:
			{
				int jumpTarget = imm30;
				uint32_t codeLength = imm30b;
				// We should only see absolute jumps at the VERY FIRST basic block
				TessaAssert(this->_currentBasicBlock->getBasicBlockId() == 0);

				#ifdef AVMPLUS_64BIT
				TessaAssert(false);
	            #else
	            const byte* new_pc = (const byte*) jumpTarget;
	            const byte* new_code_end = new_pc + codeLength;
	            #endif

				this->_abcStartPosition= new_pc;
	            this->_abcCodeLength = int(new_code_end - new_pc);
				codeEnd = new_code_end;

				// Since we're iterating over everything already, have to control the loop up top to start from the new jump location
				// God awful hack
				nextpc = new_pc;	
	            break;
			}

            case OP_throw:
				TessaAssert(false);
				break;

            case OP_returnvalue:
            case OP_returnvoid:
				returnCount++;
				_currentBasicBlock->_terminatesWithReturn = true;
				break;

			default:
				break;
			}
		}

		if (returnCount > 1) {
			_hasMultipleReturnValues = true;
			_returnBlock = _tessaInstructionCreator->createNewBasicBlock();
		}

		if (core->config.tessaVerbose) {
			printResults();	
		}
	}	// End recreate control flow

	/****
	 * We do the same control flow as CodegenLIR.
	 * We still keep track of the avm2 stack by spec pushing/popping newly created Tessa Instructions
	 * Thus we must ensure that at every step along the way, the stacks are identical
	 */
	void AbcToTessaTranslator::write(const FrameState* state, const avmplus::byte* pc, AbcOpcode opcode, Traits *type) {
#ifdef DEBUG
		int offset = int(pc - _abcStartPosition);
		printOpcode(pc, opcode, offset);
#endif

		int currentStackSize = getStackSize();
		int stateSize = state->size();
		TessaAssert(currentStackSize == stateSize);	
        int sp = state->sp();
        const byte* nextpc = pc;
		unsigned int imm30=0, imm30b=0;
        int imm8=0, imm24=0;
        AvmCore::readOperands(nextpc, imm30, imm24, imm30b, imm8);

        switch (opcode) {
		case OP_pop:
		{
			popInstruction();
			break;
		}
        case OP_nop:
        case OP_label:
		{
	        // do nothing
            break;
		}
        case OP_getlocal0:
        case OP_getlocal1:
        case OP_getlocal2:
        case OP_getlocal3:
            imm30 = opcode-OP_getlocal0;
            // hack imm30 and fall through
        case OP_getlocal:
		{
			int localSlot = imm30;
			Type* localType = getType(state->value(localSlot).traits);
			TessaInstruction* currentReference = _tessaInstructionCreator->getLocalVariable(localSlot);
			currentReference->setType(localType);
			pushInstruction(currentReference);
            break;
		}
        case OP_setlocal0:
        case OP_setlocal1:
        case OP_setlocal2:
        case OP_setlocal3:
            imm30 = opcode-OP_setlocal0;
            // hack imm30 and fall through
        case OP_setlocal:
		{
			int localSlot = imm30;
			Type* localType = getType(state->value(sp).traits);
			TessaInstruction* newReference = popInstruction();
			newReference->setType(localType);
			_tessaInstructionCreator->setLocalVariable(localSlot, newReference);
            break;
		}
        case OP_pushtrue:
		{
            AvmAssert(type == BOOLEAN_TYPE);
			pushInstruction(createConstantValueInstruction(createConstantBoolean(true), _currentBasicBlock)); 
            break;
		}
        case OP_pushfalse:
		{
            AvmAssert(type == BOOLEAN_TYPE);
			pushInstruction(createConstantValueInstruction(createConstantBoolean(false), _currentBasicBlock)); 
            break;
		}
        case OP_pushnull:
		{
            AvmAssert(type == NULL_TYPE);
			pushInstruction(createConstantValueInstruction(createConstantNull(), _currentBasicBlock)); 
            break;
		}
        case OP_pushundefined:
            AvmAssert(type == VOID_TYPE);
			pushInstruction(createConstantValueInstruction(createConstantUndefined(), _currentBasicBlock)); 
            break;
        case OP_pushshort:
		{
            AvmAssert(type == INT_TYPE);
			pushInstruction(createConstantValueInstruction(createConstantInteger((signed short)imm30), _currentBasicBlock)); 
            break;
		}
        case OP_pushbyte:
		{
            AvmAssert(type == INT_TYPE);
			int8_t byteValue = (int8_t)imm8;
			pushInstruction(createConstantValueInstruction(createConstantInteger((int32_t)byteValue), _currentBasicBlock)); 
            break;
		}
        case OP_pushstring:
		{
            AvmAssert(type == STRING_TYPE);
			TessaAssert(getStackSize() == state->size());
			Stringp constantString = _poolObject->getString(imm30);
			pushInstruction(createConstantValueInstruction(createConstantString(constantString), _currentBasicBlock)); 
            break;
		}
        case OP_pushnamespace:
            AvmAssert(type == NAMESPACE_TYPE);
			AvmAssert(false);
            //emitPtrConst(sp+1, pool->cpool_ns[imm30], type);
            break;
		case OP_pushint: {
            AvmAssert(type == INT_TYPE);
			int intValue = _poolObject->cpool_int[imm30]; 
			pushInstruction(createConstantValueInstruction(createConstantInteger(intValue), _currentBasicBlock)); 
            break;
		 }
        case OP_pushuint:
            AvmAssert(type == UINT_TYPE);
            //emitIntConst(sp+1, pool->cpool_uint[imm30], type);
            break;
        case OP_pushdouble:
		{
            AvmAssert(type == NUMBER_TYPE);
			double doubleValue = *(_poolObject->cpool_double[(uint32_t)imm30]);
			pushInstruction(createConstantValueInstruction(createConstantDouble(doubleValue), _currentBasicBlock)); 
            break;
		}
        case OP_pushnan:
		{
            AvmAssert(type == NUMBER_TYPE);
			double doubleValue = *((double*)atomPtr(core->kNaN)); 
			pushInstruction(createConstantValueInstruction(createConstantDouble(doubleValue), _currentBasicBlock)); 
            break;
		}
        case OP_lookupswitch:
		{
			int caseCount = int(1 + imm30b);	// By spec, there are case count + 1 case offets.
			int defaultPcOffset = state->pc + imm24;
			TessaAssert(caseCount >= 0);

            const byte* jumpTablePc = 4 + _abcStartPosition + state->pc;
            AvmCore::readU32(jumpTablePc );  // skip count

			/***
			 * case count also includes the default case. Let's remove that. Taken from CodegenLIR
			 */
		    // Delete any trailing table entries that == default case (effective for asc output)
			while (caseCount > 0 && defaultPcOffset == (state->pc + AvmCore::readS24(jumpTablePc + 3*(caseCount - 1)))) {
                caseCount--;
			}

			int* caseTargetPcs = new int[caseCount];

			for (int i = 0; i < caseCount; i++) {
				int targetPc = state->pc + AvmCore::readS24(jumpTablePc + (3 * i));	// Taken from CodegenLir
				caseTargetPcs[i] = targetPc;
			}

			emitSwitch(caseCount, defaultPcOffset, caseTargetPcs);
            break;
		}
        case OP_throw:
			AvmAssert(false);
			break;
        case OP_returnvalue:
        case OP_returnvoid:
		{
			emitReturnInstruction(opcode);
            break;
		}
        case OP_debugfile:
        {
            break;
        }
        case OP_dxns:
        {
			AvmAssert(false);
			/*
            Stringp str = pool->getString(imm30);  // assume been checked already
            emit(opcode, (uintptr)str);
			*/
            break;
        }
        case OP_dxnslate:
            // codgen will call intern on the input atom.
            //emit(opcode, sp);
            break;
        case OP_kill:
            emitKill(imm30);
            break;
        case OP_inclocal:
        case OP_declocal:
			TessaAssert(false);
            //emit(opcode, imm30, opcode==OP_inclocal ? 1 : -1, NUMBER_TYPE);
            break;
        case OP_inclocal_i:
        case OP_declocal_i:
			TessaAssert(false);
            //emit(opcode, imm30, opcode==OP_inclocal_i ? 1 : -1, INT_TYPE);
            break;
		case OP_equals:
        case OP_strictequals:
        case OP_lessthan:
        case OP_greaterthan:
        case OP_lessequals:
        case OP_greaterequals:
		{
			emitCondition(opcode);
            break;
		}
        case OP_getdescendants:
        {
			/*
            const Multiname *name = pool->precomputedMultiname(imm30);
            emit(opcode, (uintptr)name, 0, NULL);
			*/
            break;
        }
        case OP_checkfilter:
		{
			TessaAssert(false);
            //emit(opcode, sp, 0, NULL);
            break;
		}
        case OP_deleteproperty:
        {
			/*
            const Multiname *name = pool->precomputedMultiname(imm30);
            emit(opcode, (uintptr)name, 0, BOOLEAN_TYPE);
			*/
			TessaAssert(false);
            break;
        }
        case OP_astype:
        {
			/*
            const Multiname *name = pool->precomputedMultiname(imm30);
            Traits *t = pool->getTraits(*name, state->verifier->getToplevel(this));
            emit(OP_astype, (uintptr)t, sp, t && t->isMachineType() ? OBJECT_TYPE : t);
			*/
			TessaAssert(false);
            break;
        }
        case OP_astypelate:
        {
			/*
            const Value& classValue = state->peek(1); // rhs - class
            Traits* ct = classValue.traits;
            Traits* t = NULL;
            if (ct && (t=ct->itraits) != 0)
                if (t->isMachineType())
                    t = OBJECT_TYPE;
            emit(opcode, 0, 0, t);
			*/
			TessaAssert(false);
            break;
        }
        case OP_coerce:
        case OP_coerce_b:
        case OP_convert_b:
        case OP_coerce_o:
        case OP_coerce_a:
        case OP_convert_i:
        case OP_coerce_i:
        case OP_convert_u:
        case OP_coerce_u:
        case OP_convert_d:
        case OP_coerce_d:
        case OP_coerce_s:
		{
            AvmAssert(
                    (opcode == OP_coerce    && type != NULL) ||
                    (opcode == OP_coerce_b  && type == BOOLEAN_TYPE) ||
                    (opcode == OP_convert_b && type == BOOLEAN_TYPE) ||
                    (opcode == OP_coerce_o  && type == OBJECT_TYPE) ||
                    (opcode == OP_coerce_a  && type == NULL) ||
                    (opcode == OP_convert_i && type == INT_TYPE) ||
                    (opcode == OP_coerce_i  && type == INT_TYPE) ||
                    (opcode == OP_convert_u && type == UINT_TYPE) ||
                    (opcode == OP_coerce_u  && type == UINT_TYPE) ||
                    (opcode == OP_convert_d && type == NUMBER_TYPE) ||
                    (opcode == OP_coerce_d  && type == NUMBER_TYPE) ||
                    (opcode == OP_coerce_s  && type == STRING_TYPE));
			Traits* instructionToCoerceTraits = state->value(sp).traits;
			emitConvertOrCoerceInstruction(opcode, imm30, type, instructionToCoerceTraits);
            break;
		}
        case OP_istype:
        {
			TessaAssert(false);
            // expects a CONSTANT_Multiname cpool index
            // used when operator "is" RHS is a compile-time type constant
            //sp[0] = istype(sp[0], itraits);
			/*
            const Multiname *name = pool->precomputedMultiname(imm30);
            Traits* itraits = pool->getTraits(*name, state->verifier->getToplevel(this));
            LIns* obj = loadAtomRep(sp);
            LIns* out = callIns(FUNCTIONID(istype), 2, obj, InsConstPtr(itraits));
            localSet(sp, out, BOOLEAN_TYPE);
			*/
            break;
        }
        case OP_istypelate:
        {
			emitTypeOfInstruction(opcode);
            break;
        }
        case OP_convert_o:
            // NOTE check null has already been done
            break;
        case OP_applytype:
		{
            // * is ok for the type, as Vector classes have no statics
            // when we implement type parameters fully, we should do something here.
			int argCount = imm30;
			emitAbcOpcodeCall(opcode, argCount);
            break;
		}
        case OP_newobject:
		{
			// New objects come in name/value pairs. imm30 refers to the number of pairs, but we need the total number of args
			uint32_t argCount = imm30 * 2;	
			emitNewObjectInstruction(argCount);
            break;
		}
        case OP_newarray:
		{
			uint32_t numberOfElements = imm30;
			emitNewArray(numberOfElements);
            break;
		}
        case OP_newactivation:
		{
			TessaInstruction* newActivation = new (gc) NewActivationInstruction(_currentBasicBlock); 
			newActivation->setType(getType((type)));
			pushInstruction(newActivation);
            break;
		}
        case OP_newcatch:
        {
			/*
            ExceptionHandler* handler = &info->abc_exceptions()->exceptions[imm30];
            emit(opcode, 0, 0, handler->scopeTraits);
			*/
			AvmAssert(false);
            break;
        }
        case OP_popscope:
			/*
            if (haveDebugger)
                emitKill(ms->local_count() + state->scopeDepth);
			*/
			TessaAssert(false);
            break;
        case OP_getslot:
        {
			const Value& obj = state->peek(1);
            int slotIndex = imm30-1;
			Traits* objectTraits = state->value(sp).traits;
			Traits* slotTraits = getSlotTraits(objectTraits, slotIndex);
            emitGetSlot(slotIndex, objectTraits, slotTraits);
            break;
        }
        case OP_setslot:
		{
			int slotIndex = imm30 - 1;
			Traits* objectTraits = state->value(sp - 1).traits;
			Traits* slotTraits = getSlotTraits(objectTraits, slotIndex);
			emitSetSlot(slotIndex, objectTraits, slotTraits);
            break;
		}
        case OP_dup:
		{
			TessaInstruction* topOfStack = popInstruction();
			pushInstruction(topOfStack);
			pushInstruction(topOfStack);
            break;
		}
        case OP_swap:
		{
			TessaInstruction* topOfStack = popInstruction();
			TessaInstruction* belowTop = popInstruction();
			pushInstruction(topOfStack);
			pushInstruction(belowTop);
            break;
		}
        
        case OP_instanceof:
        case OP_in:
			TessaAssert(false);
            //emit(opcode, 0, 0, BOOLEAN_TYPE);
            break;
        case OP_not:
            AvmAssert(type == BOOLEAN_TYPE);
			emitUnaryInstruction(opcode);
			break;
        case OP_increment:
        case OP_decrement:
        case OP_increment_i:
        case OP_decrement_i:
		{
			pushInstruction(createConstantValueInstruction(createConstantInteger(1), _currentBasicBlock));
			emitBinaryInstruction(opcode, type);
			TessaInstruction* binaryInstruction = peekInstruction(0);
			binaryInstruction->setType(_typeFactory->integerType());
			break;
		}
		case OP_modulo:
        case OP_divide:
        case OP_multiply:
		case OP_subtract:
		{
			TessaInstruction* result = emitBinaryInstruction(opcode, type);	
			result->setType(_typeFactory->numberType());
            break;
		}

		/***
		 * The Traits* type doesn't
		 * tell the verifier that this result is an integer
		 */
		case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
        case OP_add_i:
        case OP_subtract_i:
        case OP_multiply_i:
		case OP_lshift:
        case OP_rshift:
		{
			TessaInstruction* result = emitBinaryInstruction(opcode, type);	
			result->setType(_typeFactory->integerType());
            break;
		}

		case OP_urshift:
		{
			TessaInstruction* result = emitBinaryInstruction(opcode, type);	
			result->setType(_typeFactory->uintType());
            break;
		}

		case OP_bitnot:
        case OP_negate:
			emitUnaryInstruction(opcode);
            break;
        case OP_negate_i:
		{
			TessaAssert(false);
		}
			
        case OP_typeof:
            //emit(opcode, sp, 0, STRING_TYPE);
            break;
        case OP_debugline:
        {
            #if defined(DEBUGGER) || defined(VTUNE)
            #ifdef VTUNE
            const bool do_emit = true;
            #else
            const bool do_emit = haveDebugger;
            #endif
            // we actually do generate code for these, in debugger mode
            //if (do_emit) emit(opcode, imm30);
            #endif
            break;
        }
        case OP_nextvalue:
		{
			TessaAssert(false);
			break;
		}
        case OP_nextname:
		{
			emitNextNameInstruction();
            break;
		}
        case OP_hasnext:
		{
            //emit(opcode, 0, 0, INT_TYPE);
			TessaAssert(false);
            break;
		}
        case OP_hasnext2:
		{
			int objectIndex = imm30;
			int registerIndex = imm30b;
			emitHasNextTwo(objectIndex, registerIndex);
            break;
		}
        // sign extends
        case OP_sxi1:
        case OP_sxi8:
        case OP_sxi16:
            //emit(opcode, sp, 0, INT_TYPE);
            break;
        // loads
        case OP_lix8:
        case OP_lix16:
        case OP_li8:
        case OP_li16:
        case OP_li32:
        case OP_lf32:
        case OP_lf64:
        {
			/*
            Traits* result = (opcode == OP_lf32 || opcode == OP_lf64) ? NUMBER_TYPE : INT_TYPE;
            emit(opcode, sp, 0, result);
			*/
            break;
        }
        // stores
        case OP_si8:
        case OP_si16:
        case OP_si32:
        case OP_sf32:
        case OP_sf64:
        {
            //emit(opcode, 0, 0, VOID_TYPE);
            break;
        }
        case OP_getglobalscope:
		{
			/*
			const ScopeTypeChain* scope = methodInfo->declaringScope();
	        int captured_depth = scope->size;
			if (captured_depth > 0) {
				this->emitGetScopeObject(0, false);
				//TessaAssert(false);
			} else {
	            // local scope
	            AvmAssert(state->scopeDepth > 0); // verifier checked.
				this->emitGetScopeObject(0, false);
	        }
			*/
			GetGlobalScopeInstruction* getGlobalScope = new (gc) GetGlobalScopeInstruction(_currentBasicBlock); 
			getGlobalScope->setType(_typeFactory->scriptObjectType());
			pushInstruction(getGlobalScope);
            break;
		}
        case OP_add:
        {
			emitBinaryInstruction(opcode, type);
            break;
        }
        case OP_convert_s:
		{
			emitConvertToString(state, sp);
            break;
		}
        case OP_esc_xelem:
        case OP_esc_xattr:
		{
			AvmAssert(false);
            //emit(opcode, sp, 0, STRING_TYPE);
            break;
		}
        case OP_debug:
            // ignored
            break;
        default:
            AvmAssertMsg(false, "unhandled opcode in AbcToTessaTranslator::write()");
            break;
        }
		(void)state;
		(void)pc;
		(void)opcode;
		(void)type;
	}

	void AbcToTessaTranslator::writeOp1(const FrameState* state, const avmplus::byte *pc, AbcOpcode opcode, uint32_t opd1, Traits* type) {
		 //this->state = state;
        //emitSetPc();
		int sp = state->sp();
		int currentStackSize = getStackSize();
		int stateSize = state->size();
		TessaAssert(currentStackSize == stateSize);	

#ifdef DEBUG
		int offset = int(pc - _abcStartPosition);
		printOpcode(pc, opcode, offset);
#endif

        switch (opcode) {
        case OP_iflt:
        case OP_ifle:
        case OP_ifgt:
        case OP_ifge:
		case OP_ifeq:
        case OP_ifstricteq:
		case OP_ifnlt:
        case OP_ifnle:
        case OP_ifngt:
        case OP_ifne:
        case OP_ifstrictne:
        case OP_ifnge:
		{
            int32_t offset = (int32_t) opd1;
			int trueTarget = state->pc + 4 + offset;	// 4 is the size of a avmplus::byte
			int falseTarget = state->pc + 4;	// false always falls through
			emitConditionalBranch(opcode, trueTarget, falseTarget, state);
            break;
        }
		
        case OP_iftrue:
        case OP_iffalse:
        {
			int32_t offset = (int32_t) opd1;
			int trueTarget = state->pc + 4 + offset;	// 4 is the size of a avmplus::byte
			int falseTarget = state->pc + 4;	// Always fall through
			emitBooleanBranch(opcode, trueTarget, falseTarget, state);
            break;
        }
        case OP_jump:
        {
            int32_t offset = (int32_t) opd1;
			emitUnconditionalBranch(state->pc, state->pc+4 + offset, state);
            break;
        }
        case OP_getslot:
		{
			int slotIndex = opd1;
			Traits* objectTraits = state->value(sp).traits;
            emitGetSlot(opd1, objectTraits, type);
            break;
		}
        case OP_getglobalslot: {
			/*
            int32_t dest_index = state->sp(); // verifier already incremented it
            uint32_t slot = opd1;
            emitGetGlobalScope(dest_index);
            emitGetslot(slot, dest_index, type slot type );
			*/
			TessaAssert(false);
            break;
        }
        case OP_setglobalslot:
            //emitSetslot(OP_setglobalslot, opd1, 0 /* computed or ignored */);
			TessaAssert(false);
            break;
        case OP_call:
		{
			int32_t argCount = opd1;
			emitCallDynamicMethod(argCount, type);
            break;
		}
        case OP_construct:
        {
            uint32_t argCount = opd1;
            int constructorIndex = state->sp() - argCount;
            Traits* classTraits = state->value(constructorIndex).traits;
			emitConstruct(argCount, classTraits);
            break;
        }
        case OP_getouterscope:
		{
			int scopeIndex = opd1; 
			emitGetScopeObject(scopeIndex, true);
            break;
		}
        case OP_getscopeobject:
		{
			int scopeIndex = opd1;
			emitGetScopeObject(scopeIndex, false);
            break;
		}
        case OP_newfunction:
		{
			uint32_t functionId = opd1;
            AvmAssert(_poolObject->getMethodInfo(functionId)->declaringTraits() == type);
			emitNewFunction(functionId);
            break;
		}
        case OP_pushscope:
		{
			PushScopeInstruction* pushScopeInstruction = new (gc) PushScopeInstruction(popInstruction(), _currentBasicBlock); 
			pushScopeInstruction->setType(_typeFactory->scriptObjectType());
			break;
		}
        case OP_pushwith:
			TessaAssert(false);
            //emitCopy(state->sp(), opd1);
            break;
        case OP_findpropstrict:
        case OP_findproperty:
		case OP_findpropglobal:
		case OP_findpropglobalstrict:
        {
			bool isStrict = false;
			if ((opcode == OP_findpropstrict) || (opcode == OP_findpropglobalstrict)) {
				isStrict = true;
			}

			int scopeDepth = state->scopeDepth;
			int nameIndex = opd1;
			const Multiname *multiname = _poolObject->precomputedMultiname(nameIndex);
            AvmAssert(multiname->isBinding());
			AvmAssert(!multiname->isRuntime());
			FindPropertyInstruction* findPropertyInstruction = new (gc) FindPropertyInstruction(scopeDepth, multiname, isStrict, _currentBasicBlock); 
			findPropertyInstruction->setType(_typeFactory->anyType());
			pushInstruction(findPropertyInstruction);
            break;
        }
        case OP_newclass:
        {
			/*
            Traits* ctraits = pool->getClassTraits(opd1);
            AvmAssert(ctraits == type);
            emit(opcode, (uintptr)(void*)ctraits, state->sp(), type);
			*/
            break;
        }
        case OP_finddef:
        {
			int nameIndex = opd1;
            const Multiname *multiname = _poolObject->precomputedMultiname(nameIndex);
            AvmAssert(multiname->isBinding());
			int cacheSlot = _finddef_cache_builder.allocateCacheSlot(nameIndex);
			FindPropertyInstruction* findPropertyInstruction = new (gc) FindPropertyInstruction(multiname, nameIndex, _currentBasicBlock); 
			findPropertyInstruction->setCacheSlot(cacheSlot);
			findPropertyInstruction->setType(_typeFactory->scriptObjectType());
			pushInstruction(findPropertyInstruction);
            break;
        }
        default:
            // verifier has called writeOp1 improperly
            AvmAssert(false);
            break;
        }
		(void)state;
		(void)pc;
		(void)opcode;
		(void)opd1;
		(void)type;
	}
	void AbcToTessaTranslator::writeOp2(const FrameState* state, const avmplus::byte *pc, AbcOpcode opcode, uint32_t opd1, uint32_t opd2, Traits* type) {
        int sp = state->sp();
		TessaAssert(getStackSize() == state->size());
		int offset = int(pc - _abcStartPosition);

        switch (opcode) {
        case OP_constructsuper:
		{
			int argCount = opd2;
			emitConstructSuper(argCount, _methodInfo->declaringTraits()->base->init, type);
            break;
		}
        case OP_setsuper:
        {
			/*
            const uint32_t index = opd1;
            const uint32_t n = opd2;
            Traits* base = type;
            int32_t ptrIndex = sp-(n-1);
            const Multiname* name = pool->precomputedMultiname(index);
            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(base, name);
            Traits* propType = state->verifier->readBinding(base, b);
            const TraitsBindingsp basetd = base->getTraitsBindings();
            if (AvmCore::isSlotBinding(b))
            {
                if (AvmCore::isVarBinding(b))
                {
                    int slot_id = AvmCore::bindingToSlotId(b);
                    LIns* value = coerceToType(sp, propType);
                    emitSetslot(OP_setslot, slot_id, ptrIndex, value);
                }
                // else, ignore write to readonly accessor
            }
            else
            if (AvmCore::isAccessorBinding(b))
            {
                if (AvmCore::hasSetterBinding(b))
                {
                    // Invoke the setter
                    int disp_id = AvmCore::bindingToSetterId(b);
                    MethodInfo *f = basetd->getMethod(disp_id);
                    emitCoerceCall(OP_callsuperid, disp_id, 1, f);
                }
                // else, ignore write to readonly accessor
            }
            else {
                emit(opcode, (uintptr)name);
            }
			*/
            break;
        }
        case OP_getsuper:
        {
			/*
            const uint32_t index = opd1;
            const uint32_t n = opd2;
            Traits* base = type;
            const Multiname* name = pool->precomputedMultiname(index);
            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(base, name);
            Traits* propType = state->verifier->readBinding(base, b);
            if (AvmCore::isSlotBinding(b))
            {
                int slot_id = AvmCore::bindingToSlotId(b);
                emitGetslot(slot_id, state->sp()-(n-1), propType);
            }
            else
            if (AvmCore::hasGetterBinding(b))
            {
                // Invoke the getter
                int disp_id = AvmCore::bindingToGetterId(b);
                const TraitsBindingsp basetd = base->getTraitsBindings();
                MethodInfo *f = basetd->getMethod(disp_id);
                emitCoerceCall(OP_callsuperid, disp_id, 0, f);
            }
            else {
                emit(opcode, (uintptr)name, 0, propType);
            }
			*/
            break;
        }
        case OP_callsuper:
        case OP_callsupervoid:
        {
            const uint32_t index = opd1;
            const uint32_t argCount = opd2;
            Traits* baseType = type;
			emitCallSuper(index, argCount, baseType, state, type);
            break;
        }
        case OP_constructprop:
        {
            const uint32_t argCount = opd2;
            const Multiname* multiName = _poolObject->precomputedMultiname(opd1);
			TessaInstruction* receiverObject = peekInstruction(argCount);
			emitConstructProperty(argCount, multiName, receiverObject, state, type);
            break;
        }
        case OP_getproperty:
        {
            const Multiname* multiname = _poolObject->precomputedMultiname(opd1);
            const Value& obj = state->peek(opd2); // object
            Toplevel* toplevel = state->verifier->getToplevel();
            Binding binding = toplevel->getBinding(obj.traits, multiname);
			Traits* indexTraits = state->value(sp).traits;
			Traits* objectTraits = obj.traits;

            // early bind accessor
			if (AvmCore::hasGetterBinding(binding)) {
                // Invoke the getter
                int getterId = AvmCore::bindingToGetterId(binding);
                const TraitsBindingsp objectTraitsBinding = obj.traits->getTraitsBindings();
                MethodInfo *methodInfo = objectTraitsBinding->getMethod(getterId);
                AvmAssert(methodInfo != NULL);
				int argCount = 0;

                if (!obj.traits->isInterface()) {
					//TessaAssertMessage(false, "Don't know how to deal with static methods yet\n");
					emitCallInstruction(argCount, getterId, methodInfo, type);
                }
                else {
					int interfaceId = methodInfo->iid();
					emitCallInterface(state, argCount, methodInfo, interfaceId, type);
					//TessaAssertMessage(false, "Don't know what to do with interfaces yet\n");
                }
                TessaAssert(type == methodInfo->getMethodSignature()->returnTraits());
            }
            else {
				// Means we have to go the slow route
				emitGetPropertySlow(multiname, state, &binding, indexTraits, objectTraits, type);
            }
            break;
        }
		case OP_initproperty:
        case OP_setproperty:
        {
            // opd2=n the stack offset to the reciever
            const Multiname *multiname = _poolObject->precomputedMultiname(opd1);
            const Value& obj = state->peek(opd2); // object
            Toplevel* toplevel = state->verifier->getToplevel();
            Binding binding = toplevel->getBinding(obj.traits, multiname);
			int32_t sp = state->sp();
			Traits* objectTraits = obj.traits;

			if (AvmCore::hasSetterBinding(binding)) {
				int setterId = AvmCore::bindingToSetterId(binding);
				const TraitsBindingsp objectTraitsBinding = obj.traits->getTraitsBindings();
				MethodInfo* methodInfo = objectTraitsBinding->getMethod(setterId);
				AvmAssert(methodInfo != NULL);
				int argCount = 1;	// For the value to set

				if (!obj.traits->isInterface()) {
					emitCallInstruction(argCount, setterId, methodInfo, type);
					popInstruction();	// Don't care about the result of the call, just do it
				} else {
					TessaAssert(false);
				}
			} else if (opcode == OP_setproperty) {
				// Late bound, slow route. Have to guess some more of what kind of property we're setting.
				emitSetPropertySlow(multiname, state, &binding, objectTraits);
			}
			else if (opcode == OP_initproperty) {
				/***
				 * Late bound init property. Need to be different because they have different fail semantics
				 */
				/*
				TessaInstruction* namespaceInstruction = NULL;  
				if (multiname->isRtns() || multiname->isRtname()) {
					namespaceInstruction = popInstruction();	
				}
				*/
				TessaInstruction* namespaceInstruction = NULL;  
				emitInitProperty(multiname, state, namespaceInstruction, objectTraits);
			}
            break;
        }
		/*
		case OP_initproperty:
		{
			const Multiname* multiname = _poolObject->precomputedMultiname(opd1);
			TessaInstruction* namespaceInstruction = NULL;  
			if (multiname->isRtns() || multiname->isRtname()) {
				namespaceInstruction = popInstruction();	
			}
			emitInitProperty(multiname, namespaceInstruction);
			break;
		}
		*/
        case OP_setslot:
		{
			int slotIndex = opd1;
			int traitsIndex = opd2;
			Traits* objectTraits = state->value(traitsIndex).traits;
			Traits* slotTraits = getSlotTraits(objectTraits, slotIndex);
			emitSetSlot(slotIndex, objectTraits, slotTraits);
            break;
		}
        case OP_abs_jump:
        {
            #ifdef AVMPLUS_64BIT
			TessaAssert(false);
            #else
            const byte* new_pc = (const byte*) opd1;
            const byte* new_code_end = new_pc + opd2;
            #endif
			this->_abcStartPosition= new_pc;
            this->_abcCodeLength = int(new_code_end - new_pc);
            break;
        }
        case OP_callproperty:
        case OP_callproplex:
        case OP_callpropvoid:
        {
		    // stack in: obj [ns [name]] arg1..N
            // stack out: result
            // obj = sp[-argc]
			// op1 - index into the namespace
			// op2 - the number of arguments
            int argCount = int(opd2);
			int baseObject = sp - argCount;
            const Multiname* propertyName = _poolObject->precomputedMultiname((int)opd1);
			TessaAssert(!propertyName->isRuntime());	// Otherwise we have to init

			Traits* baseTraits = state->value(baseObject).traits;
            Binding baseBindings = state->verifier->getToplevel()->getBinding(baseTraits, propertyName);

			if (AvmCore::isSlotBinding(baseBindings)) {
				// Early bound FTW
				emitCallProperty(argCount, propertyName, type, NULL);
			} else if (!propertyName->isRuntime()) {
				// Can use a cache here, although I don't think it matters right now. Check later
				CallCache* cacheSlot = _call_cache_builder.allocateCacheSlot(propertyName);
				emitCallProperty(argCount, propertyName, type, cacheSlot);
			} else {
				// Late bind, do generic
				CallCache* cacheSlot = NULL;
				emitCallProperty(argCount, propertyName, type, cacheSlot);
			}
            break;
        }
        case OP_callstatic: {
			TessaAssert(false);
			/*
            uint32_t method_id = opd1;
            uint32_t argc = opd2;
            emitTypedCall(OP_callstatic, method_id, argc, type, pool->getMethodInfo(method_id));
			*/
            break;
        }
        default:
            AvmAssert(false);
            break;
        }
		(void)state;
		(void)pc;
		(void)opcode;
		(void)type;
		(void)opd1;
		(void)opd2;
	}

	void	AbcToTessaTranslator::emitCallInterface(const FrameState* state, uint32_t argCount, MethodInfo* methodInfo, int methodId, Traits* resultsTraits) {
		(void) state;
		TessaInstruction* receiverObject = this->peekInstruction(argCount);	
		ArrayOfInstructions* callArguments = this->createMethodArguments(receiverObject, argCount);
		CallInterfaceInstruction* callInterface =  new (gc) CallInterfaceInstruction(receiverObject, resultsTraits, callArguments, methodId, methodInfo, _currentBasicBlock);
		Type* returnType = getType(resultsTraits);
		callInterface->setType(returnType);
		pushInstruction(callInterface);
	}

	void AbcToTessaTranslator::writeMethodCall(const FrameState* state, const avmplus::byte *pc, AbcOpcode opcode, MethodInfo* methodInfo, uintptr_t disp_id, uint32_t argc, Traits* type) {
		TessaAssert(getStackSize() == state->size());
		int sp = state->sp();
		switch (opcode) {
			case OP_callproperty:
	        case OP_callproplex:
	        case OP_callpropvoid:
	        {
				// This means we are at an interface call
				AvmAssert(methodInfo->declaringTraits()->isInterface());
				emitCallInterface(state, argc, methodInfo, disp_id, type);
	            break;
	        }
			case OP_callmethod:
			{
				this->emitCallInstruction(argc, disp_id, methodInfo, type);
				break;
			}
			default:
			{
				TessaAssert(false);
				break;
			}
		}
		(void)pc;
		(void)methodInfo;
		(void)type;
		(void)disp_id;
		(void)argc;
		(void)type;
	}

	/***
	 * I have no idea why this is ok... but, take whats on top of the stack.
	 * pop 2 items, and push the value back on top of the stack
	 */
	void AbcToTessaTranslator::writeNip(const FrameState* state, const avmplus::byte *pc) {
		(void)state;
		(void)pc;
		TessaInstruction* topOfStack = popInstruction();
		popInstruction();	// Throw away
		pushInstruction(topOfStack);
	}

	void AbcToTessaTranslator::writeCheckNull(const FrameState* state, uint32_t index) {
		(void)state;
		(void)index;
	}

	void AbcToTessaTranslator::writeCoerce(const FrameState* state, uint32_t index, Traits *traitsType) {
		Type* typeToCoerce = getType(traitsType);
		TessaAssert((int)index < state->verifier->frameSize);
		Traits* instructionToCoerceTraits = state->value(index).traits;
		bool needCoerceObjectAtom = this->needCoerceObject(traitsType, instructionToCoerceTraits);

		/*** 
		 * The verifier considers anything on the operand stack as a "local" variable.
		 * TESSA only considers actual ActionScript identifiers with the var keyword as a local variable.
		 * If it's an actual TESSA local var, update that, otherwise pop the value off the stack and put on the new one
		 */
		if (index < (uint32_t)_numberOfLocals) {
			TessaInstruction* localVariable = _tessaInstructionCreator->getLocalVariable(index);
			if (localVariable->getType() != typeToCoerce) {
				CoerceInstruction* coercedInstruction = new (gc) CoerceInstruction(typeToCoerce, localVariable, _currentBasicBlock);
				coercedInstruction->useCoerceObjAtom = needCoerceObjectAtom;
				coercedInstruction->resultTraits = traitsType;
				_tessaInstructionCreator->setLocalVariable(index, coercedInstruction);
			}
		} else {
			/***
			 * The way peek and modify stack take an index assumes that the index paramter
			 * occurs from the top of the stack. So to get the top element of the operand stack
			 * requires an index 0, etc.
			 * However, the index we are given here counts from the BOTTOM of the stack, 
			 * so to get the top of the stack means that "index" has to be the length of the stack
			 */
			int verifierTotal = state->stackDepth + state->verifier->max_scope + state->verifier->local_count;
			int total = _operandStack->size() + _numberOfLocals + state->verifier->max_scope;
			int stackSlot = total - index; 
			stackSlot = getStackSize() - stackSlot;

			TessaAssert(state->stackDepth == _operandStack->size());
			TessaAssert(state->verifier->local_count == _numberOfLocals);
			TessaAssert(verifierTotal == total);
			TessaAssert((int)_operandStack->size() > stackSlot);

			TessaInstruction* localVariable = _operandStack->get(stackSlot);
			if (localVariable->getType() != typeToCoerce) {
				CoerceInstruction* coercedInstruction = new (gc) CoerceInstruction(typeToCoerce, localVariable, _currentBasicBlock);
				coercedInstruction->useCoerceObjAtom = needCoerceObjectAtom;
				coercedInstruction->resultTraits = traitsType;
				_operandStack->set(stackSlot, coercedInstruction);
			}
		}
	}

	void AbcToTessaTranslator::writePrologue(const FrameState* state, const avmplus::byte *pc) {
		/***
		 * We could have an absolute jump when we recreated the control flow
		 * Absolute jump rewrites the abc start position and code length
		 * Therefore, we have to reset the start position back to the first portion of the abc we're parsing
		 * We'll see another abs_jump later which changes these variables again
		 */
		_abcStartPosition= state->verifier->code_pos;
		this->_abcCodeLength = state->verifier->code_length;

		BasicBlock* firstBasicBlock = getBasicBlockAtPc(int(pc - _abcStartPosition));
		startParsingBasicBlock(firstBasicBlock, state);
	}

	/****
	 * We make all return instructions jump to a basic block.
	 * All the return values become a phi value, at which point we return that phi instruction
	 */
	void AbcToTessaTranslator::fillReturnBlock() {
		PhiInstruction* phiReturn = new (gc) PhiInstruction(gc, _returnBlock);
		TessaAssert(!_returnValues->isEmpty());

		Type* phiType = _returnValues->get(0)->getType();
		for (uint32_t i = 0; i < _returnValues->size(); i++) {
			TessaInstruction* returnValue = _returnValues->get(i);
			if (returnValue->getType() != phiType) {
				phiType = _typeFactory->anyType();
			}

			BasicBlock* returnBlock = returnValue->getInBasicBlock();
			phiReturn->addOperand(returnBlock, returnValue);
		}

		phiReturn->setType(phiType);

		ReturnInstruction* returnInstruction = new (gc) ReturnInstruction(phiReturn, _returnBlock);
		Type* returnType = getType(_methodInfo->getMethodSignature()->returnTraits());
		returnInstruction->setType(returnType);
	}

	void AbcToTessaTranslator::writeEpilogue(const FrameState* state) {
		(void)state;
		if (core->config.tessaVerbose) {
			printf("\n=== Before SSA ===");
			this->printResults();
		}

		if (_hasMultipleReturnValues) {
			fillReturnBlock();
		}
		_tessaInstructionCreator->finishMethod(); 
		_methodInfo->set_lookup_cache_size(_finddef_cache_builder.next_cache);
	}

	void AbcToTessaTranslator::patchBranchesToTarget(avmplus::List<BranchInstruction*, avmplus::LIST_GCObjects> *unpatchedBranches, BasicBlock* target) {
		for (uint32_t i = 0; i < unpatchedBranches->size(); i++) {
			BranchInstruction* branchInstruction = unpatchedBranches->get(i);
			if (branchInstruction->isConditionalBranch()) {
				// Don't do anything yet, we already patched it when we created the conditional branch
			} else {
				TessaAssert(branchInstruction->isUnconditionalBranch());
				UnconditionalBranchInstruction* unconditionalBranch = (UnconditionalBranchInstruction*) branchInstruction;
				unconditionalBranch->setBranchTarget(target);
			}
		}
	}

	void AbcToTessaTranslator::setTypesOfLocals(BasicBlock* basicBlock, const FrameState* state) {
		List<ParameterInstruction*, LIST_GCObjects>* parameters = basicBlock->getParameterInstructions();
		int localCount = state->verifier->local_count;

		// We can be greater because of ternary operators
		TessaAssert(parameters->size() >= (uint32_t)localCount);
		for (int i = 0; i < localCount; i++) {
			ParameterInstruction* tessaLocalVariable = parameters->get(i);
			Type* localType = getType(state->value(i).traits);
			tessaLocalVariable->setType(localType);
		}
	}

	/**
	* Horrible name for a method but we have to use it to conform to the Coder class.
	* This really means a block join node, so we can do phi insertion here
	*/
	void AbcToTessaTranslator::writeBlockStart(const FrameState* state) {
		int pc = state->pc;
		List<BranchInstruction*, avmplus::LIST_GCObjects> *branchesToHere = getBranchesToPc(pc);
		BasicBlock* basicBlock = getBasicBlockAtPc(pc);

		emitUnconditionalBranchForFallThrough(pc, basicBlock, state);
		patchBranchesToTarget(branchesToHere, basicBlock);
		startParsingBasicBlock(basicBlock, state);
		loadState(state);
	}

	void AbcToTessaTranslator::writeOpcodeVerified(const FrameState* state, const avmplus::byte* pc, AbcOpcode opcode) {
		(void)state;
		(void)pc;
		(void)opcode;
	}

	void AbcToTessaTranslator::writeFixExceptionsAndLabels(const FrameState* state, const avmplus::byte* pc) {
		(void)state;
		(void)pc;
	}

	void AbcToTessaTranslator::cleanup() {
	}
}
