
#include <iostream>

using namespace std;

#pragma warning(disable:4100)	// Disable unreferenced variables as a warning, which causes the build to fail since warnings are errors

/***
 * A type describes the structure of a value.
 * Type Hierarchy:
 *               Any
 *   Number              Object            Pointer
 * int, uint, bool                       String, composite
 *                                                  [composite -> vector, array, probable, union]
 */
namespace TessaTypes {
	class Type : public MMgc::GCObject {
	public:
		Type() { }
		virtual bool isNumber() { return false; }
		virtual bool isInteger() { return false; }
		virtual bool isUnsignedInt() { return false; }
		virtual bool isBoolean() { return false; }
		virtual bool isString() { return false; }
		virtual bool isObject() { return false; }
		virtual bool isAnyType() { return false; }
		virtual bool isVoid() { return false; }
		virtual bool isUnknown() { return false; }
		virtual bool isNull() { return false; }
		virtual bool isUndefined() { return false; }
		virtual bool isSigned() { return false; }
		virtual bool isArray() { return false; }
		virtual bool isVector() { return false; }
		virtual bool isMethodEnv() { return false; }
		virtual bool isPointer() { return false; }
		virtual bool isScriptObject() { return false; }
		virtual bool isCompositeType() { return false; }
		virtual bool isProbabilisticType() { return false; }
		virtual bool isUnionType() { return false; }
		virtual bool isNumeric() { return isNumber() || isInteger() || isUnsignedInt() || isBoolean(); }
		virtual std::string toString() { return "type"; }

		/***
		 * The strictest type is the type where two different type can merge up
		 * onto the type ladder. eg int and number -> return number.
		 */
		virtual Type* getStrictestType(Type* otherType) { (void)otherType; AvmAssert(false); return this; }

		/***
		 * Most concrete type is the opposite of strictest type. This is the LOWEST 
		 * type on the type ladder. int and number -> int.
		 */
		virtual Type* getMostConcreteType(Type* otherType) { (void)otherType; AvmAssert(false); return this; }

		/***
		 * An equivalent type is one where the underlying binary representation is fundamentally the same
		 * For example, int and unsigned int, boolean. These are probably the only ones.
		 */
		virtual bool isEquivalentType(Type* otherType) { return this == otherType; }

		/***
		 * A compatible type is whether or not a specific type can represent the other type.
		 * A number can represent an integer, but an integer cannot represent an number.
		 * Any can represent string. etc.
		 */
		virtual bool isCompatibleType(Type* otherType) { (void)otherType; return false;} 

		/***
		 * A downcast is where the otherType where our type is lower on the the lattice. Going from any to anything else is a downcast.
		 * Going from any -> string is a downcast.
		 * number -> int is a downcast.
		 */
		virtual bool isDownCast(Type* otherType) { (void)otherType; return false; }

		/***
		 * A horizontal cast is where the other type is equivalent on the type lattice, but is
		 * horizontally on a different column. For example, the number and object types
		 */
		virtual bool isHorizontalType(Type* otherType) { (void)otherType; AvmAssert(false); return false; }
		

		virtual Type*	getContainedType(int index) { 
			(void)index; 
			return this; 
		}
	}; // End Type Class

	/***
	 * This represents a volatile type. The value can be anything
	 * and can switch at anytime.
	 */
	class AnyType : public Type {
	private:

	public:
		AnyType() { };
		bool isAnyType() { return true; };
		string toString() { return "any type"; }

		Type* getStrictestType(Type* otherType) { return this; }	// We have to give up here.
		Type* getMostConcreteType(Type* otherType) { return otherType; }	// Anything is better
		bool isCompatibleType(Type* otherType) { return true; } // Any is compatible with all!
		bool isDownCast(Type* otherType) { return true; }	// Can only get better from here.
		bool isHorizontalType(Type* otherType) { return false; }
	}; // End Anytype class

	/***
	 * This is an unknown type. It could later be changed to a well known
	 * type such as an object.
	 */
	class UnknownType : public Type {
	private:

	public:
		UnknownType() { };
		bool isUnknown() { return true; };
		string toString() { return "unknown type"; }
	}; // End Unknown class

	/***
	 * The ActionScript void type
	 */
	class VoidType : public Type {
	private:

	public:
		VoidType() {}
		bool isVoid() { return true; }
		string toString() { return "void"; }
	}; // End void class

	// By spec undefined is under voidtype
	class UndefinedType : public VoidType {
	private:

