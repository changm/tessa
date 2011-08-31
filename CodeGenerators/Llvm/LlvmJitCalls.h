/***
 * Here we more or less mimic jit-calls.h. This is also one giant build hack.
 * The main reason we need this is that LLVM's JIT has to call an llvm::function*, you can't just call an address.
 *
 * Each DEFINE_CALL_INFO takes in a "Pretty Name" - Which is what you want to call it. Keep it clean so its easy to recognize what its calling. Ala AvmCoreIntToAtom is AvmCore::IntToAtom
 * Mangled name - What llvm compiles the method declaration to. Ugly, but we have to have it
 * address - the address to the actual method
 * KEEP THIS FILE CLEAN AND READABLE - KEEP THIS FILE CLEAN AND READABLE - KEEP THIS FILE CLEAN AND READABLE
 *
 * The build process:
 * This file is included in two places.
 * 1) VMCPPWrapperDefinitions.h - This simply DECLARES that a call info exists as an extern.
 * 2) VMCPPWrapperDefinitions.cpp - Defines each call info
 *
 * Every file that wants to USE a call info just includes VMCPPWrapperDefinitions.h. Since it is declared an extern, it shouldn't use more memory
 * This is a giant hack thats needed so that we can use a call info in more than just one file
 *
 * USE PURE_FUNCTION_LLVM_CALLINFO for CDECL PURE methods, that is methods that do not cause side affects
 * USE FUNCTION_LLVM_CALLINFO for CDECL ABI methods
 * USE METHOD_LLVM_CALLINFO for THISCALL methods / C++ instance members
 * USE FASTCALL_LLVM_CALLINFO for FASTCALL methods
 */

/****
 * PURE CDECL methods, or don't have side effects
 */
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreIsTypeAtom, _Z17AvmCoreIstypeAtomiPN7avmplus6TraitsE, FUNCADDR(AvmCore::istypeAtom))
PURE_FUNCTION_LLVM_CALLINFO(toVtable, _Z8toVTablePN7avmplus8ToplevelEi, (uintptr)(toVTable_Toplevel)&toVTable<Toplevel*>) 
PURE_FUNCTION_LLVM_CALLINFO(MathUtilsMod, _Z12MathUtilsModdd, (uintptr)(&MathUtils::mod))
PURE_FUNCTION_LLVM_CALLINFO(finddef_cache, _Z13finddef_cachePN7avmplus9MethodEnvEPKNS_9MultinameEj, FUNCADDR(&finddef_cache))
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreBooleanAtom, _Z14AvmCorebooleani, (uintptr)(&AvmCore::booleanAtom))
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreBoolean, _Z14AvmCorebooleani, (uintptr)(&AvmCore::boolean))
PURE_FUNCTION_LLVM_CALLINFO(AtomToScriptObject, _Z18AtomToScriptObjecti, (uintptr)(&AvmCore::atomToScriptObject))

// No idea why these are different
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreInteger, _Z7integeri, (uintptr)(&AvmCore::integer)) 
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreAtomToSignedInt, _Z9integer_ii, (uintptr)(&AvmCore::integer_i))
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreAtomToUnsignedInt, _Z9integer_ui, (uintptr)(&AvmCore::integer_u))

// No idea why these are different
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreNumber, _Z6numberi, (uintptr)(&AvmCore::number))
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreAtomToNumberFast, _Z8number_di, (uintptr)(&AvmCore::number_d))
PURE_FUNCTION_LLVM_CALLINFO(AvmCoreToUint32, _Z8toUInt32i, (uintptr)(&AvmCore::toUInt32))

/***
 * CDECL methods
 */
