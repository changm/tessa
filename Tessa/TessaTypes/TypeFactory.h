
namespace TessaTypes {
	class TypeFactory : public MMgc::GCRoot {
	private:
		static TypeFactory* _singleton;
		MMgc::GC*_gc;

		/***
		 * Primitive types, all singletons
		 */
		AnyType* _anyType;
		BooleanType* _boolType; 
		IntegerType* _signedInt; 
		UnsignedIntegerType* _uintType; 
		NumberType* _numberType;
		PointerType* _pointerType;
		StringType* _stringType;
		ObjectType* _objectType;
		ScriptObjectType* _scriptObjectType;
		VoidType* _voidType;
		NullType* _nullType;
		UndefinedType* _undefinedType;
		UnknownType* _unknownType;

		ArrayType* _anyArrayType;
		VectorType* _anyVectorType;
		VectorType* _intVectorType;
		VectorType* _uintVectorType;
		VectorType* _numberVectorType;
		VectorType* _objectVectorType;

		TypeFactory(MMgc::GC* gc);
		~TypeFactory();
		

	public:
		static void initInstance(MMgc::GC* gc);
		static TypeFactory* getInstance();
		static void deleteInstance();

		/***
		 * Getters
		 */
		AnyType* anyType() const { return _anyType; }
		BooleanType* boolType() const { return _boolType; }
		IntegerType* integerType() const { return _signedInt; }
		UnsignedIntegerType* uintType() const { return _uintType; }
		UndefinedType* undefinedType() const { return _undefinedType; }
		NullType* nullType() const { return _nullType; }
		VoidType* voidType() const { return _voidType; }
		ScriptObjectType* scriptObjectType() const { return _scriptObjectType; }
		NumberType* numberType() const { return _numberType; }
		PointerType* pointerType() const { return _pointerType; }
		StringType* stringType() const { return _stringType; }
		ObjectType* objectType() const { return _objectType; }
		UnknownType* unknownType() const { return _unknownType; }

		ArrayType* anyArrayType() const { return _anyArrayType; }
		VectorType* anyVectorType() const { return _anyVectorType; }
		VectorType* intVectorType() const { return _intVectorType; }
		VectorType* uintVectorType() const { return _uintVectorType; }
		VectorType* numberVectorType() const { return _numberVectorType; }
		VectorType* objectVectorType() const { return _objectVectorType; }
	};
}