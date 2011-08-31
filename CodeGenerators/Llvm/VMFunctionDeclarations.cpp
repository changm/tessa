
/***
 * LLVM jitted code has to call back into the VM.
 * However, the VM C++ is not compiled by LLVM, and instead GCC/MSVC.
 * In order for JIT compiled LLVM code to call back into the VM, LLVM has to have llvm::Function definitions to call to.
 * This is required by the llvm::CallInst object. Therefore, we have to create llvm::Function* objects and add them to the llvm::module
 * We could manually create the llvm::Function* via typing them in C++ source code, but this is annoying, error prone, 
 * and is a huge swath of depedencies.
 *
 * Instead, we want to declare all the VM C++ functions here. Then we can compile this file with llvm
 * which will automatically generate the C++ function objects for us. During runtime, we then have to link these
 * function declarations with their actual addresses.

 * These are only the DECLARATIONS so that we can compile and get the llvm::Function objects 
 * easily by building ONLY this file with LLVM, and not ALL of AVM. We only want the llvm::function objects,
 * we don't care about any actual bitcode or method implementations.
 *
 * Use the build script tamarin-redux/CodeGenerators/llvm/buildWrapperDeclarations.sh to get an updated LLVM bitcode file.
 *
 * This has to be a .cpp file otherwise LLVM won't compile it correctly. LLVM Ignores header files (grr).
 * Because it is a .cpp file, we have to exclude it from the normal build process as well. 
 * 
 * For methods that are C++ instance methods, the first argument of the declaration MUST be the "This" pointer
 */

// We have to forward declare everything. Can't include "avmplus.h" otherwise the llvm build gets complicated
// and this no longer stays a one file compilation
typedef int Atom;

namespace avmplus {
	class AvmCore;
	class ArrayObject;
	class ClassClosure;
	class MethodEnv;
	class MethodInfo;
	class MethodInfoProcHolder;
	class Multiname;
	class ScriptObject;
	class ScopeChain;
	class Toplevel;
	class Traits;
	class VTable;
	class String;
	class CallCache;
	class CallCache_Handler;
	class GetCache;
	class SetCache;
	class VectorBaseObject;
	class TypedVectorObject;
}

namespace MMgc {
	class GC;
}

#include <stdio.h>

using namespace avmplus;
using namespace MMgc;

/***
 * THESE HAVE TO HAVE EMPTY DEFINITIONS, otherwise LLVM won't build it if we just have declarations
 * Also, the argument orders MUST be correct. "This" C++ instance methods should have the instance be the first parameter
 */ 

// The standard wrapper to call a method
/*static*/ Atom verifyCoerceEnter(MethodEnv* env, int argc, Atom* args) { }

// Standard VM Core Methods
Atom op_add(AvmCore* core, Atom lhs, Atom rhs) { }
String* StringConcatStrings(String* leftStr, String* rightStr) { }
String* AvmCoreString(AvmCore* core, Atom atom) {}
String* AvmCoreCoerce_s(AvmCore* core, Atom atom) {}

void AvmCoreAtomWriteBarrier(MMgc::GC* gc, int* container, Atom *address, Atom atomNew) {}
void GCprivateWriteBarrierRC(MMgc::GC* gc, int* container, int *address, int *value) {}

VTable * toVTable(Toplevel* env, Atom atom) { }
ScriptObject* finddef_cache(MethodEnv* env, const Multiname* name, unsigned int slot) {}
double MathUtilsMod(double x, double y) { }
unsigned int toUInt32(Atom atom) {}
Atom uintToAtom(AvmCore* core, unsigned int n) {}
Atom op_construct(Toplevel* toplevel, Atom ctor, int argc, Atom* atomv) {} 
Atom constructprop(MethodEnv* env, const Multiname* multiname, int argc, Atom* atomv) {}
Atom op_call_toplevel(Toplevel*, Atom method, int argCount, Atom* atomv) {}
Atom op_call_methodEnv(MethodEnv*, Atom method, int argCount, Atom* atomv) {}
Atom MethodEnvOp_applytype(MethodEnv* env, Atom factory, int argc, Atom* args) {}
Toplevel* getToplevel(MethodEnv* env) {}
AvmCore* getCore(MethodEnv* env) {}

/*** 
 * Type conversion methods
 */