FUNCTION_LLVM_CALLINFO(JitAssertFalse, _Z14JitAssertFalsev, (uintptr)(&JitAssertFalse))
FUNCTION_LLVM_CALLINFO(TestThisCall, _Z12testThisCallPN7avmplus10MethodInfoEPNS_9MethodEnvEi, METHODINFOADDR(MethodInfoProcHolder::testThisCall))
FUNCTION_LLVM_CALLINFO(op_add, _Z6op_addPN7avmplus7AvmCoreEii, (uintptr)(&op_add))
FUNCTION_LLVM_CALLINFO(op_call_toplevel, _Z16op_call_toplevelPN7avmplus8ToplevelEiiPi, (uintptr)(op_call_Toplevel)&op_call<Toplevel*>) 
FUNCTION_LLVM_CALLINFO(op_call_methodEnv, _Z17op_call_methodEnvPN7avmplus9MethodEnvEiiPi, (uintptr)(op_call_instr)&op_call<MethodEnv*>) 
FUNCTION_LLVM_CALLINFO(MethodEnvFindProperty, _Z21MethodEnvFindPropertyPN7avmplus9MethodEnvEPKNS_9MultinameEb, (uintptr)(&MethodEnvFindProperty))
FUNCTION_LLVM_CALLINFO(MethodEnvNewFunction, _Z20MethodEnvNewFunctionPN7avmplus9MethodEnvEPNS_7AvmCoreEiPPNS_12ScriptObjectEi, (uintptr)(&MethodEnvNewFunction))
FUNCTION_LLVM_CALLINFO(op_construct, _Z12op_constructPN7avmplus8ToplevelEiiPi, (uintptr)((op_construct_Toplevel)(&op_construct<Toplevel*>)))
FUNCTION_LLVM_CALLINFO(constructprop, _Z13constructpropPN7avmplus9MethodEnvEPKNS_9MultinameEiPi, (uintptr)((constructprop_MethodEnv)(&constructprop<MethodEnv*>)))
FUNCTION_LLVM_CALLINFO(ToplevelGetProperty, _Z19ToplevelGetPropertyPN7avmplus8ToplevelEiiPKNS_9MultinameE, (uintptr)(&ToplevelGetProperty))
FUNCTION_LLVM_CALLINFO(GetPropertyIndex, _Z26jit_calls_GetPropertyIndexPN7avmplus9MethodEnvEiPKNS_9MultinameEi, (uintptr)(&jit_calls_GetPropertyIndex))
FUNCTION_LLVM_CALLINFO(ToplevelSetProperty, _Z19ToplevelSetPropertyPN7avmplus8ToplevelEiiiPKNS_9MultinameE, (uintptr)(&ToplevelSetProperty))
FUNCTION_LLVM_CALLINFO(SetPropertyIndex, _Z16SetPropertyIndexPN7avmplus9MethodEnvEiPKNS_9MultinameEii, (uintptr)(&jit_calls_setprop_index))
FUNCTION_LLVM_CALLINFO(newarray, _Z15AvmCoreNewArrayPN7avmplus8ToplevelEiPi, (uintptr)((op_newarray_Toplevel)(&avmplus::newarray)))
FUNCTION_LLVM_CALLINFO(InitMultinameLate, _Z17InitMultinameLatePN7avmplus7AvmCoreERNS_9MultinameEi, (uintptr)(&jit_calls_initMultinameLate))
FUNCTION_LLVM_CALLINFO(MethodEnvNewObject, _Z18MethodEnvNewObjectPN7avmplus9MethodEnvEPii, (uintptr)(&MethodEnvNewObject))
FUNCTION_LLVM_CALLINFO(MethodEnvOp_applytype, _Z21MethodEnvOp_applytypePN7avmplus9MethodEnvEiiPi, (uintptr)(&MethodEnvOp_applytype))
FUNCTION_LLVM_CALLINFO(AvmCoreAtomWriteBarrier, _Z23AvmCoreAtomWriteBarrierPN4MMgc2GCEPiS2_i, FUNCADDR(AvmCore::atomWriteBarrier))
FUNCTION_LLVM_CALLINFO(NumberToInteger, _Z9integer_dd, FUNCADDR(AvmCore::integer_d_sse2))
FUNCTION_LLVM_CALLINFO(CoerceObject_Atom, _Z14coerceobj_atomPN7avmplus9MethodEnvEiPNS_6TraitsE, FUNCADDR(&coerceobj_atom))

/***
 * Indirect calls
 */ 
FUNCTION_LLVM_CALLINFO(VTableCreateInstance, _Z20VTableCreateInstancePN7avmplus12ClassClosureEPNS_6VTableE, FUNCADDR(0))
FUNCTION_LLVM_CALLINFO(InvokeReturnInt, _Z15InvokeReturnIntPN7avmplus9MethodEnvEiPi, FUNCADDR(0))
FUNCTION_LLVM_CALLINFO(InvokeReturnPointer, _Z19InvokeReturnPointerPN7avmplus9MethodEnvEiPi, FUNCADDR(0))
FUNCTION_LLVM_CALLINFO(InvokeReturnNumber, _Z18InvokeReturnNumberPN7avmplus9MethodEnvEiPi, FUNCADDR(0))

/*** 
 * Cache handlers
 */
