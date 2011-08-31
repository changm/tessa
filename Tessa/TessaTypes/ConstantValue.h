
namespace TessaTypes {
	class ConstantValue : public TessaValue {
	public:
		ConstantValue();
		bool	isConstantValue();

		virtual bool isString();
		virtual bool isNumber();
		virtual bool isInteger();
		virtual bool isUndefined();
		virtual bool isNull();
		virtual bool isBoolean();
		virtual string toString();
		virtual bool isPointer();
	}; // End constant value class

	class ConstantInt : public ConstantValue {
	private:
		int _value;
		bool _isSigned;

	public:
		ConstantInt(int value, bool isSigned);
		int getValue();
		bool isInteger();
		string toString();
	}; // End constant int

	class ConstantPtr : public ConstantValue {
	private:
		intptr_t _value;

	public:
		ConstantPtr(intptr_t value);
		intptr_t getValue();
		string toString();
		bool isPointer();
	};

	class ConstantBool : public ConstantValue {
	private:
		bool _value;

	public:
		ConstantBool(bool value);
		bool getValue();
		bool isBoolean();
		string toString();
	};

	class ConstantFloat : public ConstantValue {
	private:
		double _value;

	public:
		ConstantFloat(double value);
		double getValue();
		bool isNumber();
		string toString();
	};

	class ConstantString : public ConstantValue {
	private:
		Stringp _value;

	public:
		ConstantString(Stringp value);
		Stringp getValue();
		bool isString();
		string toString();
	};

	class ConstantNull : public ConstantValue {
	public:
		ConstantNull();
		bool isNull();
		string toString();
	};

	class ConstantUndefined : public ConstantValue {
	public:
		ConstantUndefined();
		bool isUndefined();
		string toString();
	};

}	// End namespace