	public:
		UndefinedType() {}
		bool isUndefined() { return true; }
		string toString() { return "undefined"; }
		Type* getStrictestType(Type* otherType); 
		Type* getMostConcreteType(Type* otherType); 
		//bool isCompatibleType(Type* otherType) { return otherType->isUndefined() || otherType->isAnyType(); }
		bool isCompatibleType(Type* otherType) { return true; }
		bool isHorizontalType(Type* otherType) { return otherType->isObject() || otherType->isNumeric() || otherType->isPointer(); } 
	}; // End Undefined class

	/***
	 * By spec the null type exists in addition to void and undefined.
	 */
	class NullType : public Type {
	private:

	public:
		NullType() {}
		bool isNull() { return true; }
		string toString() { return "null"; }
		bool isCompatibleType(Type* otherType) { return true; }		// Everything can be null
		Type* getStrictestType(Type* otherType);
		Type* getMostConcreteType(Type* otherType) { if (otherType->isAnyType()) { return this; } else { return otherType; } }
		bool isHorizontalType(Type* otherType) { return !otherType->isNull(); }
	}; // End null class

	class BooleanType : public Type {
	private:

	public:
		BooleanType()  {}
		bool isBoolean() { return true; }
		string toString() { return "boolean"; }

		Type* getStrictestType(Type* otherType); 
		Type* getMostConcreteType(Type* otherType) { return this; }	// We're the lowest on the ladder
		bool isEquivalentType(Type* otherType) { return otherType->isInteger() || otherType->isUnsignedInt() || otherType->isBoolean(); } 
		bool isCompatibleType(Type* otherType) { return otherType->isBoolean() || otherType->isUndefined(); }
		bool isDownCast(Type* otherType) { return false; }	// We're the lowest on the ladder, cannot be downcasted into anything.
		bool isHorizontalType(Type* otherType) { return otherType->isPointer() || otherType->isObject(); }
	}; // End Boolean Type

	/***
	 * This represents a signed 32 bit integer type.
	 */
	class IntegerType : public Type {
	private:

	public:
		IntegerType() {};
		string toString() { return "int"; }
		bool isInteger() { return true; }
		bool isSigned() { return true; }

		Type* getStrictestType(Type* otherType); 
		Type* getMostConcreteType(Type* otherType); 
		bool isEquivalentType(Type* otherType) { return otherType->isInteger() || otherType->isUnsignedInt() || otherType->isBoolean(); } 
		bool isCompatibleType(Type* otherType) { return otherType->isBoolean() || otherType->isInteger() || otherType->isUnsignedInt() || otherType->isUndefined(); }
		bool isDownCast(Type* otherType) { return otherType->isBoolean(); }	
		bool isHorizontalType(Type* otherType) { return otherType->isPointer() || otherType->isObject(); }
	}; // End IntegerType

	/***
	 * Keep this as a separate type from an integer type because
	 * it's easy to do type checks using the isInteger() without
	 * checking if it's an unsigned integer, which will lead to
	 * difficult subtle bugs. Also uint is a primitive type in ActionScript.
	 */
	class UnsignedIntegerType : public Type {
	private:

	public:
		UnsignedIntegerType() {}
		string toString() { return "unsigned int"; }
		bool isSigned() { return true; }
		bool isUnsignedInt() { return true; }
		bool isHigherOnTypeLattice(Type* newType) { return newType->isBoolean(); }

		Type* getMostConcreteType(Type* otherType); 
		Type* getStrictestType(Type* otherType); 
		bool isEquivalentType(Type* otherType) { return otherType->isInteger() || otherType->isUnsignedInt() || otherType->isBoolean(); } 
		bool isCompatibleType(Type* otherType) { return otherType->isBoolean() || otherType->isInteger() || otherType->isUnsignedInt(); }
		bool isDownCast(Type* otherType) { return otherType->isBoolean(); }	
		bool isHorizontalType(Type* otherType) { return otherType->isPointer() || otherType->isObject(); }
	}; // End IntegerType

	class NumberType : public Type {
	private:

	public:
		NumberType() { }
		bool isNumber() { return true; }
		string toString() { return "number"; }

		Type* getMostConcreteType(Type* otherType); 
		Type* getStrictestType(Type* otherType); 
		bool isEquivalentType(Type* otherType) { return otherType->isNumber(); }
		bool isCompatibleType(Type* otherType) { return otherType->isNumeric() || otherType->isUndefined(); }
		bool isDownCast(Type* otherType) { return otherType->isBoolean() || otherType->isInteger() || otherType->isUnsignedInt(); }
		bool isHorizontalType(Type* otherType) { return otherType->isPointer() || otherType->isObject(); }
	}; // End Number Type

