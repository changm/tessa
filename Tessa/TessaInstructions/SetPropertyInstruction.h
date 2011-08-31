
namespace avmplus {
	class SetCache;
}

namespace TessaInstructions {
	class SetPropertyInstruction : public PropertyAccessInstruction {
	private:
		TessaInstruction* _valueToSet;

	public:
		Traits* objectTraits;
		Traits* valueTraits;
		SetCache* setCacheSlot;
		int					setCacheHandlerOffset;
		bool				usePropertyCache;

	public:
		SetPropertyInstruction(TessaInstruction* receiverInstruction, TessaInstruction* propertyKey, TessaInstruction *valueToSet, const Multiname* multiname, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd);
		TessaInstruction*	getValueToSet();
		
		bool isSetProperty();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		SetPropertyInstruction*	clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}
