
namespace avmplus {
	class GetCache;
}

namespace TessaInstructions {
	class GetPropertyInstruction : public PropertyAccessInstruction {
	private:
		const Multiname*	_propertyName;

	public:
		Traits* objectTraits;
		Traits* resultTraits;
		GetCache* getCacheSlot;

		/***
		 * Need the offsetof(GetCache, _get_handler) here because LIR generators can't include CodegenLIR.h, so
		 * set this in the SWF translators
		 */
		int					getCacheGetHandlerOffset;
		bool				usePropertyCache;

	public:
		GetPropertyInstruction(TessaInstruction* receiverInstruction, TessaInstruction*	propertyKey, const Multiname* propertyMultiname, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd);
		bool				isGetProperty();
		const Multiname*	getPropertyMultiname();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool producesValue();
		GetPropertyInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}