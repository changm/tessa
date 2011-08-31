

namespace TessaVM {
	class ASFunction;
	class BasicBlock;
}

namespace TessaInstructions {
	using namespace TessaVM;

	class ConsistencyChecker : public MMgc::GCObject {
	private:
		avmplus::AvmCore* core;
		ASFunction* _functionToCheck;

	private:
		void	checkAllBlocksHaveTerminators();
		void	checkAllBlocksHaveOneTerminator();
		void	checkAllBlocksWithMultiplePredecessorsHavePhi();
		void	checkAllPhisHaveSameType();
		void	checkPhiOperandsJumpToCurrentBlock();
		bool	allPhiOperandsArePredecessors(PhiInstruction* phiInstruction);
		bool phiOperandsExistsInPredecessorBlock(PhiInstruction* phiInstruction);

	public:
		ConsistencyChecker(avmplus::AvmCore* core);
		void	consistencyCheck(ASFunction* function);
		
	};
}