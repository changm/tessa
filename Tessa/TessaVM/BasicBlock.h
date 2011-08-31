
namespace TessaVM {
	using namespace avmplus;

	class BasicBlock : public MMgc::GCObject {
	private:
		List<TessaInstruction*, avmplus::LIST_GCObjects>* _instructions;
		List<PhiInstruction*, avmplus::LIST_GCObjects>* _phiInstructions;
		List<ParameterInstruction*, avmplus::LIST_GCObjects>* _parameterInstructions;
		List<TessaInstruction*, avmplus::LIST_GCObjects>* _predecessors;

		bool _loopHeader;
		bool _switchHeader;
		avmplus::AvmCore*	_core;
		MMgc::GC*	_gc;

		StateVector*	_beginStateVector;
		StateVector*	_endStateVector;
		int				_basicBlockId;

	public:
		bool			_terminatesWithReturn;

		BasicBlock();
		BasicBlock(avmplus::AvmCore* core, int basicBlockNumber);
		void	addInstruction(TessaInstruction* tessaInstruction);
		void	addInstructionAfter(TessaInstruction* instruction, TessaInstruction* insertAfter); 
		void	addInstructionBefore(TessaInstruction* instruction, TessaInstruction* insertBefore);
		void	removeInstruction(TessaInstruction* instruction);
		void	addPhiInstruction(PhiInstruction* phiInstruction);
		void	addParameterInstruction(ParameterInstruction* phiInstruction);
		bool	isLoopHeader();
		void	setLoopHeader(bool loopHeader);
		bool	isSwitchHeader();
		void	setSwitchHeader(bool switchHeader);
		void	printResults();

		void	addPredecessor(TessaInstruction* predecessorTerminator);
		void	removePredecessor(TessaInstruction* predecessorTerminator);
		bool	isPredecessor(BasicBlock* predecessor);
		bool	isSuccessor(BasicBlock* successor);
		int		getInstructionIndex(TessaInstruction* instruction);

		int getBasicBlockId();
		List<TessaInstruction*, avmplus::LIST_GCObjects>* getInstructions();
		List<PhiInstruction*, avmplus::LIST_GCObjects>* getPhiInstructions();
		List<ParameterInstruction*, avmplus::LIST_GCObjects>* getParameterInstructions();
		List<BasicBlock*, avmplus::LIST_GCObjects>* getSuccessors();
		List<BasicBlock*, avmplus::LIST_GCObjects>* getPredecessors();
		TessaInstruction* getLastInstruction();
		void cleanInstructions();

		void setBeginState(StateVector* stateVector); 
		void setEndState(StateVector* stateVector); 
		StateVector*	getEndState();
		StateVector*	getBeginState();
	};
}