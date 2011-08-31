
namespace TessaInstructions {
	class PropertyAccessInstruction : public TessaInstruction {
	private:

	protected:
		TessaInstruction*	_receiverInstruction;
		TessaInstruction*	_propertyKey;
		const Multiname*	_propertyName;
		Traits*				indexTraits;

		/***
		 * We decide early bind decisions during tessa creation time. These are the results of those decisions
		 */
	public:
		TessaTypes::Type* indexType;
		TessaTypes::Type* objectType;

	public:
		PropertyAccessInstruction(TessaInstruction* receiverInstruction, TessaInstruction* propertyKey, const Multiname* propertyName, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd);
		bool isPropertyAccess();
		TessaInstruction*	getReceiverInstruction();
		const Multiname*	getPropertyName();
		void				setReceiverInstruction(TessaInstruction* newReceiverInstruction);
		void				setPropertyName(const Multiname* propertyName);
		Traits*				getIndexTraits();
		bool hasSideEffect() { return true; }	
		TessaInstruction*	getPropertyKey() { return _propertyKey; }
		void				setPropertyKey(TessaInstruction* newPropertyKey); 
	};
}