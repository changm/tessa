namespace TessaInstructions {
	class CoerceInstruction: public TessaInstruction {
	private:
		const Multiname* multinameToCoerce;
		
	public:
		bool useCoerceObjAtom;
		Traits* resultTraits;

	public:
		TessaInstruction*	_instructionToCoerce;
		CoerceInstruction(TessaTypes::Type* typeToConvertTo, TessaInstruction* instructionToCoerce, TessaVM::BasicBlock* insertAtEnd);
		CoerceInstruction(const Multiname* multinameToCoerce, TessaInstruction* instructionToCoerce, TessaTypes::Type* typeToConvert, TessaVM::BasicBlock* insertAtEnd);
		bool isCoerce(); 
		bool producesValue();
		TessaTypes::Type*		getTypeToCoerce();
		TessaInstruction*	getInstructionToCoerce();
		void				setTypeToCoerce(TessaTypes::Type* newType);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isMultinameCoerce();
		const Multiname* getMultinameToCoerce();
		Traits* getResultTraits();
		CoerceInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}