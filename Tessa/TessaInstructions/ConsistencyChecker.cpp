
#include "TessaInstructionHeader.h"

/***
 * Have to manually include TessaVM files because TessaVM includes TessaInstructionHeader
 */
#include "ASFunction.h"
#include "StateVector.h"
#include "BasicBlock.h"


namespace TessaInstructions {
	ConsistencyChecker::ConsistencyChecker(AvmCore* core) {
		this->core = core;
	}	

	void ConsistencyChecker::consistencyCheck(ASFunction* function) {
		_functionToCheck = function;
		checkAllBlocksHaveTerminators();
		checkAllBlocksHaveOneTerminator();
		checkPhiOperandsJumpToCurrentBlock();
		//checkAllBlocksWithMultiplePredecessorsHavePhi();
		//checkAllPhisHaveSameType(function);
	}

	void ConsistencyChecker::checkAllPhisHaveSameType() {
		List<BasicBlock*, LIST_GCObjects>* basicBlocks = _functionToCheck->getBasicBlocksInReversePostOrder();
		for (uint32_t i = 0; i < basicBlocks->size(); i++) {
			BasicBlock* currentBasicBlock = basicBlocks->get(i);

			List<PhiInstruction*, LIST_GCObjects>* phiInstructions = currentBasicBlock->getPhiInstructions();
			for	(uint32_t i = 0; i < phiInstructions->size(); i++) {
				PhiInstruction* phiInstruction = phiInstructions->get(i);
				Type* phiType = phiInstruction->getType();

				for (int j = 0; j < phiInstruction->numberOfOperands(); j++) {
					BasicBlock* phiIncomingBlock = phiInstruction->getIncomingEdge(j);
					TessaInstruction* phiIncomingOperand = phiInstruction->getOperand(phiIncomingBlock);
					TessaAssert(phiType == phiIncomingOperand->getType());
				}		
			}	
		}
		
	}

	void ConsistencyChecker::checkAllBlocksWithMultiplePredecessorsHavePhi() {
		List<BasicBlock*, LIST_GCObjects>* basicBlocks = _functionToCheck->getBasicBlocksInReversePostOrder();
		TessaAssert(basicBlocks != NULL);
		char messageBuffer[64];
		for (uint32_t i = 0; i < basicBlocks->size(); i++) {
			BasicBlock* currentBasicBlock = basicBlocks->get(i);
			
			if (currentBasicBlock->getPredecessors()->size() > 1) {
				List<TessaInstruction*, LIST_GCObjects>* instructions = currentBasicBlock->getInstructions();

				for (uint32_t j = 0; j < instructions->size(); j++) {
					TessaInstruction* instruction = instructions->get(j);
					if (instruction->isParameter()) {
						ParameterInstruction* paramInstruction = (ParameterInstruction*) instruction;
						TessaInstruction* forwardInstruction = paramInstruction->resolve();

						VMPI_snprintf(messageBuffer, sizeof(messageBuffer), "BB %d Param %d does not map to Phi \n", currentBasicBlock->getBasicBlockId(), paramInstruction->getValueId());
						TessaAssertMessage(forwardInstruction->isPhi(), messageBuffer);

						if (!forwardInstruction->isPhi()) {
							printf("%s\n", messageBuffer);
						}
					}
				}
			}
		}
	}

	bool ConsistencyChecker::allPhiOperandsArePredecessors(PhiInstruction* phiInstruction) {
		BasicBlock* parentBlock = phiInstruction->getInBasicBlock();
		int numberOfOperands = phiInstruction->numberOfOperands();
		char errorMessage[64];

		for (int i = 0; i < numberOfOperands; i++) {
			BasicBlock* operandBlock = phiInstruction->getIncomingEdge(i);
			List<BasicBlock*, LIST_GCObjects>* successors = operandBlock->getSuccessors();
			if (!successors->contains(parentBlock)) { 
				VMPI_snprintf(errorMessage, sizeof(errorMessage), "Phi %d has incoming block %d, but block is not a predecessor\n", 
					phiInstruction->getValueId(), operandBlock->getBasicBlockId());
				printf(errorMessage);
				return false;
			}
		}

		return true;
	}

