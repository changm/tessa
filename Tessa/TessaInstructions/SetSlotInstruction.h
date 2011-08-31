
namespace TessaInstructions {
	class SetSlotInstruction : public SlotAccessInstruction {
	private: 
		TessaInstruction *valueToSet;
		Traits*	slotTraits;

	public:
		SetSlotInstruction(uint32_t slotNumber, int32_t slotOffset, TessaInstruction* receiverObject, TessaInstruction* valueToSet, Traits* slotTraits, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		void setValueToSet(TessaInstruction* valueToset);
		TessaInstruction* getValueToSet();
		Traits*	getSlotTraits();
		SetSlotInstruction*	clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}