#include "TessaTypeHeader.h"

namespace TessaTypes {
	/***
	 * Constant Value Class Begin
	 */
	ConstantValue::ConstantValue() {
	}

	bool ConstantValue::isConstantValue() {
		return true;
	}

	bool ConstantValue::isString() {
		return false;
	}

	bool ConstantValue::isNumber() {
		return false;
	}

	bool ConstantValue::isInteger() {
		return false;
	}

	bool ConstantValue::isUndefined() {
		return false;
	}

	bool ConstantValue::isNull() {
		return false;
	}

	bool ConstantValue::isBoolean() {
		return false;
	}

	bool ConstantValue::isPointer() {
		return false;
	}

	string ConstantValue::toString() {
		return "ConstantValue";
	}

	/***
	 * Constant int class begin
	 */
	ConstantInt::ConstantInt(int value, bool isSigned) 
	{
		this->_value = value;
		_isSigned = isSigned;
		if (isSigned) {
			setType(TypeFactory::getInstance()->integerType());
		} else {
			setType(TypeFactory::getInstance()->uintType());
		}
	}

	int ConstantInt::getValue() {
		return _value;
	}

	bool ConstantInt::isInteger() {
		return true;
	}
	
	string ConstantInt::toString() {
		char buffer[32];
		_itoa(_value, buffer, 10);
		return buffer;
	} // End constant Int

	ConstantPtr::ConstantPtr(intptr_t value) {
		_value = value;
		setType(TypeFactory::getInstance()->pointerType());
	}

	intptr_t ConstantPtr::getValue() {
		return _value;
	}

	bool ConstantPtr::isPointer() {
		return true;
	}

	string ConstantPtr::toString() {
		char buffer[32];
		//_itoa(_value, ptrValue, 10);
		VMPI_snprintf(buffer, sizeof(buffer), "ptr 0x%x", _value);
		return buffer;
	} // End constant ptr

	/***
	 * Constant boolean 
	 */
	ConstantBool::ConstantBool(bool value) {
		_value = value;
		setType(TypeFactory::getInstance()->boolType());
	}

	bool ConstantBool::getValue() {
		return _value;
	}

	string ConstantBool::toString() {
		if (_value) {
			return "true";
		} else {
			return "false";
		}
	}
	
	bool ConstantBool::isBoolean() {
		return true;
	}

	/***
	 * Constant Floating Point
	 */
	ConstantFloat::ConstantFloat(double value) 
	{
		_value = value;
		setType(TypeFactory::getInstance()->numberType());
	}

	double ConstantFloat::getValue() {
		return _value;
	}

	string ConstantFloat::toString() {
		char buffer[32];
		VMPI_snprintf(buffer, sizeof(buffer), "%f", _value);
		return buffer;
	}

	bool ConstantFloat::isNumber() {
		return true;
	}

	/***
	 * ConstantString 
	 */
	ConstantString::ConstantString(Stringp value) {
		_value = value;
		setType(TypeFactory::getInstance()->stringType());
	}

	Stringp ConstantString::getValue() {
		return _value;
	}

	string ConstantString::toString() {
		StUTF8String utf8String(_value);
		const char* charValue = utf8String.c_str();
		string stringValue = "\"";
		stringValue += charValue;
		stringValue += "\"";
		return stringValue;
	}

	bool ConstantString::isString() {
		return true;
	}

	/***
	 * Constant null
	 */
	ConstantNull::ConstantNull() 
	{
		setType(TypeFactory::getInstance()->nullType());
	}

	bool ConstantNull::isNull() {
		return true;
	}

	string ConstantNull::toString() {
		return "null";
	}

	/***
	 * Constant undefined
	 */
	ConstantUndefined::ConstantUndefined() 
	{
		setType(TypeFactory::getInstance()->undefinedType());
	}

	bool ConstantUndefined::isUndefined() {
		return true;
	}

	string ConstantUndefined::toString() {
		return "Undefined";
	}
}