	bool ConsistencyChecker::phiOperandsExistsInPredecessorBlock(PhiInstruction* phiInstruction) {
		BasicBlock* parentBlock = phiInstruction->getInBasicBlock();
		int numberOfOperands = phiInstruction->numberOfOperands();
		char errorMessage[256];

		for (int i = 0; i < numberOfOperands; i++) {
			BasicBlock* operandBlock = phiInstruction->getIncomingEdge(i);
			TessaInstruction* phiOperand = phiInstruction->getOperand(operandBlock);
			BasicBlock* phiOperandBlock = phiOperand->getInBasicBlock();

			VMPI_snprintf(errorMessage, sizeof(errorMessage), "Phi %d has operand %d with incoming block %d, but operand does not exist in block\n\n",
					phiInstruction->getValueId(), phiOperand->getValueId(), operandBlock->getBasicBlockId());
			AvmAssertMsg(operandBlock == phiOperandBlock, errorMessage);
		}

		return true;
	}

	void ConsistencyChecker::checkPhiOperandsJumpToCurrentBlock() {
		List<BasicBlock*, LIST_GCObjects>* basicBlocks = _functionToCheck->getBasicBlocks();
		TessaAssert(basicBlocks != NULL);

		for (size_t i = 0; i < basicBlocks->size(); i++) {
			BasicBlock* currentBasicBlock = basicBlocks->get(i);

			if (currentBasicBlock->getPredecessors()->size() > 1) {
				List<PhiInstruction*, LIST_GCObjects>* phiInstructions = currentBasicBlock->getPhiInstructions();

				for (size_t j = 0; j < phiInstructions->size(); j++) {
					PhiInstruction* phiInstruction = phiInstructions->get(j);
					AvmAssert(allPhiOperandsArePredecessors(phiInstruction));
					AvmAssert(phiOperandsExistsInPredecessorBlock(phiInstruction));
				}
			}
		}
	}

	void ConsistencyChecker::checkAllBlocksHaveOneTerminator() {
		List<BasicBlock*, LIST_GCObjects>* basicBlocks = _functionToCheck->getBasicBlocksInReversePostOrder();
		TessaAssert(basicBlocks != NULL);
		char messageBuffer[64];
		for (uint32_t i = 0; i < basicBlocks->size(); i++) {
			BasicBlock* currentBasicBlock = basicBlocks->get(i);
			List<TessaInstruction*, LIST_GCObjects>* instructions = currentBasicBlock->getInstructions();
			int numberOfTerminators = 0;

			for (uint32_t j = 0; j < instructions->size(); j++) {
				if (instructions->get(j)->isBlockTerminator()) {
					numberOfTerminators++;
				}
			}

			VMPI_snprintf(messageBuffer, sizeof(messageBuffer), "BB %d has %d terminators instruction\n", currentBasicBlock->getBasicBlockId(), numberOfTerminators);
			TessaAssertMessage(numberOfTerminators == 1, messageBuffer);
			if (numberOfTerminators != 1) {
				printf("%s\n", messageBuffer);
			}
		}
	}

	void ConsistencyChecker::checkAllBlocksHaveTerminators() {
		/*** 
		 * Have to do reverse post order because some blocks may be dead due to weird control flow (breaks/continue statements)
		 */
		List<BasicBlock*, LIST_GCObjects>* basicBlocks = _functionToCheck->getBasicBlocksInReversePostOrder();
		TessaAssert(basicBlocks != NULL);

		char messageBuffer[64];
		for (uint32_t i = 0; i < basicBlocks->size(); i++) {
			BasicBlock* currentBasicBlock = basicBlocks->get(i);
			TessaInstruction* lastInstruction = currentBasicBlock->getLastInstruction();

			VMPI_snprintf(messageBuffer, sizeof(messageBuffer), "BB %d does not have terminator instruction\n", currentBasicBlock->getBasicBlockId());
			TessaAssertMessage(lastInstruction->isBlockTerminator(), messageBuffer);
			if (!lastInstruction->isBlockTerminator()) {
				printf("%s\n", messageBuffer);
			}
		}
	}
}