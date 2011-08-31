/***
 * The value class is a key component in the Tessa IR.
 * Instructions operate on multiple values and can produce multiple values.
 * Values have a type associated with them.
 * Values have no execution semantics.
 */

namespace TessaInstructions {
	class TessaInstruction;
}

namespace TessaTypes {
	class TessaValue : public MMgc::GCObject {
	private:
		Type*	_type;
		static int nextAvailableId;

	protected:
		int _id;

	public:
		TessaValue();
		TessaValue(Type* type);

		Type*	getType();
		void	setType(Type *type);

		virtual bool isInstruction();
		virtual bool isConstantValue();

		bool	isNumeric();
		bool	isInteger();
		bool	isUnsignedInteger();
		bool	isBoolean();
		bool	isNumber();
		bool	isArray();
		bool	isObject();
		bool	isString();
		bool	isAny();
		bool	isPointer();
		bool	isScriptObject();
		bool	isVector();

		virtual std::string		toString();
		virtual TessaValue*		resolve();
		virtual string getOperandString();

		int getValueId();
		TessaInstructions::TessaInstruction* toInstruction();
	};
}