Atom AvmcoreBooleanAtom(Atom atom) {}
int AvmCoreboolean(Atom atom) {}
double number(Atom atom) { }
double number_d(Atom atom) { }
Atom doubleToAtom(AvmCore* core, double value) { }
int integer(Atom atom) {} 
int integer_i(Atom atom) {} 
int integer_u(Atom atom) {} 
int integer_d(double value) {}
int numberToInteger(double value) {}
Atom intToAtom(AvmCore* core, int value) {}

/***
 * Property Access methods
 */
void ArrayObjectSetIntProperty(ArrayObject* arrayObject, int index, Atom value) {}
void ArrayObjectSetUIntProperty(ArrayObject* arrayObject, unsigned int index, Atom value) {}
Atom ArrayObjectGetIntProperty(ArrayObject* arrayObject, int index) {}
Atom ArrayObjectGetUintProperty(ArrayObject* arrayObject, unsigned int index) {}

// Equality operators
Atom equals(AvmCore* core, Atom leftOperand, Atom rightOperand) {}
Atom compare(AvmCore* core, Atom leftOperand, Atom rightOperand) {}
Atom strictEquals(AvmCore* core, Atom leftOperand, Atom rightOperand) {}

// C++ Wrapper Methods

//Atom MethodInfoInvoke(MethodInfo* methodInfo, MethodEnv* env, int argc, Atom* args) {}
Atom MethodInfoInvoke(MethodInfo* methodInfo, MethodEnv* env, int argc, Atom* args) {}
Atom InvokeReturnInt(MethodEnv* env, int argc, Atom* args) {}
ScriptObject* InvokeReturnPointer(MethodEnv* env, int argc, Atom* args) {}
double InvokeReturnNumber(MethodEnv* env, int argc, Atom* args) {}

// Used for interface calls
Atom InterfaceInvokeReturnInt(MethodEnv* env, int argc, Atom* args, int* interfaceId) {}
ScriptObject* InterfaceInvokeReturnPointer(MethodEnv* env, int argc, Atom* args, int* interfaceId) {}
double InterfaceInvokeReturnNumber(MethodEnv* env, int argc, Atom* args, int* interfaceId) {}

// Cache handler calls
//typedef Atom (*CallCache_Handler)(CallCache&, Atom base, int argc, Atom* args, MethodEnv*);
Atom CallCacheHandler (CallCache&, Atom base, int argCount, Atom* arguments, MethodEnv* methodEnv) {}
Atom GetCacheHandler (GetCache& cache, MethodEnv* methodEnv, Atom object) {}
void SetCacheHandler (SetCache& cache, Atom object, Atom valueToSet, MethodEnv* methodEnv) {}

Atom MethodInfoInvokeTest(Toplevel* toplevel, Atom receiverObject, int methodId, int argc, Atom* args) { }
Atom ToplevelGetProperty(Toplevel* toplevel, Atom receiverObject, Atom key, const Multiname* multiname) {}
Atom TopLevelGetPropertyNonRuntime(Toplevel* toplevel, Atom receiverObject, const Multiname* multiname, VTable* vtable) {} 
Atom MethodEnvGetPropertyLateInteger(MethodEnv* methodEnv, Atom object, int index) {}
Atom MethodEnvGetPropertyLateUnsignedInteger(MethodEnv* methodEnv, Atom object, unsigned int index) {}
Atom jit_calls_GetPropertyIndex(MethodEnv* caller_env, Atom obj, const Multiname *name, Atom index) {}

void SetPropertyIndex(MethodEnv* caller_env, Atom obj, const Multiname* name, Atom value, Atom index) {}
void ToplevelSetProperty(Toplevel* toplevel, Atom receiverObject, Atom valueToSet, Atom key, const Multiname* propertyName) {} 
void ToplevelSetPropertyNonRuntime(Toplevel* toplevel, Atom receiverObject, const Multiname* propertyName, Atom valueToset, VTable* vtable) {}
void MethodEnvSetPropertyLateInteger(MethodEnv* methodEnv, Atom object, int index, Atom valueToSet) {}
void MethodEnvSetPropertyLateUnsignedInteger(MethodEnv* methodEnv, Atom object, unsigned int index, Atom valueToSet) {}

Atom callprop_late(MethodEnv* caller_env, Atom base, const Multiname* name, int argc, Atom* args) {}