FUNCTION_LLVM_CALLINFO(CallCacheHandler, _Z16CallCacheHandlerRN7avmplus9CallCacheEiiPiPNS_9MethodEnvE, FUNCADDR(0))
FUNCTION_LLVM_CALLINFO(GetCacheHandler, _Z15GetCacheHandlerRN7avmplus8GetCacheEPNS_9MethodEnvEi, FUNCADDR(0))
FUNCTION_LLVM_CALLINFO(SetCacheHandler, _Z15SetCacheHandlerRN7avmplus8SetCacheEiiPNS_9MethodEnvE, FUNCADDR(0))

/*** 
 * Indirect Calls for ActionScript interface methods
 */ 
FUNCTION_LLVM_CALLINFO(InterfaceInvokeReturnInt, _Z24InterfaceInvokeReturnIntPN7avmplus9MethodEnvEiPiS2_, FUNCADDR(0))
FUNCTION_LLVM_CALLINFO(InterfaceInvokeReturnNumber, _Z27InterfaceInvokeReturnNumberPN7avmplus9MethodEnvEiPiS2_, FUNCADDR(0))
FUNCTION_LLVM_CALLINFO(InterfaceInvokeReturnPointer, _Z28InterfaceInvokeReturnPointerPN7avmplus9MethodEnvEiPiS2_, FUNCADDR(0))

/***
 * PURE THISCALL methods
 */
PURE_METHOD_LLVM_CALLINFO(AvmCoreDoubleToAtom, _Z12doubleToAtomPN7avmplus7AvmCoreEd, COREADDR(AvmCore::doubleToAtom_sse2))
PURE_METHOD_LLVM_CALLINFO(AvmCoreIntToAtom, _Z9intToAtomPN7avmplus7AvmCoreEi, COREADDR(AvmCore::intToAtom))
PURE_METHOD_LLVM_CALLINFO(AvmCoreUintToAtom, _Z10uintToAtomPN7avmplus7AvmCoreEj, COREADDR(AvmCore::uintToAtom))
PURE_METHOD_LLVM_CALLINFO(AvmCoreString, _Z13AvmCoreStringPN7avmplus7AvmCoreEi, COREADDR(AvmCore::string))
PURE_METHOD_LLVM_CALLINFO(AvmCoreCoerce_s, _Z15AvmCoreCoerce_sPN7avmplus7AvmCoreEi, COREADDR(AvmCore::coerce_s))
PURE_METHOD_LLVM_CALLINFO(AvmCoreCompare, _Z7comparePN7avmplus7AvmCoreEii, COREADDR(AvmCore::compare))
PURE_METHOD_LLVM_CALLINFO(AvmCoreStrictEquals, _Z12strictEqualsPN7avmplus7AvmCoreEii, COREADDR(AvmCore::stricteq))
PURE_METHOD_LLVM_CALLINFO(ScriptObjectToAtom, _Z18ScriptObjectToAtomPN7avmplus12ScriptObjectE, SCRIPTADDR(ScriptObject::atom))

/*** 
 * THISCALL methods
 */
