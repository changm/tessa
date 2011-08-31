
#include "LlvmCodeGenerator.h"

// Read the header file why we need these
#include "VMCPPWrapperDefinitions.h"

using namespace LlvmCodeGenerator;

/***
 * C++ Wrapper Definitions
 */

bool isInteger(Atom value) {
	return (((value) & 7) == kIntptrType);
}

uint32_t toUint32(Atom value) {
	return uint32_t(atomGetIntptr(value));
}

/*** 
 * These other methods are just too complicated to replicate in the LLVM ir for now
 */

Atom ToplevelGetProperty(Toplevel* toplevel, Atom receiverObject, Atom key, const Multiname* propertyName) {
	AvmAssert(propertyName->isRuntime());
	AvmAssert(!(!propertyName->isRtns() && isInteger(key) && atomCanBeUint32(key) && AvmCore::isObject(receiverObject))); 
	AvmAssert(!(propertyName->isRtns() && !AvmCore::isDictionaryLookup(receiverObject, receiverObject)));
	return AvmCore::atomToScriptObject(receiverObject)->getAtomProperty(key);
	/*
	* This is the long route
	if (!propertyName->isRtns() && isInteger(key) && atomCanBeUint32(key) && AvmCore::isObject(receiverObject)) {
		uint32_t uintValue = toUint32(key);
		return AvmCore::atomToScriptObject(receiverObject)->getUintProperty(uintValue);
	} else if (propertyName->isRtns() && !AvmCore::isDictionaryLookup(receiverObject, receiverObject)) {
		AvmAssert(false);
	} else {
		return AvmCore::atomToScriptObject(receiverObject)->getAtomProperty(key);
	}
	*/
}

void ToplevelSetProperty(Toplevel* toplevel, Atom receiverObject, Atom valueToSet, Atom key, const Multiname* propertyName) {
	AvmAssert(propertyName->isRuntime());
	AvmAssert(!(!propertyName->isRtns() && isInteger(key) && atomCanBeUint32(key) && AvmCore::isObject(receiverObject))); 
	AvmAssert(!(propertyName->isRtns() && !AvmCore::isDictionaryLookup(receiverObject, key)));
	AvmCore::atomToScriptObject(receiverObject)->setAtomProperty(key, valueToSet);
	/*
	* Still need this just in case
	if (!propertyName->isRtns() && isInteger(key) && atomCanBeUint32(key) && AvmCore::isObject(receiverObject)) {
		AvmCore::atomToScriptObject(receiverObject)->setUintProperty(toUint32(key), valueToSet);
	} else if (propertyName->isRtns() && !AvmCore::isDictionaryLookup(receiverObject, key)) {
		AvmAssert(false);
	} else {
		AvmCore::atomToScriptObject(receiverObject)->setAtomProperty(key, valueToSet);
	}
	*/
}

ScriptObject* MethodEnvNewObject(MethodEnv* env, Atom* objectPropertyPairs, int numberOfProperties) {
	// op_newobject expects argv to be at the end of the object property pairs
	Atom* endOfObjectPropertyPairs = objectPropertyPairs + (numberOfProperties * 2) - 1;
	return env->op_newobject(endOfObjectPropertyPairs, numberOfProperties);
}

Atom MethodEnvOp_applytype(MethodEnv* env, Atom factory, int argc, Atom* args) {
	// Have to do arguments + 1 because arguments[0] is the function object, and apply_type Atom* arg can't have that
	AvmAssert(args[0] == factory);
	return op_applytype(env, factory, argc, (args + 1));
}

ClassClosure* MethodEnvNewFunction(MethodEnv* env, AvmCore* core, int scopeDepth, ScriptObject** scopeStack, Atom methodIndex) {
	uint32_t methodIndexInteger = core->integer_u(methodIndex);
	MethodInfo* methodInfo = env->method->pool()->getMethodInfo(methodIndexInteger); 

	Atom* scopeBase = new Atom[scopeDepth];
	VMPI_memset(scopeBase, 0, sizeof(Atom) * scopeDepth);
	for (int i = 0; i < scopeDepth; i++) {
		scopeBase[i] = scopeStack[i]->atom();
	}

	return env->newfunction(methodInfo, env->scope(), scopeBase);
}

/***
 * Todo: Work in scope base and with statements
 */
Atom MethodEnvFindProperty(MethodEnv* env, const Multiname* multiname, bool isStrict) {
	ScopeChain* scopeChain = env->scope();
	Atom* withBase = NULL;
	//Atom* scopeBase = new Atom[1];
	Atom scopeBase[1];
	int scopeDepth = 0;
	scopeBase[0] = NULL;
	return env->findproperty(scopeChain, scopeBase, scopeDepth, multiname, isStrict, withBase);
}

/***
 * Some methods have to be copied from core/jit-calls.h to here because we can't include jit-calls.h
 * otherwise the build gets ugly
 */
