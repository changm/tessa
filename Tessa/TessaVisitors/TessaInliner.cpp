#include "TessaVisitors.h"

/***
 * Tessa Inliner inlines high level TESSA Instructions into other Tessa methods.
 * This is done via a two phase process:
 * 1) Iterate over each TESSA instruction in a given method, creating a shallow clone of the instruction.
 * 2) Visit the newly cloned instructions, and modifying their operands to point to the newly cloned operands
 */
namespace TessaVisitors {
	TessaInliner::TessaInliner(AvmCore* core) :
		_originalToCloneMap(256)
	{
		this->_core = core;
		this->_gc = core->gc;
		this->_inlinedAMethod = false;
		_inlineCount = 0;
	}

	void TessaInliner::visit(TessaInstruction* tessaInstruction) {
		AvmAssert(false);
	}

	void TessaInliner::visit(FindPropertyInstruction* findPropertyInstruction) {
	}

	void TessaInliner::visit(ConstantValueInstruction* constantValueInstruction) {
	}

	void TessaInliner::visit(ArrayOfInstructions* arrayOfInstructions) {
	}

	void TessaInliner::visit(ConstructInstruction* constructInstruction) {
	}

	void TessaInliner::visit(NewObjectInstruction* newObjectInstruction) {
	}

	void TessaInliner::visit(ConstructPropertyInstruction* constructPropertyInstruction) {
	}

	void TessaInliner::visit(ConstructSuperInstruction* constructSuperInstruction) {
	}

	void TessaInliner::visit(CallInterfaceInstruction* callInterfaceInstruction) {
	}

	void TessaInliner::visit(CallSuperInstruction* callSuperInstruction) {
	}

	void TessaInliner::visit(CallPropertyInstruction* callPropertyInstruction) {
	}

	void TessaInliner::visit(ReturnInstruction* returnInstruction) {
	}

	void TessaInliner::visit(BinaryInstruction* binaryInstruction) {
	}

	void TessaInliner::visit(ConditionInstruction* conditionInstruction) {
	}

	void TessaInliner::visit(TessaInstructions::UnaryInstruction* unaryInstruction) {
	}

	void TessaInliner::visit(ConditionalBranchInstruction* conditionalBranchInstruction) {
	}

	void TessaInliner::visit(UnconditionalBranchInstruction* unconditionalBranchInstruction) {
	}

	void TessaInliner::visit(PhiInstruction* phiInstruction) {
	}

	void TessaInliner::visit(ParameterInstruction* parameterInstruction) {
	}

	void TessaInliner::visit(CoerceInstruction* coerceInstruction) {
	}

	void TessaInliner::visit(ConvertInstruction* convertInstruction) {
	}

	void TessaInliner::visit(GetPropertyInstruction* getPropertyInstruction) {
	}

	void TessaInliner::visit(SetPropertyInstruction* setPropertyInstruction) {
	}

	void TessaInliner::visit(InitPropertyInstruction* initPropertyInstruction) {
	}

	void TessaInliner::visit(GetSlotInstruction* getSlotInstruction) {
	}

	void TessaInliner::visit(SetSlotInstruction* setSlotInstruction) {
	}

	void TessaInliner::visit(NewArrayInstruction* newArrayInstruction) {
	}

	void TessaInliner::visit(NextNameInstruction* nextNameInstruction) {
	}

	void TessaInliner::visit(PushScopeInstruction* pushScopeInstruction) {
	}

	void TessaInliner::visit(PopScopeInstruction* popScopeInstruction) {
	}

	void TessaInliner::visit(WithInstruction* withInstruction) {
	}

	void TessaInliner::visit(TypeOfInstruction* typeOfInstruction) {
	}

	void TessaInliner::visit(GetScopeObjectInstruction* getScopeObjectInstruction) {
	}

	void TessaInliner::visit(GetGlobalScopeInstruction* getGlobalScopeInstruction) {
	}

