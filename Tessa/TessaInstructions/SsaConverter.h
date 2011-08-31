
namespace TessaVM {
	class ASFunction;
	class BasicBlock;
	class StateVector;
}

namespace TessaInstructions {
	using namespace TessaVM;

	class SsaConverter : public MMgc::GCObject {
	private:
		avmplus::AvmCore* core;

	private:
		bool	allOperandsAreEqual(List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> * predecessors, StateVector* mergeBlockStateVector, int localVarId); 
		void	insertPhiInstructions(TessaVM::BasicBlock* intoBasicBlock);
		void	mergePredecessorValuesIntoMergeBlockPhiInstruction(List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> * predecessors, PhiInstruction* phiInstruction, int localVarId); 
		void	mergePredecessorValuesIntoMergeBlock(TessaVM::BasicBlock* mergeBlock, TessaVM::BasicBlock* predecessor); 
		void	forwardParametersToPreviousBlock(TessaVM::BasicBlock* mergeBlock, TessaVM::BasicBlock* predecessor); 

	public:
		SsaConverter(avmplus::AvmCore* core);
		void convertIntoSsa(ASFunction* function);
	};
}