	class ObjectType : public Type {
	private:

	public:
		ObjectType() {}
		bool isObject() { return true; }
		string toString() { return "object"; }

		Type* getMostConcreteType(Type* otherType); 
		Type* getStrictestType(Type* otherType); 
		bool isEquivalentType(Type* otherType) { return otherType->isObject(); }
		bool isCompatibleType(Type* otherType) { return otherType->isObject() || otherType->isAnyType() || otherType->isUndefined(); }
		bool isDownCast(Type* otherType); 
		bool isHorizontalType(Type* otherType) { return otherType->isPointer() || otherType->isNumeric(); }
	}; // End Object Type

	class ProbabilisticType : public Type {
	private:
		/***
		 * These two lists must be the same size and are indexable by the same index.
		 * eg, probableType[0]'s probability of being that type is _probability[0]
		 */
		List<Type*, LIST_GCObjects>* _probableType;
		List<double, LIST_NonGCObjects>* _probability;
		MMgc::GC* _gc;
		bool listsSameSize();
		int getIndex(Type* probableType);

	public:
		ProbabilisticType(MMgc::GC* gc); 
		bool isProbabilisticType() { return true; }
		string toString();
		void addProbableType(Type* newProbableType);
		void updateProbability(Type* typeToUpdate, double newProbability);
		double getProbability(Type* typeToGet);
		Type* getMostProbableType();
		bool hasSingleType(); 
		int getNumberOfTypes(); 
		bool containsType(Type* type); 
	};

	class PointerType : public Type {
	private:

	public:
		PointerType() {} 
		string toString() { return "pointer"; }
		bool isPointer() { return true; }
		Type* getStrictestType(Type* otherType);
		Type* getMostConcreteType(Type* otherType);
		bool isCompatibleType(Type* otherType) { return otherType->isPointer(); }
		bool isHorizontalType(Type* otherType) { return otherType->isNumeric() || otherType->isObject() || otherType->isNull(); }
	}; // End PointerType

	class StringType : public PointerType {
	private:

	public:
		StringType() {}
		string toString() { return "string"; }
		bool isString() { return true; }
		bool isCompatibleType(Type* otherType) { return true; }		// Anything can be turned into a string representation
	}; // End String Type

	class ScriptObjectType : public PointerType {
	private:

	public:
		ScriptObjectType() {}
		string toString() { return "ScriptObject"; }
		bool isScriptObject() { return true; }
		bool isCompatibleType(Type* otherType) { return otherType->isScriptObject() || otherType->isUndefined(); }
	}; // End ScriptObject

	/***
	 * The method env is a special actionscript object and is the first
	 * parameter to every method that's called. That's why it gets a special
	 * type.
	 */
	class MethodEnvType : public PointerType {
	public:
		MethodEnvType() {}
		string toString() { return "Method Environment"; }
		bool isMethodEnv() { return true; }
	};

	/**
	 * A composite type is a type that contains other types. For example,
	 * An array is a composite type since the array type is really
	 * Just an array of the element type.
	 */
	class CompositeType : public PointerType {
	private:

	protected:
		// A contained type is the type of each element in this composite 
		avmplus::List<Type*, avmplus::LIST_GCObjects>* _containedTypes;
		MMgc::GC* _gc;
		virtual ~CompositeType() {}

	public:
		CompositeType(MMgc::GC* gc); 
		CompositeType(MMgc::GC* gc, Type* elementType);
		string toString() { return "composite type"; }
		bool isCompositeType() { return true; }

		Type* getContainedType(int index);
		void addContainedType(Type* containedType);
		virtual Type* getElementType() { AvmAssert(false); return NULL; }
		int containedTypes() { return _containedTypes->size(); }
	};

	class ArrayType : public CompositeType {
	private:
	public:
		ArrayType(MMgc::GC* gc) : CompositeType(gc) {}
		ArrayType(MMgc::GC* gc, Type* elementType) : CompositeType(gc, elementType) {}
		bool isCompatibleType(Type* otherType) { return otherType->isArray() || otherType->isUndefined();} 
		bool isArray() { return true; }
		string toString() { return "array type"; }
		Type* getElementType();
	}; // End Array Type

	class VectorType : public CompositeType {
	private:
	public:
		VectorType(MMgc::GC* gc) : CompositeType(gc) {}
		VectorType(MMgc::GC* gc, Type* elementType) : CompositeType(gc, elementType) {}
		VectorType(Type* elementType);
		bool isVector() { return true; }
		string toString() { return "vector type"; }
		Type* getElementType();
	}; // End VectorType Type
}