METHOD_LLVM_CALLINFO(MethodEnvCreateRest, _Z19MethodEnvCreateRestPN7avmplus9MethodEnvEPii, ENVADDR(MethodEnv::createRest))
METHOD_LLVM_CALLINFO(MethodEnvSuperInit, _Z18MethodEnvSuperInitPN7avmplus9MethodEnvE, ENVADDR(MethodEnv::super_init))
METHOD_LLVM_CALLINFO(MethodEnvCoerceEnter, _Z20MethodEnvCoerceEnterPN7avmplus9MethodEnvEiPi, intptr_t(coerceEnterIntPtr))
METHOD_LLVM_CALLINFO(MethodEnvInitProperty, _Z21MethodEnvInitPropertyPN7avmplus9MethodEnvEiPKNS_9MultinameEiPNS_6VTableE, ENVADDR(MethodEnv::initproperty))
METHOD_LLVM_CALLINFO(MethodEnvCallSuper, _Z18MethodEnvCallSuperPN7avmplus9MethodEnvEPKNS_9MultinameEiPi, ENVADDR(MethodEnv::callsuper))
METHOD_LLVM_CALLINFO(MethodEnvNullCheck, _Z18MethodEnvNullCheckPN7avmplus9MethodEnvEi, ENVADDR(MethodEnv::nullcheck))
METHOD_LLVM_CALLINFO(ToplevelGetPropertyNonRuntime, _Z29TopLevelGetPropertyNonRuntimePN7avmplus8ToplevelEiPKNS_9MultinameEPNS_6VTableE, TOPLEVELADDR(Toplevel::getproperty))
METHOD_LLVM_CALLINFO(ToplevelSetPropertyNonRuntime, _Z29ToplevelSetPropertyNonRuntimePN7avmplus8ToplevelEiPKNS_9MultinameEiPNS_6VTableE, TOPLEVELADDR(Toplevel::setproperty))
METHOD_LLVM_CALLINFO(arrayObjectAtom, _Z19AvmCoreNewArrayAtomPN7avmplus11ArrayObjectE, ARRAYADDR(ArrayObject::atom)) 
METHOD_LLVM_CALLINFO(ToplevelCallProperty, _Z20ToplevelCallPropertyPN7avmplus8ToplevelEiPKNS_9MultinameEiPiPNS_6VTableE, TOPLEVELADDR(Toplevel::callproperty))
METHOD_LLVM_CALLINFO(ToplevelToClassITraits, _Z22ToplevelToClassITraitsPN7avmplus8ToplevelEi, TOPLEVELADDR(Toplevel::toClassITraits))
METHOD_LLVM_CALLINFO(AvmCoreEquals, _Z6equalsPN7avmplus7AvmCoreEii, COREADDR(AvmCore::equals))
METHOD_LLVM_CALLINFO(ScriptObjectGetSlot, _Z23ScriptObjectGetSlotAtomPN7avmplus12ScriptObjectEi, SCRIPTADDR(ScriptObject::getSlotAtom))
METHOD_LLVM_CALLINFO(ScriptObjectSetSlot, _Z23ScriptObjectSetSlotAtomPN7avmplus12ScriptObjectEii, SCRIPTADDR(ScriptObject::coerceAndSetSlotAtom))
METHOD_LLVM_CALLINFO(MethodEnvNewActivation, _Z22MethodEnvNewActivationPN7avmplus9MethodEnvE, ENVADDR(MethodEnv::newActivation))
METHOD_LLVM_CALLINFO(MethodInfoInvoke, _Z16MethodInfoInvokePN7avmplus10MethodInfoEPNS_9MethodEnvEiPi, METHODINFOADDR(MethodInfo::invoke))
METHOD_LLVM_CALLINFO(ArrayObjectSetUintProperty, _Z26ArrayObjectSetUIntPropertyPN7avmplus11ArrayObjectEji, ARRAYADDR(ArrayObject::_setUintProperty))
METHOD_LLVM_CALLINFO(ArrayObjectSetIntProperty, _Z25ArrayObjectSetIntPropertyPN7avmplus11ArrayObjectEii, ARRAYADDR(ArrayObject::_setIntProperty))
METHOD_LLVM_CALLINFO(ArrayObjectGetIntProperty, _Z25ArrayObjectGetIntPropertyPN7avmplus11ArrayObjectEi, ARRAYADDR(ArrayObject::_getIntProperty))
METHOD_LLVM_CALLINFO(ArrayObjectGetUintProperty, _Z26ArrayObjectGetUintPropertyPN7avmplus11ArrayObjectEj, ARRAYADDR(ArrayObject::_getUintProperty))
METHOD_LLVM_CALLINFO(MMgcPrivateWriteBarrierRC, _Z23GCprivateWriteBarrierRCPN4MMgc2GCEPiS2_S2_, GCADDR(MMgc::GC::privateWriteBarrierRC))
METHOD_LLVM_CALLINFO(GetToplevel, _Z11getToplevelPN7avmplus9MethodEnvE, ENVADDR(MethodEnv::toplevel))
METHOD_LLVM_CALLINFO(MethodEnvGetPropertyLateInteger, _Z31MethodEnvGetPropertyLateIntegerPN7avmplus9MethodEnvEii, ENVADDR(MethodEnv::getpropertylate_i))
METHOD_LLVM_CALLINFO(MethodEnvGetPropertyLateUnsignedInteger, _Z39MethodEnvGetPropertyLateUnsignedIntegerPN7avmplus9MethodEnvEij, ENVADDR(MethodEnv::getpropertylate_u))
METHOD_LLVM_CALLINFO(MethodEnvSetPropertyLateInteger, _Z31MethodEnvSetPropertyLateIntegerPN7avmplus9MethodEnvEiii, ENVADDR(MethodEnv::setpropertylate_i))
METHOD_LLVM_CALLINFO(MethodEnvSetPropertyLateUnsignedInteger, _Z39MethodEnvSetPropertyLateUnsignedIntegerPN7avmplus9MethodEnvEiji, ENVADDR(MethodEnv::setpropertylate_u))

/***
 * Vector addresses, needed for benchmarks
 */

 // Integer Vectors
