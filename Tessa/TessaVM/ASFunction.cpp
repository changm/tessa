#include "TessaVM.h"

namespace TessaVM {
	ASFunction::ASFunction(string methodName, MethodInfo* methodInfo, int localCount, int scopeStackSize, avmplus::AvmCore* core, MMgc::GC* gc) {
		this->_core = core;
		this->_gc = core->gc;
		this->_methodName = methodName;
		this->_methodInfo = methodInfo;
		this->_localCount = localCount;
		this->_scopeStackSize = scopeStackSize;
		this->_numberOfFunctionsInlined = 0;

		this->_basicBlocks = new (gc) List<BasicBlock*, avmplus::LIST_GCObjects>(gc);

	}

	BasicBlock* ASFunction::createNewBasicBlock() {
		BasicBlock* newBasicBlock = new (_gc) BasicBlock(_core, _basicBlocks->size());
		_basicBlocks->add(newBasicBlock);
		return newBasicBlock;
	}

	void ASFunction::switchToBasicBlock(BasicBlock* basicBlock) {
		int basicBlockId = basicBlock->getBasicBlockId();
		TessaAssertMessage((_basicBlocks->get(basicBlockId) == basicBlock), "Current function does not contain given basic block");
		this->_currentBasicBlock = basicBlock;
	}
	
	void ASFunction::addInstruction(TessaInstruction* instructionToAdd) {
		TessaAssert(_currentBasicBlock != 0);
		_currentBasicBlock->addInstruction(instructionToAdd);
	}

	void ASFunction::printResults() {
		for (uint32_t i = 0; i < _basicBlocks->size(); i++) {
			BasicBlock *currentBasicBlock = _basicBlocks->get(i);
			currentBasicBlock->printResults();
		}
	}

	BasicBlock* ASFunction::getEntryBlock() {
		BasicBlock* rootBlock = _basicBlocks->get(0);
		TessaAssert(rootBlock->getBasicBlockId() == 0);
		return rootBlock;
	}

	/***
	 * This gets the basic blocks in the order that they are created. 
	 */
	List<BasicBlock* , avmplus::LIST_GCObjects> * ASFunction::getBasicBlocks() {
		return _basicBlocks;
	}

	BasicBlock*	ASFunction::getCurrentBasicBlock() {
		return this->_currentBasicBlock;
	}

	void ASFunction::setCurrentBasicBlock(BasicBlock* basicBlock) {
		TessaAssert(basicBlock != NULL);
		_currentBasicBlock = basicBlock;
	}

	int ASFunction::getLocalCount() {
		return _localCount;
	}

	int	ASFunction::getScopeStackSize() {
		return _scopeStackSize;
	}

	/***
	 * If we inline a method, we want to merge the scope stacks in the same memory location.
	 * So this increases the scope stack size by @param
	 */
	void ASFunction::addNumberOfScopeElements(int newScopeObjectSize) {
		_scopeStackSize += newScopeObjectSize;
	}

	void ASFunction::addFunctionInlined() {
		this->_numberOfFunctionsInlined++;
	}

	int ASFunction::getNumberOfFunctionsInlined() {
		return this->_numberOfFunctionsInlined;
	}

	bool ASFunction::alreadyVisitedBlock(BasicBlock* basicBlock, bool* visitedBlockSet) {
		int blockId = basicBlock->getBasicBlockId();
		return visitedBlockSet[blockId] == true;
	}

	void ASFunction::setVisitedBlock(BasicBlock* basicBlock, bool* visitedBlockSet) {
		visitedBlockSet[basicBlock->getBasicBlockId()] = true;
	}

	void ASFunction::createReversePostOrder(BasicBlock* root, List<BasicBlock*, avmplus::LIST_GCObjects>* reversePostOrderList, bool* visitedSet) {
		setVisitedBlock(root, visitedSet);

		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects>* successors = root->getSuccessors();
		for (uint32_t successorIndex = 0; successorIndex < successors->size(); successorIndex++) {
			TessaVM::BasicBlock* successorBlock = successors->get(successorIndex);
			if (!alreadyVisitedBlock(successorBlock, visitedSet)) {
				createReversePostOrder(successorBlock, reversePostOrderList, visitedSet);
			}
		}

		reversePostOrderList->insert(0, root);
	}

	/***
	 * Checks that every block in "reversePostOrderList" exists in "original". Only a consistency check.
	 */
	bool ASFunction::containsAllBlocks(List<BasicBlock*, avmplus::LIST_GCObjects>* original, List<BasicBlock*, avmplus::LIST_GCObjects>* reversePostOrderList) {
		if (reversePostOrderList->size() > original->size()) {
			// We might have a totally dead block
			return false;
		}

		return true;
	}
 
	List<BasicBlock* , avmplus::LIST_GCObjects> * ASFunction::getBasicBlocksInReversePostOrder() {
		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks = getBasicBlocks();
		TessaVM::BasicBlock* rootBlock = getEntryBlock();
		TessaAssert(rootBlock->getBasicBlockId() == 0);

		int numberOfBlocks = basicBlocks->size();
		bool* visitedBlockSet = new bool[numberOfBlocks + 1];
		VMPI_memset(visitedBlockSet, 0, sizeof(bool) * (numberOfBlocks + 1));

		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects>* reversePostOrderList = new (_gc) List<BasicBlock*, LIST_GCObjects> (_gc, numberOfBlocks);
		createReversePostOrder(rootBlock, reversePostOrderList, visitedBlockSet);
		TessaAssert(containsAllBlocks(basicBlocks, reversePostOrderList));
		return reversePostOrderList;
	}

	List<BasicBlock* , avmplus::LIST_GCObjects> * ASFunction::getBasicBlocksInDominatorOrder() {
		TessaAssertMessage(false, "Dominator order not implemented yet");
		return NULL;
	}

	void* ASFunction::getNativeCodeLocation() {
		TessaAssert(_nativeCodeLocation != NULL);
		return _nativeCodeLocation;
	}

	string ASFunction::getMethodName() {
		return _methodName;
	}

	MethodInfo* ASFunction::getMethodInfo() {
		return _methodInfo;
	}
}

