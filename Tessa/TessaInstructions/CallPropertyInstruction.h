
namespace avmplus {
	class CallCache;
}

namespace TessaInstructions {
	using namespace avmplus;

	class CallPropertyInstruction : public CallInstruction {
	private:
		const Multiname* _propertyName;
		GetSlotInstruction* _functionValue;
		bool _isSlotBound;

	public:
		CallCache* cacheSlot;

		/***
		 * The call cache declarations exist in CodegenLIR.h. However, we need to do offsetof(CallCache, _handler) in the LIR generators
		 * We can either mess with the build and include CodegenLIR.h or at this instruction creation time, set the offset.
		 * We can alleviate this issue once we pull the binding caches out of codegenlir 
		 */
		int cacheHandlerOffset;

	public:
		CallPropertyInstruction(GetSlotInstruction* functionValue, TessaInstruction* receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, const Multiname* propertyName, TessaVM::BasicBlock* insertAtEnd);
		CallPropertyInstruction(TessaInstruction* receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, const Multiname* propertyName, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		const Multiname* getProperty();
		bool hasValidCacheSlot();
		CallPropertyInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		bool isSlotBound();
		GetSlotInstruction* getFunctionValue();
	};

}