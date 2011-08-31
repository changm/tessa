
/***
 * Wrapper around ABC data classes
 */
namespace TessaVM {
	using namespace avmplus;
	using namespace std;
	class BasicBlock;

	class ASFunction : public MMgc::GCObject {
	private:
		// Data
		string		_methodName;
		MethodInfo*	_methodInfo;
		avmplus::AvmCore*	_core;
		MMgc::GC*	_gc;

		BasicBlock* _currentBasicBlock;
		int _localCount;
		int _scopeStackSize;
		int _numberOfFunctionsInlined;
		List<BasicBlock* , avmplus::LIST_GCObjects> *_basicBlocks;

		bool alreadyVisitedBlock(BasicBlock* basicBlock, bool* visitedBlockSet); 
		void setVisitedBlock(BasicBlock* basicBlock, bool* visitedBlockSet); 
		void createReversePostOrder(BasicBlock* root, List<BasicBlock*, avmplus::LIST_GCObjects>* reversePostOrderList, bool* visitedSet); 
		bool containsAllBlocks(List<BasicBlock*, avmplus::LIST_GCObjects>* original, List<BasicBlock*, avmplus::LIST_GCObjects>* reversePostOrderList); 


	public:
		ASFunction(string methodName, MethodInfo* methodInfo, int localCount, int scopeStackSize, avmplus::AvmCore* core, MMgc::GC* gc);
		void*	_nativeCodeLocation;

		void	addInstruction(TessaInstruction* instructionToAdd);
		void	printResults();
		BasicBlock* ASFunction::createNewBasicBlock(); 
		void		switchToBasicBlock(BasicBlock*);

		List<BasicBlock* , avmplus::LIST_GCObjects> * getBasicBlocks();
		List<BasicBlock* , avmplus::LIST_GCObjects> * getBasicBlocksInReversePostOrder();
		List<BasicBlock* , avmplus::LIST_GCObjects> * getBasicBlocksInDominatorOrder();

		BasicBlock* getEntryBlock(); 
		BasicBlock*	getCurrentBasicBlock();
		void		setCurrentBasicBlock(BasicBlock* basicBlock);
		int			getLocalCount();
		int			getScopeStackSize();
		void		addNumberOfScopeElements(int newScopeObjectSize);
		void*		getNativeCodeLocation();
		void		addFunctionInlined(); 
		int			getNumberOfFunctionsInlined(); 
		string		getMethodName();
		MethodInfo*	getMethodInfo();
	};
}