Atom ToplevelCallProperty(Toplevel* toplevel, Atom receiverAtom, const Multiname* propertyName, int argCount, Atom* arguments, VTable* vtable) {}
ArrayObject* MethodEnvCreateRest(MethodEnv* env, Atom* arguments, int argCount) {}

Atom AvmCoreIstypeAtom(Atom valueToCheck, Atom typeToCompare, Toplevel* toplevel) {}
void MethodEnvInitProperty(MethodEnv* env, Atom receiverObject, const Multiname* propertyName, Atom valueToInit, VTable* vtable) {}
void InitMultinameLate(AvmCore* core, Multiname& name, Atom index) {}

ScriptObject* MethodEnvNewObject(MethodEnv* env, Atom* objectPropertyPairs, int numberOfProperties) {}
Atom MethodEnvFindProperty(MethodEnv* env, const Multiname* multiname, bool isStrict) {}

void MethodEnvNullCheck(MethodEnv* env, Atom object) {} 
Atom MethodEnvCallSuper(MethodEnv* env, const Multiname* multiname, int argCount, Atom* arguments) {} 

ClassClosure* MethodEnvNewFunction(MethodEnv* env, AvmCore* core, int scopeDepth, ScriptObject** scopeStack, Atom methodIndex) { }

void JitAssertFalse() { }
int testThisCall(MethodInfo*, MethodEnv* env, int argc) { }

ArrayObject* AvmCoreNewArray(Toplevel* toplevel, int elementCount, Atom* arrayElements) {}
Atom AvmCoreNewArrayAtom(ArrayObject* arrayObject) {}

ScriptObject* MethodEnvNewActivation(MethodEnv* env) {}
ScriptObject* AtomToScriptObject(Atom receiverObject) {}

Atom ScriptObjectToAtom(ScriptObject*) {}
Atom ScriptObjectGetSlotAtom(ScriptObject* object, int index) {}
void ScriptObjectSetSlotAtom(ScriptObject* object, int slotIndex, Atom valueToSet) {}

MethodEnv* MethodEnvSuperInit(MethodEnv* env) {}
void MethodEnvCoerceEnter(MethodEnv* env, int argCount, Atom* arguments) {}

Traits* ToplevelToClassITraits(Toplevel*, Atom atom) {}
Atom AvmCoreIstypeAtom(Atom atom, Traits* itraits) {}

ScriptObject* VTableCreateInstance(ClassClosure* cls, VTable* ivtable) {}

/***
 * all the vector objects, required for the benchmarks
 */

// int vector
int IntVectorObjectGetNativeInt(TypedVectorObject* vector, int index) {}
int IntVectorObjectGetNativeUInt(TypedVectorObject* vector, unsigned int index) {}
int IntVectorObjectSetNativeInt(TypedVectorObject* vector, int index, int valueToSet) {}
int IntVectorObjectSetNativeUInt(TypedVectorObject* vector, unsigned int index, unsigned int valueToSet) {}

// unsigned vector
int UIntVectorObjectGetNativeInt(TypedVectorObject* vector, int index) {}
int UIntVectorObjectGetNativeUInt(TypedVectorObject* vector, unsigned int index) {}
void UIntVectorObjectSetNativeInt(TypedVectorObject* vector, int index, unsigned int valueToSet) {}
void UIntVectorObjectSetNativeUInt(TypedVectorObject* vector, unsigned int index, unsigned int valueToSet) {}

// double vector
double DoubleVectorObjectGetNativeInt(TypedVectorObject* vector, int index) {}
double DoubleVectorObjectGetNativeUInt(TypedVectorObject* vector, unsigned int index) {}
void DoubleVectorObjectSetNativeInt(TypedVectorObject* vector, int index, double valueToSet) {}
void DoubleVectorObjectSetNativeUInt(TypedVectorObject* vector, unsigned int index, double valueToSet) {}

// Object vector
Atom ObjectVectorObjectGetIntProperty(TypedVectorObject* vector, int index) {}
void ObjectVectorObjectSetIntProperty(TypedVectorObject* vector, int index, Atom valueToSet) {}

void coerceobj_atom(MethodEnv *env, Atom atom, Traits* traits) {}

// Debug helpers
void MethodStart() {}
void MethodEnd() {}