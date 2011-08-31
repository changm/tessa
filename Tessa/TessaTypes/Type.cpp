#include "TessaTypeHeader.h"
#include "TessaAssert.h"

using namespace TessaInstructions;

namespace TessaTypes {
	Type* BooleanType::getStrictestType(Type* otherType) { 
		if (this->isEquivalentType(otherType)) {
			return otherType;
		} else {
			return TypeFactory::getInstance()->anyType();
		}
	}

	Type* NumberType::getStrictestType(Type* otherType) {
		if (otherType->isNumeric()) {
			return this;
		} else {
			//AvmAssert(otherType->isAnyType());
			//return otherType;
			return TypeFactory::getInstance()->anyType();
		}
	}

	Type* NumberType::getMostConcreteType(Type* otherType) {
		if (!otherType->isNumeric()) {
			return this;
		} else {
			return otherType;
		}
	}

	Type* ObjectType::getStrictestType(Type* otherType) {
		if (otherType->isAnyType() || otherType->isObject()) {
			return otherType;
		} else if (otherType->isUndefined()) {
			return this;
		} else {
			return TypeFactory::getInstance()->anyType();
		}
	}

	Type* ObjectType::getMostConcreteType(Type* otherType) {
		if (otherType->isAnyType()) {
			return this;
		} else {
			return otherType;
		}
	}

	// Object is lowest on the hierarchy here
	bool ObjectType::isDownCast(Type* otherType) {
		return false;
	}

	Type* PointerType::getStrictestType(Type* otherType) {
		if (otherType->isPointer()) {
			return otherType;
		} else {
			return TypeFactory::getInstance()->anyType();
		}
	}

	Type* PointerType::getMostConcreteType(Type* otherType) {
		if (otherType->isPointer()) {
			return otherType;
		} else {
			return this;
		}
	}

	Type* NullType::getStrictestType(Type* otherType) {
		if (otherType->isNull()) {
			return this;
		} else {
			return TypeFactory::getInstance()->anyType();
		}
	}

	Type* UndefinedType::getStrictestType(Type* otherType) {
		return otherType;
		/*
		if (otherType->isUndefined()) {
			return this;
		} else {
			return otherType;
		}
		/*
		else if (otherType->isNumeric()) {
			// Other types can represent undefined easily
			return otherType;
			//return TypeFactory::getInstance()->anyType();
		}  else {
			return TypeFactory::getInstance()->anyType();
		}
		*/
	}

	Type* UndefinedType::getMostConcreteType(Type* otherType) {
		// We're only better than any type
		if (otherType->isAnyType()) {
			return this;
		} else {
			return otherType;
		}
	}

	Type* IntegerType::getStrictestType(Type* otherType) {
		if (otherType->isNumeric()) {
			return otherType;
		} else {
			return TypeFactory::getInstance()->anyType();
		}
	}

	Type* IntegerType::getMostConcreteType(Type* otherType) {
		if (otherType->isBoolean()) {
			return otherType;
		} else {
			return this;
		}
	}

	Type* UnsignedIntegerType::getMostConcreteType(Type* otherType) {
		if (otherType->isBoolean()) {
			return otherType;
		} else {
			return this;
		}
	}

	Type* UnsignedIntegerType::getStrictestType(Type* otherType) {
		if (otherType->isNumeric()) {
			return otherType;
		} else {
			return TypeFactory::getInstance()->anyType();
		}
	}

	ProbabilisticType::ProbabilisticType(MMgc::GC* gc) {
		_gc = gc;
		_probability = new (gc) List<double, LIST_NonGCObjects>(_gc);
		_probableType = new (gc) List<Type*, LIST_GCObjects>(_gc);
	}

	std::string ProbabilisticType::toString() {
		return "probable type!";
	}

	bool ProbabilisticType::listsSameSize() {
		return _probableType->size() == _probability->size();
	}

	/***
	 * Adds a single type to the probabilitic type. 
	 * The probability for the added type is defaulted to 100%.
	 */
	void ProbabilisticType::addProbableType(Type* newProbableType) {
		AvmAssert(listsSameSize());
		_probableType->add(newProbableType);
		_probability->add(100.00);	// Defaults to 100%
	}

	void ProbabilisticType::updateProbability(Type* typeToUpdate, double newProbability) {
		int index = getIndex(typeToUpdate);
		_probability->set(index, newProbability);
	}

	int ProbabilisticType::getIndex(Type* probableType) {
		AvmAssert(containsType(probableType));
		return _probableType->indexOf(probableType);
	}

	double ProbabilisticType::getProbability(Type* type) {
		return 0.0;
	}

	Type* ProbabilisticType::getMostProbableType() {
		AvmAssert(false);
		return NULL;
	}

	bool ProbabilisticType::hasSingleType() {
		AvmAssert(listsSameSize());
		return _probableType->size() == 1;
	}

	int ProbabilisticType::getNumberOfTypes() {
		AvmAssert(listsSameSize());
		return _probableType->size();
	}

	bool ProbabilisticType::containsType(Type* type) {
		AvmAssert(listsSameSize());
		return _probableType->contains(type);
	}

	CompositeType::CompositeType(MMgc::GC* gc) {
		this->_containedTypes = new (gc) avmplus::List<Type*, avmplus::LIST_GCObjects>(gc);
		_gc = gc;
	}

	CompositeType::CompositeType(MMgc::GC* gc, Type* elementType) {
		this->_containedTypes = new (gc) avmplus::List<Type*, avmplus::LIST_GCObjects>(gc);
		_gc = gc;
		addContainedType(elementType);
	}

	Type* CompositeType::getContainedType(int index) {
		return _containedTypes->get(index);
	}

	void CompositeType::addContainedType(Type* containedType) {
		_containedTypes->add(containedType);
	}

	Type* ArrayType::getElementType() {
		AvmAssert(containedTypes() == 1);
		return _containedTypes->get(0);
	}

	Type* VectorType::getElementType() {
		AvmAssert(containedTypes() == 1);
		return _containedTypes->get(0);
	}
}