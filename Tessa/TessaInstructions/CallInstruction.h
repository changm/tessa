
namespace avmplus {
	class MethodInfo;
}

namespace TessaInstructions {
	using namespace avmplus;

	class CallInstruction : public TessaInstruction {
	private:
		MethodInfo*				_methodInfo;
		ArrayOfInstructions*	_arguments;
		TessaInstruction*		_receiverObject;
		Traits*					_resultTraits;
		TessaInstruction*		_functionObject;
		uintptr_t				_methodId;

		/***
		 * This is a terrible hack. We need a better way of making the CallInstruction map cleanly to other ABC opcodes.
		 * The problem with a function pointer, which we'll probably have to do sooner or later, is that you have to manually create typedefs
		 * to map the function pointer to a correct method. In addition, having only an address is a pain to debug.
		 */
		AbcOpcode				_abcOpcode;

	public:
		CallInstruction(TessaInstruction* receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd);
		CallInstruction(TessaInstruction* functionObject, TessaInstruction *receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, TessaVM::BasicBlock* insertAtEnd);
		CallInstruction(AbcOpcode abcOpcode, TessaInstruction* functionObject, ArrayOfInstructions* arguments, TessaVM::BasicBlock* insertAtEnd);
		TessaInstruction*	getFunctionObject();
		TessaInstruction*	getReceiverObject();
		ArrayOfInstructions*	getArguments();
		MethodInfo*			getMethodInfo();
		uintptr_t			getMethodId();
		uint32_t			getNumberOfArgs();
		bool				producesValue();
		bool				isDynamicMethod();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor); 
		AbcOpcode			getAbcOpcode();
		void				setAbcOpcode(AbcOpcode opcode);
		bool				hasAbcOpcode();
		bool				isStaticCall();
		bool				isResolved();
		Traits*				getResultTraits();
		virtual bool		isCallInterface();
		bool				isCall();
		bool				isInlined();
		CallInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
		bool hasSideEffect() { return true; }
	};
}