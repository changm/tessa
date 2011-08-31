
/****
 * Convert is slightly like coerce. The semantics are just slightly different. Look at AVM2 spec for convert_type to see different
 */
namespace TessaInstructions {
	class ConvertInstruction : public CoerceInstruction {
	private:

	public:
		ConvertInstruction(TessaTypes::Type* typeToConvertTo, TessaInstruction* instructionToConvert, TessaVM::BasicBlock* insertAtEnd);
		bool isConvert();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool producesValue();
		ConvertInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
	};
}
