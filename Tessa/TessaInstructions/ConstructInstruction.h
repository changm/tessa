namespace TessaInstructions {
	class ConstructInstruction : public CallInstruction {
	private:
		const Multiname* propertyMultiname;
		int slotId;
		int constructorIndex;
		Traits* objectTraits;

	public:
		ConstructInstruction(TessaInstruction *receiverObject, Traits* classTraits, ArrayOfInstructions* arguments, const Multiname* multiname, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isConstruct();
		const Multiname* getPropertyMultiname();
		bool producesValue();
		bool isEarlyBound();
		int getSlotId();
		void setSlotId(int slotId);
		int getConstructorIndex();
		void setConstructorIndex(int constructorIndex);
		void setObjectTraits(Traits* traits);
		Traits* getObjectTraits();
		void setEarlyBindPointersForClone(ConstructInstruction* clonedConstructInstruction); 
		ConstructInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd); 
	};
}