
#ifndef __TESSA_TYPE_HEADER
#define __TESSA_TYPE_HEADER

#pragma warning(disable:4291) // Disable matching delete operator. OCcurs due to enabling C++ exceptions

#include "TessaAssert.h"
#include "avmplus.h"

using namespace avmplus;

/****
 * This represents the constant primitive types in the ActionScript specification.
 */
namespace TessaTypes {
	class TypeFactory;
	class Type;
	class BooleanType;
	class NumberType;
	class IntegerType;
	class UnsignedIntegerType;
	class StringType;
	class AnyType;
	class ObjectType;
	class ArrayType;
	class TessaValue;
	class ConstantValue;
}

#include "Type.h"
#include "TessaValue.h"
#include "ConstantValue.h"		// Relies on TessaValue
#include "TypeFactory.h"		// Relies on Type.h

#endif