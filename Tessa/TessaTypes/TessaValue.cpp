#include "TessaTypeHeader.h"

namespace TessaTypes {
	int TessaValue::nextAvailableId = 0;

	TessaValue::TessaValue() {
		setType(TypeFactory::getInstance()->anyType());
		_id = this->nextAvailableId++;
	}

	TessaValue::TessaValue(Type* type) {
		setType(type);
		_id = this->nextAvailableId++;
	}

	Type* TessaValue::getType() {
		return _type;
	}

	void TessaValue::setType(Type *type) {
		AvmAssert(type != NULL);
		_type = type;
	}
	
	bool TessaValue::isInstruction() {
		return false;
	}

	bool TessaValue::isConstantValue() {
		return false;
	}

	bool TessaValue::isInteger(){ 
		return _type->isInteger(); 
	}

	bool TessaValue::isUnsignedInteger() {
		return _type->isUnsignedInt(); 
	}

	bool TessaValue::isBoolean() {
		return _type->isBoolean(); 
	}

	bool TessaValue::isNumber() {
		return _type->isNumber();
	}

	bool TessaValue::isArray() {
		return this->_type->isArray();
	}

	bool TessaValue::isObject() {
		return _type->isObject(); 
	}

	bool TessaValue::isNumeric() {
		return isInteger() || isNumber() || isUnsignedInteger();
	}

	bool TessaValue::isString() {
		return _type->isString();
	}

	bool TessaValue::isAny() {
		return _type->isAnyType(); 
	}

	bool TessaValue::isPointer() {
		return _type->isPointer();
	}

	bool TessaValue::isVector() {
		return _type->isVector();
	}

	bool TessaValue::isScriptObject() {
		return _type->isScriptObject(); 
	}

	TessaValue*	TessaValue::resolve() {
		return this;
	}

	string TessaValue::toString() {
		char buffer[32];
		VMPI_snprintf(buffer, sizeof(buffer), "Value $%d", getValueId());
		return buffer;
	}

	string TessaValue::getOperandString() {
		char buffer[32];
		VMPI_snprintf(buffer, sizeof(buffer), this->toString().data());
		return buffer;
	}

	int TessaValue::getValueId() {
		return _id;
	}

	TessaInstructions::TessaInstruction* TessaValue::toInstruction() {
		if (this->isInstruction()) {
			return reinterpret_cast<TessaInstructions::TessaInstruction*>(this);
		} else {
			AvmAssertMsg(false, "False cast from TessaValue to tessaInstruction\n\n");
			return NULL;
		}
	}
}