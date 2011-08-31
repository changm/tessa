#include "TessaInstructionHeader.h"

/***
 * Have to manually include TessaVM files because TessaVM includes TessaInstructionHeader
 */
#include "ASFunction.h"
#include "StateVector.h"
#include "BasicBlock.h"

namespace TessaInstructions {
	SsaConverter::SsaConverter(avmplus::AvmCore* core) {
		this->core = core;
	}

	void SsaConverter::mergePredecessorValuesIntoMergeBlock(BasicBlock* mergeBlock, BasicBlock* predecessor) {
		StateVector* mergeBeginState = mergeBlock->getBeginState();
		StateVector* predecessorEndState = predecessor->getEndState();
		TessaAssertMessage(mergeBeginState->getNumberOfLocals() == predecessorEndState->getNumberOfLocals(), "Unequal number of locals");

		for (int localVar = 0; localVar < mergeBeginState->getNumberOfLocals(); localVar++) {
			TessaInstruction* predecessorValue = predecessorEndState->getLocal(localVar);
			TessaInstruction* phiInMerge = mergeBeginState->getLocal(localVar);
			TessaAssert(phiInMerge->isPhi());

			PhiInstruction* phiInstruction = (PhiInstruction*) phiInMerge;
			phiInstruction->addOperand(predecessor, predecessorValue);
		}
	}

	void SsaConverter::mergePredecessorValuesIntoMergeBlockPhiInstruction(List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> * predecessors, PhiInstruction* phiInstruction, int localVarId) {
		for (uint32_t i = 0; i < predecessors->size(); i++) {
			TessaVM::BasicBlock* predecessor = predecessors->get(i);
			TessaAssert(predecessor != phiInstruction->getInBasicBlock());
			StateVector* predecessorEndState = predecessor->getEndState();
			TessaInstruction* predecessorValue = predecessorEndState->getLocal(localVarId);
			phiInstruction->addOperand(predecessor, predecessorValue);
		}
	}

	bool SsaConverter::allOperandsAreEqual(List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> * predecessors, StateVector* mergeBlockStateVector, int localVarId) {
		TessaAssert(predecessors->size() > 1);
		TessaInstruction* firstValue = NULL;

		for (uint32_t predecessorId = 0; predecessorId < predecessors->size(); predecessorId++) {
			BasicBlock* predecessor = predecessors->get(predecessorId);
			StateVector* predecessorEndState = predecessor->getEndState();
			TessaInstruction* predecessorValue = predecessorEndState->getLocal(localVarId)->resolve();
			if (firstValue == NULL) {
				firstValue = predecessorValue;
			}

			if (predecessorValue != firstValue) {
				return false;
			}
		}

		return true;
	}

	void SsaConverter::insertPhiInstructions(TessaVM::BasicBlock *basicBlock) {
		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> * predecessors = basicBlock->getPredecessors();
		TessaAssert(predecessors->size() > 1);
		MMgc::GC* gc = core->gc;

		StateVector* mergeBlockStateVector = basicBlock->getBeginState();
		for (int localVar = 0; localVar < mergeBlockStateVector->getNumberOfLocals(); localVar++) {
			TessaInstruction* currentValue = mergeBlockStateVector->getLocal(localVar);
			TessaAssert(currentValue->isParameter());

			if (allOperandsAreEqual(predecessors, mergeBlockStateVector, localVar)) {
				TessaInstruction* predecessorValue = predecessors->get(0)->getEndState()->getLocal(localVar);
				currentValue->setForwardedInstruction(predecessorValue);
			} else {
				PhiInstruction* phiInstruction = new (gc) PhiInstruction(gc, basicBlock);
				//basicBlock->addPhiInstruction(phiInstruction);
				mergePredecessorValuesIntoMergeBlockPhiInstruction(predecessors, phiInstruction, localVar);
				currentValue->setForwardedInstruction(phiInstruction);
				mergeBlockStateVector->setLocal(localVar, phiInstruction);
			}
		}
	}

	void SsaConverter::forwardParametersToPreviousBlock(BasicBlock* mergeBlock, BasicBlock* predecessor) {
		StateVector* mergeBlockBegin = mergeBlock->getBeginState();
		StateVector* predecessorEndState = predecessor->getEndState();
		TessaAssert(mergeBlockBegin->getNumberOfLocals() == predecessorEndState->getNumberOfLocals());

		for (int localVar = 0; localVar < mergeBlockBegin->getNumberOfLocals(); localVar++) {
			TessaInstruction* mergeValue = mergeBlockBegin->getLocal(localVar);
			TessaAssert(mergeValue->isParameter());

			TessaInstruction* predecessorValue = predecessorEndState->getLocal(localVar);
			mergeValue->setForwardedInstruction(predecessorValue);
		}
	}

	void SsaConverter::convertIntoSsa(ASFunction *function) {
		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> *basicBlocks = function->getBasicBlocksInReversePostOrder();
		for (uint32_t i = 1; i < basicBlocks->size(); i++) {
			BasicBlock* currentBasicBlock = basicBlocks->get(i);
			List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> * predecessors = currentBasicBlock->getPredecessors();

			if (predecessors->size() > 1) {
				insertPhiInstructions(currentBasicBlock);
			} else {
				forwardParametersToPreviousBlock(currentBasicBlock, predecessors->get(0));
			}
		}
	}


}