// Copied from jit-calls.h
void jit_calls_initMultinameLate(AvmCore* core, Multiname& name, Atom index) {
	if (AvmCore::isObject(index)) {
        ScriptObject* i = AvmCore::atomToScriptObject(index);
		if (i->traits() == core->traits.qName_itraits) {
            QNameObject* qname = (QNameObject*) i;
            bool attr = name.isAttr();
            qname->getMultiname(name);
            name.setAttr(attr);
            return;
        }
    }

    name.setName(core->intern(index));
}

/***
 * Copied from jit-calls.h
 */
void jit_calls_setprop_index(MethodEnv* caller_env, Atom obj, const Multiname* name, Atom value, Atom index)
{
    if (isObjectPtr(obj)) {
        if (atomIsIntptr(index) && atomCanBeUint32(index)) {
            // todo: obj is probably a dense array or vector
            AvmCore::atomToScriptObject(obj)->setUintProperty(uint32_t(atomGetIntptr(index)), value);
            return;
        }
    }

    Multiname tempname = *name;
    caller_env->setpropertyHelper(obj, &tempname, value, toVTable(caller_env->toplevel(), obj), index);
}


// implements OP_getproperty with unknown base object or index type, but a multiname
// that includes public and therefore exposes dynamic properties
// copied from jit-calls.h
Atom jit_calls_GetPropertyIndex(MethodEnv* caller_env, Atom obj, const Multiname *name, Atom index)
{
    if (atomIsIntptr(index) && atomCanBeUint32(index)) {
        if (isObjectPtr(obj)) {
            return AvmCore::atomToScriptObject(obj)->getUintProperty(uint32_t(atomGetIntptr(index)));
        }
    }
    Multiname tempname = *name;
    VTable* vtable = toVTable(caller_env->toplevel(), obj);
    return caller_env->getpropertyHelper(obj, &tempname, vtable, index);
}

Toplevel* getToplevel(MethodEnv* env) {
	return env->toplevel();
}

void JitAssertFalse() { 
	assert(false);
}

void MethodStartSymbol() {

}

void MethodEndSymbol() {

}

/***
 * Crazy Call Info build jutsu
 */


/****
 * Copied from CodegenLIR
 */
#ifdef AVMPLUS_ARM
#ifdef _MSC_VER
#define RETURN_METHOD_PTR(_class, _method) \
return *((int*)&_method);
#else
#define RETURN_METHOD_PTR(_class, _method) \
union { \
    int (_class::*bar)(); \
    int foo[2]; \
}; \
bar = _method; \
return foo[0];
#endif

#elif defined __GNUC__
#define RETURN_METHOD_PTR(_class, _method) \
union { \
    int (_class::*bar)(); \
    intptr_t foo; \
}; \
bar = _method; \
return foo;
#else
#define RETURN_METHOD_PTR(_class, _method) \
return *((intptr_t*)&_method);
#endif

/***
 * Used to get the addresses of C++ class methods 
 */
	#define COREADDR(f) coreAddr((int (AvmCore::*)())(&f))
    #define GCADDR(f) gcAddr((int (MMgc::GC::*)())(&f))
    #define ENVADDR(f) envAddr((int (MethodEnv::*)())(&f))
	#define TOPLEVELADDR(f) toplevelAddr((int (Toplevel::*)())(&f))
	#define METHODINFOADDR(f) methodInfoAddr((int (MethodInfo::*)())(&f))
    #define ARRAYADDR(f) arrayAddr((int (ArrayObject::*)())(&f))
    #define VECTORINTADDR(f) vectorIntAddr((int (IntVectorObject::*)())(&f))
    #define VECTORUINTADDR(f) vectorUIntAddr((int (UIntVectorObject::*)())(&f))
    #define VECTORDOUBLEADDR(f) vectorDoubleAddr((int (DoubleVectorObject::*)())(&f))
    #define VECTOROBJADDR(f) vectorObjAddr((int (ObjectVectorObject::*)())(&f))
    #define EFADDR(f)   efAddr((int (ExceptionFrame::*)())(&f))
    #define DEBUGGERADDR(f)   debuggerAddr((int (Debugger::*)())(&f))
	#define SCRIPTADDR(f) scriptObjectAddr((int (ScriptObject::*)())(&f))
	#define SCOPECHAINADDR(f) scopeChainAddr((int (ScopeChain::*)())(&f))

    #define FUNCADDR(addr) (uintptr)addr

    intptr_t coreAddr( int (AvmCore::*f)() )
    {
        RETURN_METHOD_PTR(AvmCore, f);
    }

    intptr_t  gcAddr( int (MMgc::GC::*f)() )
    {
        RETURN_METHOD_PTR(MMgc::GC, f);
    }

    intptr_t  envAddr( int (MethodEnv::*f)() )
    {
        RETURN_METHOD_PTR(MethodEnv, f);
    }

	intptr_t  toplevelAddr( int (Toplevel::*f)() )
    {
        RETURN_METHOD_PTR(Toplevel, f);
    }
	
	intptr_t  scriptObjectAddr( int (ScriptObject::*f)() )
    {
        RETURN_METHOD_PTR(ScriptObject, f);
    }

	intptr_t  scopeChainAddr( int (ScopeChain::*f)() )
    {
        RETURN_METHOD_PTR(ScopeChain, f);
    }

	intptr_t  methodInfoAddr( int (MethodInfo::*f)() )
    {
	    RETURN_METHOD_PTR(MethodInfo, f);
    }

    intptr_t  arrayAddr(int (ArrayObject::*f)())
    {
        RETURN_METHOD_PTR(ArrayObject, f);
    }

    intptr_t vectorIntAddr(int (IntVectorObject::*f)())
    {
        RETURN_METHOD_PTR(IntVectorObject, f);
    }

    intptr_t vectorUIntAddr(int (UIntVectorObject::*f)())
    {
        RETURN_METHOD_PTR(UIntVectorObject, f);
    }

    intptr_t vectorDoubleAddr(int (DoubleVectorObject::*f)())
    {
        RETURN_METHOD_PTR(DoubleVectorObject, f);
    }

    intptr_t vectorObjAddr(int (ObjectVectorObject::*f)())
    {
        RETURN_METHOD_PTR(ObjectVectorObject, f);
    }
    intptr_t efAddr( int (ExceptionFrame::*f)() )
    {
        RETURN_METHOD_PTR(ExceptionFrame, f);
    }
	/*** End copy from CodegenLir ***/