	void TessaInliner::visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction) {
	}

	void TessaInliner::visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction) {
	}

	void TessaInliner::visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesIndexInstruction) {
	}

	void TessaInliner::visit(SwitchInstruction* switchInstruction) {
	}

	void TessaInliner::visit(CaseInstruction* caseInstruction) {
	}

	void TessaInliner::visit(NewActivationInstruction* newActiviationInstruction) {
	}

	void TessaInliner::visit(CallVirtualInstruction* callVirtualInstruction) {
		if (callVirtualInstruction->isInlined()) {
			return;
		}

		if (shouldInlineMethod(callVirtualInstruction)) {
			MethodInfo* methodToInlineInfo = callVirtualInstruction->getMethodInfo();

#ifdef DEBUG
			char methodName[512];
			Stringp methodToInlineName = methodToInlineInfo->getMethodName();
			StUTF8String cString(methodToInlineName);
			string inlineeMethodName = this->_inlineeMethod->getMethodName();
			VMPI_snprintf(methodName, sizeof(methodName), "Caller %s Inlining Method id %d ::%s", inlineeMethodName.c_str(), methodToInlineInfo->method_id(), cString.c_str());
			verbosePrint("\n\n=== Inlining method ===\n\n");
			//printf("%s\n", methodName);
#endif

			TessaAssert(methodToInlineInfo != NULL);
			ASFunction* tessaMethod = methodToInlineInfo->getTessaFunction();

			if ((tessaMethod == NULL)) { 
				AvmAssert(methodToInlineInfo != _inlineeMethod->getMethodInfo());
				methodToInlineInfo->verify(this->_verifier->getToplevel(), this->_verifier->getAbcEnv());
				tessaMethod = methodToInlineInfo->getTessaFunction();
			}

			inlineMethod(tessaMethod, callVirtualInstruction);
			consistencyCheck();
			this->_inlineeMethod->addFunctionInlined();
		} 
	}

	void TessaInliner::visit(CallStaticInstruction* callStaticInstruction) {
		TessaAssert(false);
	}

	void TessaInliner::visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction) {

	}

	void TessaInliner::verbosePrint(std::string stringToPrint) {
		if (_core->config.tessaVerbose) {
			printf("%s", stringToPrint.data());
		}
	}

	/***
	 * Add inlining heuristic here.
	 */
	bool TessaInliner::shouldInlineMethod(CallInstruction* callInstruction) {
		if (_core->config.tessaInline && callInstruction->isResolved()) {
			MethodInfo* methodInfo = callInstruction->getMethodInfo();

#ifdef DEBUG
			char methodName[512];
			Stringp methodToInlineName = methodInfo->getMethodName();
			StUTF8String cString(methodToInlineName);
			int methodId = methodInfo->method_id();
			VMPI_snprintf(methodName, sizeof(methodName), "Method id %d ::%s", methodInfo->method_id(), cString.c_str());
			//printf("Should inline method %s\n", methodName);
#endif

			// No rest parameters
			if (methodInfo->needRest()) {
				return false;
			}

			bool isActionScriptMethod = !methodInfo->isNative() && !methodInfo->isStaticInit() && methodInfo->isResolved() && methodInfo->hasMethodBody();
			bool isRecursiveMethod = methodInfo == _inlineeMethod->getMethodInfo();

			if (isActionScriptMethod && !isRecursiveMethod) {
				// Copied from the verifier
		        const byte* position = methodInfo->abc_body_pos();
		        AvmCore::skipU32(position, 4);
		        int codeLength = AvmCore::readU32(position);

				/***
				 * Code length of 30 which is roughly 20 abc opcodes
				 */
				bool shortMethod = codeLength <= 40;

				// short methods, 4 methods per inlinee
				if (shortMethod && (_inlineCount < 4)) { 
					//callInstruction->print();
					BasicBlock* callBlock = callInstruction->getInBasicBlock();
					//printf("Exists in basic block: %d\n", callBlock->getBasicBlockId());
					_inlineCount++;
					return true;
				}
			}
		} 

		return false;
	}

	void TessaInliner::visit(CallInstruction* callInstruction) {
		
	}

	void TessaInliner::visit(InlineBeginInstruction* inlineBeginInstruction) {
	}

	TessaInstruction* TessaInliner::getClonedInstruction(TessaInstruction* originalInstruction) {
		TessaInstruction* clonedInstruction = (TessaInstruction*) _originalToCloneMap.get(originalInstruction);
		AvmAssertMsg(clonedInstruction != NULL, "Original to clone map does not contain instruction. Forgot to add to the map?");
		TessaAssert(clonedInstruction->getType() == originalInstruction->getType());
		return clonedInstruction;
	}

	void TessaInliner::createClonedBasicBlocks(List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocksToInline) {
		for (uint32_t i = 0; i < basicBlocksToInline->size(); i++) {
			BasicBlock* currentBlock = basicBlocksToInline->get(i);
			BasicBlock* clonedBlock = _inlineeMethod->createNewBasicBlock();
			_originalToCloneMap.add(currentBlock, clonedBlock);
		}
	}

	void TessaInliner::updateClonedPhiInstructions(List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocksToInline) {
		for (uint32_t i = 0; i < basicBlocksToInline->size(); i++) {
			BasicBlock* inlinedPhiBlock = basicBlocksToInline->get(i);
			List<PhiInstruction*, avmplus::LIST_GCObjects>* phiInstructions = inlinedPhiBlock->getPhiInstructions();

			for (uint32_t j = 0; j < phiInstructions->size(); j++) {
				PhiInstruction* originalPhi = phiInstructions->get(j);
				PhiInstruction* clonedPhi = (PhiInstruction*) _originalToCloneMap.get(originalPhi);
				TessaAssert(clonedPhi != NULL);

				for (int k = 0; k < originalPhi->numberOfOperands(); k++) {
					BasicBlock* incomingEdge = originalPhi->getIncomingEdge(k);
					TessaInstruction* originalValue = originalPhi->getOperand(incomingEdge);

					TessaAssertMessage(originalValue->getInBasicBlock() == incomingEdge, "Lost an instruction in a basic block somewhere");
					TessaInstruction* clonedValue = (TessaInstruction*) _originalToCloneMap.get(originalValue);
					BasicBlock* clonedBlock = clonedValue->getInBasicBlock();

					TessaAssert(clonedValue != NULL);
					TessaAssert(clonedBlock != NULL);

					clonedPhi->addOperand(clonedBlock, clonedValue);
				}	// End add phi operands
			} // End phi per basic block
		} // End basic block loop
	}

	TessaInstruction* TessaInliner::cloneSingleInstruction(TessaInstruction* currentInstruction, BasicBlock* clonedBlock) {
		TessaInstruction* clonedInstruction = NULL;
		if (_core->config.tessaVerbose) {
			currentInstruction->print();
		}

		clonedInstruction = currentInstruction->clone(_gc, &_originalToCloneMap, clonedBlock);
		TessaAssert(clonedInstruction->getType() == currentInstruction->getType());
		return clonedInstruction;
	}

	/***
	 * Returns the cloned value to return from the inlined method
	 */
	TessaInstruction* TessaInliner::createClonedInstructions() {
		List<BasicBlock*, LIST_GCObjects>* basicBlocksToInline = _methodToInline->getBasicBlocksInReversePostOrder();
		TessaInstruction* clonedReturnValue = NULL;
		ParameterInstruction* paramToReturnValue;
		verbosePrint("\n\n=== Clonining instructions for inlining ===\n\n");

		createClonedBasicBlocks(basicBlocksToInline);

		// Then clone the instructions
		for (uint32_t i = 0; i < basicBlocksToInline->size(); i++) {
			BasicBlock* currentBlock = basicBlocksToInline->get(i);
			BasicBlock* clonedBlock = (BasicBlock*) _originalToCloneMap.get(currentBlock);

			List<TessaInstruction*, LIST_GCObjects>* instructionsToInline = currentBlock->getInstructions();
			uint32_t numberOfInstructionsToInline = instructionsToInline->size();

			for (uint32_t j = 0; j < numberOfInstructionsToInline; j++) {
				TessaInstruction* currentInstruction = instructionsToInline->get(j);

				if (!currentInstruction->isReturn()) {
					TessaInstruction* clonedInstruction = cloneSingleInstruction(currentInstruction, clonedBlock);
					_originalToCloneMap.add(currentInstruction, clonedInstruction);
				} else {
					ReturnInstruction* returnInstruction = static_cast<ReturnInstruction*>(currentInstruction);
					clonedReturnValue = getClonedInstruction(returnInstruction->getValueToReturn());

					paramToReturnValue = new (_gc) ParameterInstruction(NULL, clonedBlock);
					paramToReturnValue->setForwardedInstruction(clonedReturnValue);
					paramToReturnValue->setType(returnInstruction->getType());
				}
			}
		}

		verbosePrint("\n\n==== End Cloning ====\n\n");
		TessaAssert(paramToReturnValue != NULL);
		TessaAssert(clonedReturnValue != NULL);
		return paramToReturnValue;
	}

	/***
	 * Go through the basic blocks to inline, return the block that has the
	 * return instruciton. We need this since the value to return from a method
	 * may originate in a different block than the actual block where the return instruction
	 * is located.
	 */
	BasicBlock* TessaInliner::getClonedReturnBlock() {
		List<BasicBlock*, LIST_GCObjects>* basicBlocksToInline = _methodToInline->getBasicBlocksInReversePostOrder();
		for (uint32_t i = 0; i < basicBlocksToInline->size(); i++) {
			BasicBlock* currentBlock = basicBlocksToInline->get(i);
			List<TessaInstruction*, LIST_GCObjects>* instructionsToInline = currentBlock->getInstructions();
			uint32_t numberOfInstructionsToInline = instructionsToInline->size();

			for (uint32_t j = 0; j < numberOfInstructionsToInline; j++) {
				TessaInstruction* currentInstruction = instructionsToInline->get(j);

				if (currentInstruction->isReturn()) {
					BasicBlock* clonedBlock = (BasicBlock*) _originalToCloneMap.get(currentBlock);
					return clonedBlock;
				} 
			}
		}

		AvmAssertMsg(false, "Method to inline does not have a return instruction");
		return NULL;
	}

	void TessaInliner::mapMethodParametersToValues(ArrayOfInstructions* instanceArguments) {
		BasicBlock* methodToInlineFirstBasicBlock = _methodToInline->getBasicBlocks()->get(0);
		List<ParameterInstruction*, LIST_GCObjects>* parameterInstructions = methodToInlineFirstBasicBlock->getParameterInstructions();
		TessaAssert(instanceArguments->size() <= parameterInstructions->size());

		for (uint32_t i = 0; i < instanceArguments->size(); i++) {
			ParameterInstruction* parameter = parameterInstructions->get(i);
			ParameterInstruction* clonedParameter = (ParameterInstruction*)(_originalToCloneMap.get(parameter));
			TessaAssert(parameter != NULL);
			TessaAssert(clonedParameter != NULL);

			TessaInstruction* parameterValue = instanceArguments->getInstruction(i);
			clonedParameter->setForwardedInstruction(parameterValue);
		}
	}

	/***
	 * Replaces the call instruction with an inline method instruction
	 * splits the basic block the call instruction is in into two basic blocks (A, B). 
	 * A - the basic block where the call instruction existed
	 * B - The first instruction contains the instruction after the call
	 * Merges the first basic block of the inlined method into A
	 * Merges the return of the inlined method into B
	 */
	void TessaInliner::mergeClonedFirstBasicBlockIntoCall(BasicBlock* callInstructionBlock) {
		BasicBlock* firstBlockOfInlinedMethod = (BasicBlock*) _originalToCloneMap.get(_methodToInline->getBasicBlocks()->get(0));
		List<TessaInstruction*, avmplus::LIST_GCObjects>* firstInstructions = firstBlockOfInlinedMethod->getInstructions(); 
		int numberOfInstructions = firstInstructions->size();

		for (int i = 0; i < numberOfInstructions; i++) {
			TessaInstruction* inlinedInstruction = firstInstructions->get(i);
			callInstructionBlock->addInstruction(inlinedInstruction); 
		}

		firstBlockOfInlinedMethod->cleanInstructions();
	}

	void TessaInliner::insertJumpFromInlineReturnBlockToMergeBlock(BasicBlock* returnBlockOfInlinedMethod, BasicBlock* afterCallBasicBlock) {
		TessaAssert(!returnBlockOfInlinedMethod->getLastInstruction()->isBlockTerminator());
		UnconditionalBranchInstruction* jumpToAfterCallBlock = new (_gc) UnconditionalBranchInstruction(afterCallBasicBlock, returnBlockOfInlinedMethod);
	}

	TessaInstruction* TessaInliner::insertInlineGuard(BasicBlock* callBlock, BasicBlock* mergeBlock, BasicBlock* returnBlockOfInlinedMethod, 
			BasicBlock* firstBlockOfInlinedMethod, CallVirtualInstruction* originalCall, TessaInstruction* returnValueOfInlinedMethod) {
		TessaAssert(callBlock->getInstructions()->last()->isLoadVirtualMethod());
		LoadVirtualMethodInstruction* vtableLookup = static_cast<LoadVirtualMethodInstruction*>(callBlock->getInstructions()->last());
		BasicBlock* guardFailBlock = _inlineeMethod->createNewBasicBlock();

		// if (loadedMethod == inlinedMEthodInfo), ptr compare, jump to inlined method
		ConstantPtr* inlinedMethodInfo = new (_gc) ConstantPtr((intptr_t) originalCall->getMethodInfo());
		ConditionInstruction* guardInstruction = new (_gc) ConditionInstruction(EQUAL, vtableLookup->getLoadedMethodInfo(), inlinedMethodInfo, callBlock);
		ConditionalBranchInstruction* branchInstruction = new (_gc) ConditionalBranchInstruction(guardInstruction, firstBlockOfInlinedMethod, guardFailBlock, callBlock);

		// else do the actual call and jump to the merge block
		CallInstruction* guardFailCallInstruction = new (_gc) CallVirtualInstruction(originalCall->getReceiverObject(), vtableLookup, originalCall->getResultTraits(),
			originalCall->getArguments(), originalCall->getMethodId(), originalCall->getMethodInfo(), guardFailBlock);
		guardFailCallInstruction->setType(originalCall->getType());
		new (_gc) UnconditionalBranchInstruction(mergeBlock, guardFailBlock);

		// Mark that the first block of the inlined method is actually an inlined method
		ParameterInstruction* firstParameterInInlinedMethod = firstBlockOfInlinedMethod->getParameterInstructions()->get(0);
		InlineBeginInstruction* inlineBegin = new (_gc) InlineBeginInstruction(originalCall->getLoadedMethodToCall(), originalCall->getResultTraits(), originalCall->getMethodId(), 
			_inlineeMethod->getScopeStackSize(), _methodToInline->getScopeStackSize(), _methodToInline->getMethodInfo(), firstBlockOfInlinedMethod);

		firstBlockOfInlinedMethod->removeInstruction(inlineBegin);
		firstBlockOfInlinedMethod->addInstructionBefore(inlineBegin, firstParameterInInlinedMethod);

		// Set the return value to a be phi of the guard fail block and the inlined return value
		PhiInstruction* returnPhi = new (_gc) PhiInstruction(_gc, mergeBlock);
		returnPhi->addOperand(returnBlockOfInlinedMethod, returnValueOfInlinedMethod);
		returnPhi->addOperand(guardFailBlock, guardFailCallInstruction);
		returnPhi->setType(originalCall->getType());
		return returnPhi;
	}

	/***
	 * Inlining a method takes the following steps:
	 * 1) Clone all the instructions of the inlinee method.
	 * 2) Map the actual arguments  of the call instruction to the parameter instruction of the inlinee method
	 * 3) Split the basic block where the call instruction exists into 2. One block contains all the instructions before the call, the other contains all the instructions after the call
	 * 4) Map the return value of the inlinee method to the call instruction
	 * 5) Update all the cloned phi instructions to point to the new operands
	 * 6) Create the check that the method we are inlining is the actual method in the receiver object's vtable. If not, perform the real call.
	 */
	TessaInstruction* TessaInliner::inlineMethod(ASFunction* methodToInline, CallVirtualInstruction* callInstruction) {
		this->_methodToInline = methodToInline;
		TessaAssert(callInstruction->getMethodInfo()->getTessaFunction() == methodToInline)
		BasicBlock* originalCallBlock = callInstruction->getInBasicBlock();
		List<BasicBlock*, LIST_GCObjects>* originalSuccessors = originalCallBlock->getSuccessors();
		List<BasicBlock*, LIST_GCObjects>* originalPredecessors = originalCallBlock->getPredecessors();

		ArrayOfInstructions* instanceArguments = callInstruction->getArguments();

		TessaInstruction* returnValueOfInlinedMethod = createClonedInstructions();
		BasicBlock* mergeBlock = splitCallInstructionBlock(callInstruction);

		mapMethodParametersToValues(instanceArguments);
		BasicBlock* returnBlockOfInlinedMethod = getClonedReturnBlock();
		BasicBlock* firstBlockOfInlinedMethod = (BasicBlock*) _originalToCloneMap.get(_methodToInline->getBasicBlocks()->get(0));
		TessaInstruction* returnValuePhi = insertInlineGuard(originalCallBlock, mergeBlock, returnBlockOfInlinedMethod, firstBlockOfInlinedMethod, callInstruction, returnValueOfInlinedMethod);

		insertJumpFromInlineReturnBlockToMergeBlock(returnBlockOfInlinedMethod, mergeBlock);
		updateClonedPhiInstructions(methodToInline->getBasicBlocksInReversePostOrder());
		updateCallerPhiInstructions(originalCallBlock, originalSuccessors, originalPredecessors, mergeBlock);

		_inlineeMethod->addNumberOfScopeElements(methodToInline->getScopeStackSize());
		callInstruction->setForwardedInstruction(returnValuePhi);

		verbosePrint("\n=== Inlined Method ===\n\n");
		if (_core->config.tessaVerbose) {
			printf("Inlined method Method ::%s\n", _inlineeMethod->getMethodName().data());
			this->_inlineeMethod->printResults();
		}
		verbosePrint("\n=== End Inlined Method ===\n\n");
		return returnValueOfInlinedMethod;
	}

	void TessaInliner::updatePhiOperands(BasicBlock* phisInBlock, TessaVM::BasicBlock* originalIncomingEdge, TessaVM::BasicBlock* updatedIncomingEdge) {
		List<PhiInstruction*, LIST_GCObjects>* phiInstructions = phisInBlock->getPhiInstructions();

		for (size_t phiIndex = 0; phiIndex < phiInstructions->size(); phiIndex++){
			PhiInstruction* currentPhi = phiInstructions->get(phiIndex);
			if (currentPhi->containsEdge(originalIncomingEdge)) {
				TessaInstruction* phiOperand = currentPhi->getOperand(originalIncomingEdge);
				currentPhi->removeOperand(originalIncomingEdge);

				TessaInstruction* newPhiOperand;
				if (phiOperand->getInBasicBlock() == updatedIncomingEdge) {
					newPhiOperand = phiOperand;
				} else {
					TessaInstruction* incomingEdgeBlockTerminator = updatedIncomingEdge->getLastInstruction();
					updatedIncomingEdge->removeInstruction(incomingEdgeBlockTerminator);

					ParameterInstruction* newParameter = new (_gc) ParameterInstruction(NULL, updatedIncomingEdge);
					newParameter->setType(phiOperand->getType());
					newParameter->setForwardedInstruction(phiOperand);
					updatedIncomingEdge->addInstruction(incomingEdgeBlockTerminator);

					newPhiOperand = newParameter;
				}

				currentPhi->addOperand(updatedIncomingEdge, newPhiOperand);
			} else {
				AvmAssert(!phisInBlock->isPredecessor(originalIncomingEdge) || !phisInBlock->isSuccessor(originalIncomingEdge));
			}
		}
	}

	/***
	 * Once we inline a method, we have to update the phi instructions in the ORIGINAL predecessors to the call block.
	 * As well as the phi instructions in the ORIGINAL successors of the ORIGINAL call instruction
	 */
	void TessaInliner::updateCallerPhiInstructions(TessaVM::BasicBlock* originalCallBlock, List<BasicBlock*, LIST_GCObjects>* originalPredecessors,
		List<BasicBlock*, LIST_GCObjects>* originalSuccessors, TessaVM::BasicBlock* mergeBlock) {

		for (size_t predecessorIndex = 0; predecessorIndex < originalPredecessors->size(); predecessorIndex++) {
			BasicBlock* currentPredecessor = originalPredecessors->get(predecessorIndex);
			updatePhiOperands(currentPredecessor, originalCallBlock, mergeBlock);
		}

		for (size_t successorIndex = 0; successorIndex < originalSuccessors->size(); successorIndex++) {
			BasicBlock* currentSuccessor = originalSuccessors->get(successorIndex);
			updatePhiOperands(currentSuccessor, originalCallBlock, mergeBlock);
		}
	}

	BasicBlock* TessaInliner::splitCallInstructionBlock(CallInstruction* callInstruction) {
		BasicBlock* callBlock = callInstruction->getInBasicBlock();
		BasicBlock* afterCallInstructionBlock = _inlineeMethod->createNewBasicBlock();

		List<TessaInstruction*, avmplus::LIST_GCObjects>* callBlockInstructions = callBlock->getInstructions();
		uint32_t indexOfCall = callBlockInstructions->indexOf(callInstruction); 
		for (uint32_t i = indexOfCall; i < callBlockInstructions->size(); i++) {
			TessaInstruction* instruction = callBlockInstructions->get(i);
			afterCallInstructionBlock->addInstruction(instruction);
		}

		for (uint32_t i = indexOfCall; i < callBlockInstructions->size(); ) {
			callBlockInstructions->removeAt(i);
		}

		TessaAssert(callBlockInstructions->last()->isLoadVirtualMethod());
		return afterCallInstructionBlock;
	}

	/***
	 * We need the verifier because we may want to inline a method that hasn't yet been compiled.
	 * If so, we have to compile the method, then inline it
	 */
	void TessaInliner::findInlineOpportunities(ASFunction* methodToAnalyze, Verifier* verifier) {
		this->_verifier = verifier;
		this->_inlineeMethod = methodToAnalyze;

		List<BasicBlock*, LIST_GCObjects>* basicBlocks = methodToAnalyze->getBasicBlocks();

		/***
		 * We need a clone because the inlining process is going to add new basic blocks and instructions. 
		 * We don't want to iterate over any new inlined methods.
		 */
		List<BasicBlock*, LIST_GCObjects>* basicBlocksClone = new (_gc) List<BasicBlock*, LIST_GCObjects>(_gc);
		for (uint32_t i = 0; i < basicBlocks->size(); i++) {
			basicBlocksClone->add(basicBlocks->get(i));
		}

		for (uint32_t i = 0; i < basicBlocksClone->size(); i++) {
			BasicBlock* currentBlock = basicBlocks->get(i);
			List<TessaInstruction*, LIST_GCObjects>* instructions = currentBlock->getInstructions();

			for (uint32_t j = 0; j < instructions->size(); j++) {
				TessaInstruction* currentInstruction = instructions->get(j)->resolve();
				currentInstruction->visit(this);
			}
		}
	}

	void TessaInliner::consistencyCheck() {
#ifdef DEBUG
		ConsistencyChecker consistencyChecker(_core);
		consistencyChecker.consistencyCheck(_inlineeMethod);
#endif
	}


}