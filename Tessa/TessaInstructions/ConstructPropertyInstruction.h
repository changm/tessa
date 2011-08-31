
namespace TessaInstructions {
	class ConstructPropertyInstruction : public ConstructInstruction {
	public:
		ConstructPropertyInstruction(TessaInstruction *receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, const Multiname* propertyMultiname, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isConstructProperty();
		ConstructPropertyInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd); 
	};
}