namespace LlvmCodeGenerator {

	/*** 
	 * Ugly ugly hack to get the address of an overloaded method. Anyone know something else?
	 */
	Atom (MethodEnv::*coerceEnterPointer)(int, Atom*) = &MethodEnv::coerceEnter;
	intptr_t coerceEnterIntPtr = *((intptr_t*)&coerceEnterPointer);

/***
 * Call Info definitions. Expand each entry in LlvmJitcalls.h to:
 * extern const LlvmCallInfo llvmCallInfo_prettyName = { prettyName, mangledName, address };
 */

#define PURE_FUNCTION_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName = { #prettyName, #mangledName, address, ABI_CDECL, true};

#define FUNCTION_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName = { #prettyName, #mangledName, address, ABI_CDECL, false};

#define PURE_METHOD_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName = { #prettyName, #mangledName, address, ABI_THISCALL, true};

#define METHOD_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName = { #prettyName, #mangledName, address, ABI_THISCALL, false};

#define FASTCALL_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName = { #prettyName, #mangledName, address, ABI_FASTCALL, false};

#include "LlvmJitCalls.h"

#undef FUNCTION_LLVM_CALLINFO
#undef METHOD_LLVM_CALLINFO
#undef FASTCALL_LLVM_CALLINFO
#undef PURE_FUNCTION_LLVM_CALLINFO
#undef PURE_METHOD_LLVM_CALLINFO

/***
  * The Call Info array. THIS HAS TO BE AFTER THE INCLUDE OF LLVM JIT CALLS.
  * Just fetch the call infos to create the array. 
  * Expand into:
  * extern const LlvmCallInfo* llvmCAllInfoArray[] = {
		&llvmCallInfoName,
  }
  *	I apologize for the Macro hell. If you know a better way, please take the liberty to do it.
  * - Mason
  */

#define PURE_FUNCTION_LLVM_CALLINFO(prettyName, mangledName, address) \
	GET_LLVM_CALLINFO(prettyName),

#define METHOD_LLVM_CALLINFO(prettyName, mangledName, address) \
	GET_LLVM_CALLINFO(prettyName),

#define PURE_METHOD_LLVM_CALLINFO(prettyName, mangledName, address) \
	GET_LLVM_CALLINFO(prettyName),

#define FUNCTION_LLVM_CALLINFO(prettyName, mangledName, address) \
	GET_LLVM_CALLINFO(prettyName),

#define FASTCALL_LLVM_CALLINFO(prettyName, mangledName, address) \
	GET_LLVM_CALLINFO(prettyName),

extern const LlvmCallInfo* llvmCallInfoArray[] = {
	#include "LlvmJitCalls.h"
};

#undef METHOD_LLVM_CALLINFO
#undef FUNCTION_LLVM_CALLINFO
#undef FASTCALL_LLVM_CALLINFO
#undef PURE_FUNCTION_LLVM_CALLINFO
#undef PURE_METHOD_LLVM_CALLINFO

extern const int SizeOfLlvmCallInfoArray = sizeof(llvmCallInfoArray);

}	// End namespace