#include "TessaInstructionHeader.h"

/***
 * Have to manually include TessaVM files because TessaVM includes TessaInstructionHeader
 */
#include "ASFunction.h"
#include "StateVector.h"
#include "BasicBlock.h"

using namespace TessaVM;

namespace TessaInstructions {

	TessaCreationApi::TessaCreationApi(AvmCore* core, MMgc::GC* gc) {
		this->core = core;
		this->gc = gc;
	}

	TessaVM::BasicBlock* TessaCreationApi::createNewFunction(MethodInfo* methodInfo, string methodName, int numberOfLocals, int scopeStackSize) {
		currentFunction = new (gc) TessaVM::ASFunction(methodName, methodInfo, numberOfLocals, scopeStackSize, core, gc);
		TessaVM::BasicBlock* firstBasicBlock = createNewBasicBlock();
		currentFunction->setCurrentBasicBlock(firstBasicBlock);
		return firstBasicBlock;
	}

	/***
	 * You can get the basic block object by asking the Label to give you the basic block
	 * When you create a basic block, it also creates the parameter instructions for the current number of local variables.
	 * If you are tracking more than just local variables, you have to manually create a parameter instruction later.
	 */
	TessaVM::BasicBlock* TessaCreationApi::createNewBasicBlock() {
		TessaVM::BasicBlock* newBasicBlock = currentFunction->createNewBasicBlock();
		this->switchToBasicBlock(newBasicBlock);
		int localCount = currentFunction->getLocalCount();
		StateVector* beginStateVector = new (gc) StateVector(localCount, gc);

		for (int i = 0; i < localCount; i++) {
			beginStateVector->setLocal(i, new (gc) ParameterInstruction(NULL, newBasicBlock));
		}

		StateVector* endStateVector = beginStateVector->clone();

		newBasicBlock->setBeginState(beginStateVector);
		newBasicBlock->setEndState(endStateVector);
		return newBasicBlock;
	}

	/***
	 * Switches the current basic block to the one attached to the label
	 * All instructions created after this statement will belong to this basic block
	 */
	void TessaCreationApi::switchToBasicBlock(BasicBlock* basicBlock) {
		currentFunction->switchToBasicBlock(basicBlock);
		this->_currentBasicBlock = basicBlock;
	}

	NextNameInstruction* TessaCreationApi::createNextNameInstruction(TessaInstruction* receiverObject, TessaInstruction* registerInstruction) {
		TessaAssert(false);
		NextNameInstruction* nextNameInstruction = new (gc) NextNameInstruction(receiverObject, registerInstruction);
		return nextNameInstruction;
	}

	HasMorePropertiesObjectInstruction* TessaCreationApi::createHasMorePropertiesObjectInstruction(TessaInstruction* objectInstruction) {
		TessaAssert(false);
		HasMorePropertiesObjectInstruction* hasMorePropertiesObjectInstruction = new (gc) HasMorePropertiesObjectInstruction(objectInstruction);
		return hasMorePropertiesObjectInstruction;
	}

	HasMorePropertiesRegisterInstruction* TessaCreationApi::createHasMorePropertiesRegisterInstruction(TessaInstruction* registerInstruction) {
		TessaAssert(false);
		HasMorePropertiesRegisterInstruction* hasMorePropertiesIndexInstruction = new (gc) HasMorePropertiesRegisterInstruction(registerInstruction);
		return hasMorePropertiesIndexInstruction;
	}

	HasMorePropertiesInstruction*	TessaCreationApi::createHasMorePropertiesInstruction(HasMorePropertiesObjectInstruction* objectInstruction, HasMorePropertiesRegisterInstruction* registerInstruction) {
		TessaAssert(false);
		HasMorePropertiesInstruction* hasMorePropertiesInstruction = new (gc) HasMorePropertiesInstruction(objectInstruction, registerInstruction);
		return hasMorePropertiesInstruction;
	}

	ASFunction*	TessaCreationApi::getFunction() {
		return currentFunction;
	}

	void TessaCreationApi::printResults() {
		currentFunction->printResults();
	}

	void TessaCreationApi::setLocalVariable(int localVariableNumber, TessaInstruction* newReference) {
		StateVector* endState = currentFunction->getCurrentBasicBlock()->getEndState();
		endState->setLocal(localVariableNumber, newReference);
	}

	TessaInstruction* TessaCreationApi::getLocalVariable(int localVariableNumber) {
		StateVector* endState = currentFunction->getCurrentBasicBlock()->getEndState();
		return endState->getLocal(localVariableNumber);
	}

	/***
	 * @param index - Index from the starting number of locals
	 * @param reference - The new reference
	 * @param startingNumberOfLocals - the number of variables the state vector is currently tracking. We need this because the
				the total number of references will change if you are adding multiple number of variables.
	*/
	void TessaCreationApi::trackExtraVariableInEndState(int index, TessaInstruction* reference, int startingNumberOfLocals) {
		BasicBlock* currentBlock = currentFunction->getCurrentBasicBlock();
		StateVector* endState = currentBlock->getEndState();
		//AvmAssert(index + startingNumberOfLocals <= endState->getNumberOfLocals());
		endState->setLocal(index + startingNumberOfLocals, reference);
	}

	void TessaCreationApi::trackExtraVariableInEntryState(int index, TessaInstruction* reference, int startingNumberOfLocals) {
		BasicBlock* currentBlock = currentFunction->getCurrentBasicBlock();
		StateVector* beginState = currentBlock->getBeginState();
		//AvmAssert(index + startingNumberOfLocals <= beginState->getNumberOfLocals());
		beginState->setLocal(index + startingNumberOfLocals, reference);
	}

	void TessaCreationApi::stopTrackingExtraVariable(int index) {
		StateVector* endState = currentFunction->getCurrentBasicBlock()->getEndState();
		int numberOfVariables = endState->getNumberOfLocals();
		int slotIndex = index + numberOfVariables;
		TessaAssert(index <= numberOfVariables)
		return endState->deleteLocal(slotIndex);
	}


	/***
	 * Tells the API that we're done creating instructions. This does multiple things
	 * 1) Creates SSA out of the instructions
	 * 2) Consistency check each basic block
	 * 3) Creates dominance and reverse post order blocks
	 */
	void TessaCreationApi::finishMethod() {
		convertIntoSsa();
#ifdef DEBUG
		if (core->config.tessaVerbose) {
			printf("\n=== After SSA ===");
			this->printResults();
		}
		consistencyCheck();
#endif
	}

	void TessaCreationApi::consistencyCheck() {
#ifdef DEBUG
		ConsistencyChecker consistencyChecker(core);
		consistencyChecker.consistencyCheck(currentFunction);
#endif
	}

	void TessaCreationApi::convertIntoSsa() {
		SsaConverter *ssaConverter = new (gc) SsaConverter(core);
		ssaConverter->convertIntoSsa(currentFunction);
	}
}