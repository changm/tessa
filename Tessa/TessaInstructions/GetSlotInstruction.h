
namespace TessaInstructions {
	class GetSlotInstruction : public SlotAccessInstruction {
	private:

	public:
		GetSlotInstruction(int32_t slotNumber, int32_t slotOffset, TessaInstruction* receiverInstruction, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* visit);
		bool producesValue();
		GetSlotInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}