METHOD_LLVM_CALLINFO(IntVectorObject_GetNativeIntProperty, _Z27IntVectorObjectGetNativeIntPN7avmplus17TypedVectorObjectEi, VECTORINTADDR(IntVectorObject::_getNativeIntProperty))
METHOD_LLVM_CALLINFO(IntVectorObject_GetNativeUIntProperty, _Z28IntVectorObjectGetNativeUIntPN7avmplus17TypedVectorObjectEj, VECTORINTADDR(IntVectorObject::_getNativeUintProperty))
METHOD_LLVM_CALLINFO(IntVectorObject_SetNativeIntProperty, _Z27IntVectorObjectSetNativeIntPN7avmplus17TypedVectorObjectEii, VECTORINTADDR(IntVectorObject::_setNativeIntProperty))
METHOD_LLVM_CALLINFO(IntVectorObject_SetNativeUIntProperty, _Z28IntVectorObjectSetNativeUIntPN7avmplus17TypedVectorObjectEjj, VECTORINTADDR(IntVectorObject::_setNativeUintProperty))

// Uint Vectors
METHOD_LLVM_CALLINFO(UIntVectorObject_GetNativeIntProperty, _Z28UIntVectorObjectGetNativeIntPN7avmplus17TypedVectorObjectEi, VECTORINTADDR(UIntVectorObject::_getNativeIntProperty))
METHOD_LLVM_CALLINFO(UIntVectorObject_GetNativeUIntProperty, _Z29UIntVectorObjectGetNativeUIntPN7avmplus17TypedVectorObjectEj, VECTORINTADDR(UIntVectorObject::_getNativeUintProperty))
METHOD_LLVM_CALLINFO(UIntVectorObject_SetNativeIntProperty, _Z28UIntVectorObjectSetNativeIntPN7avmplus17TypedVectorObjectEij, VECTORINTADDR(UIntVectorObject::_setNativeIntProperty))
METHOD_LLVM_CALLINFO(UIntVectorObject_SetNativeUIntProperty, _Z29UIntVectorObjectSetNativeUIntPN7avmplus17TypedVectorObjectEjj, VECTORINTADDR(UIntVectorObject::_setNativeUintProperty))

// Double vectors
METHOD_LLVM_CALLINFO(DoubleVectorObject_GetNativeIntProperty, _Z30DoubleVectorObjectGetNativeIntPN7avmplus17TypedVectorObjectEi, VECTORDOUBLEADDR(DoubleVectorObject::_getNativeIntProperty))
METHOD_LLVM_CALLINFO(DoubleVectorObject_GetNativeUIntProperty, _Z31DoubleVectorObjectGetNativeUIntPN7avmplus17TypedVectorObjectEj, VECTORDOUBLEADDR(DoubleVectorObject::_getNativeUintProperty))
METHOD_LLVM_CALLINFO(DoubleVectorObject_SetNativeIntProperty, _Z30DoubleVectorObjectSetNativeIntPN7avmplus17TypedVectorObjectEid, VECTORDOUBLEADDR(DoubleVectorObject::_setNativeIntProperty))
METHOD_LLVM_CALLINFO(DoubleVectorObject_SetNativeUIntProperty, _Z31DoubleVectorObjectSetNativeUIntPN7avmplus17TypedVectorObjectEjd, VECTORDOUBLEADDR(DoubleVectorObject::_setNativeUintProperty))

// Object vectors
METHOD_LLVM_CALLINFO(ObjectVectorObject_GetIntProperty, _Z32ObjectVectorObjectGetIntPropertyPN7avmplus17TypedVectorObjectEi,VECTOROBJADDR(ObjectVectorObject::_getIntProperty))
METHOD_LLVM_CALLINFO(ObjectVectorObject_SetIntProperty, _Z32ObjectVectorObjectSetIntPropertyPN7avmplus17TypedVectorObjectEii, VECTOROBJADDR(ObjectVectorObject::_setIntProperty))

/***
 * FASTCALL
 */
FASTCALL_LLVM_CALLINFO(StringConcatStrings, _Z19StringConcatStringsPN7avmplus6StringES1_, FUNCADDR(avmplus::String::concatStrings))

/***
 * Debug Methods
 */
FUNCTION_LLVM_CALLINFO(MethodStartSymbol, _Z11MethodStartv, FUNCADDR(MethodStartSymbol))
FUNCTION_LLVM_CALLINFO(MethodEndSymbol, _Z9MethodEndv, FUNCADDR(MethodEndSymbol))

