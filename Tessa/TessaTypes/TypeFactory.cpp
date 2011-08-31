
#include "TessaTypeHeader.h"

namespace TessaTypes {

	TypeFactory::TypeFactory(MMgc::GC* gc) 
		: MMgc::GCRoot(gc)
	{
		_gc = gc;

		_anyType = new (gc) AnyType();
		_boolType = new (gc) BooleanType(); 
		_signedInt = new (gc) IntegerType();
		_uintType = new (gc) UnsignedIntegerType();
		_numberType = new (gc) NumberType();
		_pointerType = new (gc) PointerType();
		_stringType = new (gc) StringType();
		_objectType = new (gc) ObjectType();
		_scriptObjectType = new (gc) ScriptObjectType();
		_voidType = new (gc) VoidType();
		_nullType = new (gc) NullType();
		_undefinedType = new (gc) UndefinedType();
		_unknownType = new (gc) UnknownType();

		_anyArrayType = new (gc) ArrayType(gc, _anyType);
		_anyVectorType = new (gc) VectorType(gc, _anyType);
		_intVectorType = new (gc) VectorType(gc, _signedInt);
		_uintVectorType = new (gc) VectorType(gc, _uintType);
		_numberVectorType = new (gc) VectorType(gc, _numberType);
		_objectVectorType = new (gc) VectorType(gc, _objectType);
	}

	TypeFactory::~TypeFactory() {

	}

	TypeFactory* TypeFactory::_singleton = NULL;	// static initializer
	void TypeFactory::initInstance(MMgc::GC* gc) {
		if (_singleton == NULL) {
			_singleton = new TypeFactory(gc);
		}
	}

	TypeFactory* TypeFactory::getInstance() {
		AvmAssert(_singleton != NULL);
		return _singleton;
	}

	void TypeFactory::deleteInstance() {
		if (_singleton != NULL) {
			delete _singleton;
		}
	}
}