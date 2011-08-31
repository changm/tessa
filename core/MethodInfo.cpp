/* -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is [Open Source Virtual Machine.].
 *
 * The Initial Developer of the Original Code is
 * Adobe System Incorporated.
 * Portions created by the Initial Developer are Copyright (C) 2004-2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Adobe AS3 Team
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "avmplus.h"


#ifdef FEATURE_NANOJIT
#include "CodegenLIR.h"
#endif

//#define DOPROF
//#include "../vprof/vprof.h"

#include "AbcToTessaTranslator.h"
#include "TessaVisitors.h"
#include "TessaCodeGenerators.h"

namespace LirGenerators {
	class LlvmIRGenerator;
}

namespace avmplus
{
    using namespace MMgc;

    /**
     * MethodInfo wrapper around a system-generated init method.  Used when
     * there is no init method defined in the abc; this only occurs for activation
     * object traits and catch-block activation traits.
     */
    MethodInfo::MethodInfo(InitMethodStub, Traits* declTraits) :
        MethodInfoProcHolder(),
        _msref(declTraits->pool->core->GetGC()->emptyWeakRef),
        _declarer(declTraits),
        _activation(NULL),
        _pool(declTraits->pool),
        _abc_info_pos(NULL),
        _flags(RESOLVED),
        _method_id(-1)
    {
    }

#ifdef VMCFG_AOT
    MethodInfo::MethodInfo(InitMethodStub, Traits* declTraits, const NativeMethodInfo* native_info) :
        MethodInfoProcHolder(),
        _msref(declTraits->pool->core->GetGC()->emptyWeakRef),
        _declarer(declTraits),
        _activation(NULL),
        _pool(declTraits->pool),
        _abc_info_pos(NULL),
        _flags(RESOLVED),
        _method_id(-1)
    {
        AvmAssert(native_info != NULL);
        this->_native.thunker = native_info->thunker;
        this->_native.handler = native_info->handler;
        this->_flags |= compiledMethodFlags() | AOT_COMPILED;
    }
#endif

    /**
     * ordinary MethodInfo for abc or native method.
     */
    MethodInfo::MethodInfo(int32_t method_id,
                            PoolObject* pool,
                            const uint8_t* abc_info_pos,
                            uint8_t abcFlags,
                            const NativeMethodInfo* native_info) :
        MethodInfoProcHolder(),
        _msref(pool->core->GetGC()->emptyWeakRef),
        _declarer(NULL),
        _activation(NULL),
        _pool(pool),
        _abc_info_pos(abc_info_pos),
        _flags(abcFlags),
        _method_id(method_id)
    {
        AvmAssert(method_id >= 0);
#if !defined(MEMORY_INFO)
        MMGC_STATIC_ASSERT(offsetof(MethodInfo, _implGPR) == 0);
#endif

#ifdef VMCFG_VERIFYALL
        if (abcFlags & MethodInfo::NATIVE && !native_info && pool->core->config.verifyonly)
            _flags |= ABSTRACT_METHOD;
#endif

        if (native_info)
        {
            this->_native.thunker = native_info->thunker;
#ifdef VMCFG_INDIRECT_NATIVE_THUNKS
            this->_native.handler = native_info->handler;
#endif
            this->_flags |= NEEDS_CODECONTEXT | NEEDS_DXNS | ABSTRACT_METHOD;
        }
    }

    void MethodInfo::init_activationTraits(Traits* t)
    {
        AvmAssert(_activation.getTraits() == NULL);
        _activation.setTraits(pool()->core->GetGC(), this, t);
    }

    static bool hasTypedArgs(MethodSignaturep ms)
    {
        int32_t param_count = ms->param_count();
        for (int32_t i = 1; i <= param_count; i++) {
            if (ms->paramTraits(i) != NULL) {
                // at least one parameter is typed; need full coerceEnter
                return true;
            }
        }
        return false;
    }

    void MethodInfo::setInterpImpl()
    {
        MethodSignaturep ms = getMethodSignature();
        if (ms->returnTraitsBT() == BUILTIN_number)
            _implFPR = avmplus::interpFPR;
        else
            _implGPR = avmplus::interpGPR;
        AvmAssert(isInterpreted());
        _invoker = hasTypedArgs(ms) ? MethodEnv::coerceEnter_interp : MethodEnv::coerceEnter_interp_nocoerce;
    }

    void MethodInfo::setNativeImpl(GprMethodProc p)
    {
        _implGPR = p;
        _invoker = MethodEnv::coerceEnter_generic;
        AvmAssert(!isInterpreted());
    }

#ifdef INTEL_VTUNE
    void MethodInfo::setNativeImpl(GprMethodProc p, int32_t codeSize)
    {
		setNativeImpl(p);

		avmplus::Stringp methodNameStringP = this->getMethodName();
		StUTF8String cString(methodNameStringP);
		std::string string = cString.c_str();
		//printf("Name %s, iid %d, address %x, code size %d\n", string.data(), this->iid(), (void*)p, codeSize);
		VTune_NotifyLoad((char*)string.data(), this->iid(), (void*)p, codeSize);
    }
#endif

	void MethodInfo::setTessaInterpImpl() {
		_implGPR = (GprMethodProc) MethodEnv::coerceEnter_Tessa_interp;
		_invoker = MethodEnv::coerceEnter_Tessa_interp;
	}

	void MethodInfo::setTessaNativeImpl(ASFunction* function, void* nativeCodeLocation, bool setInvoker) {
		if (setInvoker) {
			_invoker = (AtomMethodProc) nativeCodeLocation;
		} else {
			_invoker = MethodEnv::coerceEnter_Tessa_native;
		}
		
		_implGPR = (GprMethodProc) nativeCodeLocation;
		function->_nativeCodeLocation = nativeCodeLocation;
	}

    void MethodInfo::setTessaFunction(ASFunction* function)
    {
		_tessaFunction = function;
    }

	ASFunction* MethodInfo::getTessaFunction() {
		return _tessaFunction;
	}

#ifdef DEBUGGER
    /*static*/ AvmBox MethodInfo::debugEnterExitWrapper32(AvmMethodEnv env, uint32_t argc, AvmBox* argv)
    {
        CallStackNode csn(CallStackNode::kEmpty);
        env->debugEnter(/*frametraits*/0, &csn, /*framep*/0, /*eip*/0);
        const AvmBox result = env->method->thunker()(env, argc, argv);
        env->debugExit(&csn);
        return result;
    }

    /*static*/ double MethodInfo::debugEnterExitWrapperN(AvmMethodEnv env, uint32_t argc, AvmBox* argv)
    {
        CallStackNode csn(CallStackNode::kEmpty);
        env->debugEnter(/*frametraits*/0, &csn, /*framep*/0, /*eip*/0);
        const double result = (reinterpret_cast<AvmThunkNativeThunkerN>(env->method->thunker()))(env, argc, argv);
        env->debugExit(&csn);
        return result;
    }
#endif

	void MethodInfo::printMethodName() {
#ifdef DEBUG
		int methodIid = this->iid();
		int methodId = this->method_id();
		char methodName[128];
		avmplus::Stringp methodNameStringP = this->getMethodName();
		StUTF8String cString(methodNameStringP);
		VMPI_snprintf(methodName, sizeof(methodName), "Method id %d ::%s", methodId, cString.c_str());
		printf("Verifying %s\n", methodName);
#endif
	}

    /*static*/ uintptr_t MethodInfo::verifyEnterGPR(MethodEnv* env, int32_t argc, uint32_t* ap)
    {
        MethodInfo* f = env->method;

        #ifdef VMCFG_VERIFYALL
        // never verify late in verifyall mode
        AvmAssert(!f->pool()->core->config.verifyall);
        #endif

		//int methodId = f->method_id();
		//printf("gpr enter: %d, method id %d\n", f->iid(), f->method_id());

		//f->printMethodName();
		if (env->core()->isUserScript()) {
	        f->verify(env->toplevel(), env->abcEnv());
		} else {
		    f->verify(env->toplevel(), env->abcEnv());
		}

		

#ifdef VMCFG_METHODENV_IMPL32
        // we got here by calling env->_implGPR, which now is pointing to verifyEnter(),
        // but next time we want to call the real code, not verifyEnter again.
        // All other MethodEnv's in their default state will call the target method
        // directly and never go through verifyEnter().
        env->_implGPR = f->implGPR();
#endif

        AvmAssert(f->implGPR() != MethodInfo::verifyEnterGPR);
        return f->implGPR()(env, argc, ap);
    }

    /*static*/ double MethodInfo::verifyEnterFPR(MethodEnv* env, int32_t argc, uint32_t* ap)
    {
        MethodInfo* f = env->method;

        #ifdef VMCFG_VERIFYALL
        // never verify late in verifyall mode
        AvmAssert(!f->pool()->core->config.verifyall);
        #endif

		//f->printMethodName();
        f->verify(env->toplevel(), env->abcEnv());

#ifdef VMCFG_METHODENV_IMPL32
        // we got here by calling env->_implGPR, which now is pointing to verifyEnter(),
        // but next time we want to call the real code, not verifyEnter again.
        // All other MethodEnv's in their default state will call the target method
        // directly and never go through verifyEnter().
        env->_implFPR = f->implFPR();
#endif

        AvmAssert(f->implFPR() != MethodInfo::verifyEnterFPR);
        return f->implFPR()(env, argc, ap);
    }

    // entry point when the first call to the method is late bound.
    /*static*/ Atom MethodInfo::verifyCoerceEnter(MethodEnv* env, int argc, Atom* args)
    {
        MethodInfo* f = env->method;

        #ifdef VMCFG_VERIFYALL
        // never verify late in verifyall mode
        AvmAssert(!f->pool()->core->config.verifyall);
        #endif

		AvmCore* core = env->core();
		//f->printMethodName();

		if (core->isUserScript()) {
	        f->verify(env->toplevel(), env->abcEnv());
		} else {
			f->verify(env->toplevel(), env->abcEnv());
		}

#ifdef VMCFG_METHODENV_IMPL32
        // we got here by calling env->_implGPR, which now is pointing to verifyEnter(),
        // but next time we want to call the real code, not verifyEnter again.
        // All other MethodEnv's in their default state will call the target method
        // directly and never go through verifyEnter().
        env->_implGPR = f->implGPR();
#endif
		if (core->isUserScript()) {
			AvmAssert(f->_invoker != MethodInfo::verifyCoerceEnter);
			//printf("coerce enter: %d, method id %d\n", f->iid(), f->method_id());
	        return f->invoke(env, argc, args);
		} else {
			AvmAssert(f->_invoker != MethodInfo::verifyCoerceEnter);
	        return f->invoke(env, argc, args);
		}
    }

	void MethodInfo::doTessaOptimizations(ASFunction* tessaMethod, AvmCore* core, Verifier* verifier) {
		if (core->config.tessaInline) {
			TessaInliner tessaInliner(core);
			tessaInliner.findInlineOpportunities(tessaMethod, verifier);
		}

		TypePropagator typePropagator(core, core->gc); 
		if (core->config.tessaTypePropagation) {
			typePropagator.propagateTypeInformation(tessaMethod);
		} else {
			typePropagator.resolvePhiTypesOnly(tessaMethod);
		}
		
		DeadCodeElimination deadCodeEliminator(core, core->gc);
		deadCodeEliminator.eliminateDeadCode(tessaMethod);
	}

	void MethodInfo::generateMachineCode(ASFunction* tessaMethod, AvmCore* core) {
		TessaCodeGenerators::TessaCodeGenerator codeGenerator;
		codeGenerator.compileCode(tessaMethod, this, core);
		this->setNativeImpl((GprMethodProc) codeGenerator.getNativeCodeLocation());
		this->_flags |= MethodInfo::JIT_IMPL;
        InvokerCompiler::initCompilerHook(this);
	}

	void MethodInfo::compileWithTessa(Verifier* verifier) {
		AvmCore* core = this->pool()->core;
		AbcToTessaTranslator *tessaCreator = new (core->gc) AbcToTessaTranslator(core, this, verifier);
		tessaCreator->recreateControlFlowGraph(this->abc_body_pos());
		verifier->verify(tessaCreator);
		core->tessaCount++;

		ASFunction* tessaMethod = tessaCreator->getMethod();
		doTessaOptimizations(tessaMethod, core, verifier);

		if (core->config.tessaInterp) {
			this->setTessaInterpImpl();
		} else {
			generateMachineCode(tessaMethod, core);
		}

		this->setTessaFunction(tessaMethod);
		core->alreadyDoingTessa = true;

		// Since "this" method info doesn't hold a reference to tessa creator, we have to manually delete it
		// Otherwise tessaCreator leaks
		delete tessaCreator;	
	}

    void MethodInfo::verify(Toplevel *toplevel, AbcEnv* abc_env)
    {
        AvmAssert(declaringTraits()->isResolved());
        resolveSignature(toplevel);
        AvmCore* core = this->pool()->core;
        if (isNative())
        {
            union {
                GprMethodProc implGPR;
                AvmThunkNativeThunker thunker;
                AvmThunkNativeThunkerN thunkerN;
            } u;
    #ifdef DEBUGGER
            if (core->debugger())
            {
                MethodSignaturep ms = getMethodSignature();
                if (ms->returnTraitsBT() == BUILTIN_number)
                    u.thunkerN = MethodInfo::debugEnterExitWrapperN;
                else
                    u.thunker = MethodInfo::debugEnterExitWrapper32;
            }
            else
    #endif
            {
                u.thunker = this->thunker();
            }
            this->setNativeImpl(u.implGPR);
#ifdef FEATURE_NANOJIT
            InvokerCompiler::initCompilerHook(this);
#endif
        }
        else
        {
            #ifdef DEBUGGER
            // just a fake CallStackNode here, so that if we throw a verify error,
            // we get a stack trace with the method being verified as its top entry.
            CallStackNode callStackNode(this);
            #endif /* DEBUGGER */

            PERFM_NTPROF("verify-ticks");

            CodeWriter* volatile coder = NULL;
            Verifier verifier(this, toplevel, abc_env);

            /*
                These "buf" declarations are an unfortunate but expedient hack:
                the existing CodeWriter classes (eg CodegenLIR, etc) have historically
                always been stack-allocated, thus they have no WB protection on member
                fields. Recent changes were made to allow for explicit cleanup() of them
                in the event of exception, but there was a latent bug waiting to happen:
                the actual automatic var was going out of scope, but still being referenced
                (via the 'coder' pointer) in the exception handler, but the area being
                pointed to may or may not still be valid. The ideal fix for this would
                simply be to heap-allocate the CodeWriters, but that would require going
                thru the existing code carefully and inserting WB where appropriate.
                Instead, this "expedient" hack uses placement new to ensure the memory
                stays valid into the exception handler.

                Note: the lack of a call to the dtor of the CodeWriter(s) is not an oversight.
                Calling cleanup() on coder is equivalent to running the dtor for all
                of the CodeWriters here.

                Note: allocated using arrays of intptr_t (rather than char) to ensure alignment is acceptable.
            */
        #define MAKE_BUF(name, type) \
            intptr_t name##_data[(sizeof(type)+sizeof(intptr_t)-1)/sizeof(intptr_t)];\
            type* const name = (type*) name##_data /* no semi */

        #if defined FEATURE_NANOJIT
            MAKE_BUF(jit_buf, CodegenLIR);
            #if defined VMCFG_WORDCODE
            MAKE_BUF(teeWriter_buf, TeeWriter);
            #endif
            #ifdef FEATURE_CFGWRITER
            MAKE_BUF(cfg_buf, CFGWriter);
            #endif
        #endif
            #if defined VMCFG_WORDCODE
            MAKE_BUF(translator_buf, WordcodeEmitter);
            #else
            MAKE_BUF(stubWriter_buf, CodeWriter);
            #endif

            TRY(core, kCatchAction_Rethrow)
            {
#if defined FEATURE_NANOJIT
                Runmode runmode = core->config.runmode;
				if ((core->config.useTessa) && (core->isUserScript()) && !isStaticInit()) {
					compileWithTessa(&verifier);
				}
				else if (runmode == RM_jit_all || (runmode == RM_mixed && !isStaticInit()) && (core->isUserScript()))
                {
					AvmAssert(!core->config.useTessa);
                    PERFM_NTPROF("verify & IR gen");

                    // note placement-new usage!
                    CodegenLIR* jit = new(jit_buf) CodegenLIR(this);
                    #if defined VMCFG_WORDCODE
                    WordcodeEmitter* translator = new(translator_buf) WordcodeEmitter(this, toplevel);
                    TeeWriter* teeWriter = new(teeWriter_buf) TeeWriter(translator, jit);
                    coder = teeWriter;
                    #else
                    coder = jit;
                    #endif

                #ifdef FEATURE_CFGWRITER
                    // analyze code and generate LIR
                    CFGWriter* cfg = new(cfg_buf) CFGWriter(this, coder);
                    coder = cfg;
                #endif

                    verifier.verify(coder);
                    PERFM_TPROF_END();

                    if (!jit->overflow) {
                        // assembler LIR into native code
                        jit->emitMD();
                    }

                    // the MD buffer can overflow so we need to re-iterate
                    // over the whole thing, since we aren't yet robust enough
                    // to just rebuild the MD code.

                    // mark it as interpreted and try to limp along
                    if (jit->overflow) {
                        if (core->JITMustSucceed()) {
                            Exception* e = new (core->GetGC()) Exception(core, core->newStringLatin1("JIT failed")->atom());
                            e->flags |= Exception::EXIT_EXCEPTION;
                            core->throwException(e);
                        }
                        setInterpImpl();
                    }
                    #ifdef VMCFG_WORDCODE
                    else {
                        if (_abc.word_code.translated_code)
                        {
                            set_word_code(core->GetGC(), NULL);
                        }
                        if (_abc.word_code.exceptions)
                        {
                            _abc.word_code.exceptions = NULL;
                        }
                    }
                    #endif
                }
                else
                {
                    // NOTE copied below
                    #if defined VMCFG_WORDCODE
                    WordcodeEmitter* translator = new(translator_buf) WordcodeEmitter(this, toplevel);
                    coder = translator;
                    #else
                    CodeWriter* stubWriter = new(stubWriter_buf) CodeWriter();
                    coder = stubWriter;
                    #endif
                    verifier.verify(coder); // pass2 dataflow
                    setInterpImpl();
                    // NOTE end copy
                }
#else // FEATURE_NANOJIT

                // NOTE copied from above
                #if defined VMCFG_WORDCODE
                WordcodeEmitter* translator = new(translator_buf) WordcodeEmitter(this, toplevel);
                coder = translator;
                #else
                CodeWriter* stubWriter = new(stubWriter_buf) CodeWriter();
                coder = stubWriter;
                #endif
                verifier.verify(coder); // pass2 dataflow
                setInterpImpl();
                // NOTE end copy

#endif // FEATURE_NANOJIT

                if (coder)
                {
                    coder->cleanup();
                    coder = NULL;
                }
            }
            CATCH (Exception *exception)
            {
                // clean up verifier
                verifier.~Verifier();

                // call cleanup all the way down the chain
                // each stage calls cleanup on the next one
                if (coder)
                    coder->cleanup();

                // re-throw exception
                core->throwException(exception);
            }
            END_CATCH
            END_TRY
            PERFM_TPROF_END();
        }
    }

    REALLY_INLINE double unpack_double(const void* src)
    {
    #if defined(AVMPLUS_64BIT) || defined(VMCFG_UNALIGNED_FP_ACCESS)
        return *(const double*)src;
    #else
        double_overlay u;
        u.bits32[0] = ((const uint32_t*)src)[0];
        u.bits32[1] = ((const uint32_t*)src)[1];
        return u.value;
    #endif
    }

#ifdef DEBUGGER
    // note that the "local" can be a true local (0..local_count-1)
    // or an entry on the scopechain (local_count...(local_count+max_scope)-1)
    // FIXME: this is exactly the same as makeatom() in jit-calls.h,
    // except for the decoding of unaligned doubles
    static Atom nativeLocalToAtom(AvmCore* core, void* src, SlotStorageType sst)
    {
        switch (sst)
        {
            case SST_double:
                return core->doubleToAtom(unpack_double(src));
            default:
                AvmAssert(false);
            case SST_int32:
                return core->intToAtom(*(const int32_t*)src);
            case SST_uint32:
                return core->uintToAtom(*(const uint32_t*)src);
            case SST_bool32:
                return *(const int32_t*)src ? trueAtom : falseAtom;
            case SST_atom:
                return *(const Atom*)src;
            case SST_string:
                return (*(const Stringp*)src)->atom();
            case SST_namespace:
                return (*(const Namespacep*)src)->atom();
            case SST_scriptobject:
                return (*(ScriptObject**)src)->atom();
        }
    }
#endif

    // this looks deceptively similar to nativeLocalToAtom, but is subtly different:
    // for locals, int/uint/bool are always stored as a 32-bit value in the 4 bytes
    // of the memory slot (regardless of wordsize and endianness);
    // for args, int/uint/bool are always expanded to full intptr size. In practice
    // this only makes a difference on big-endian 64-bit systems (eg PPC64) which makes
    // it a hard bug to notice.
    static Atom nativeArgToAtom(AvmCore* core, void* src, BuiltinType bt)
    {
        switch (bt)
        {
            case BUILTIN_number:
            {
                return core->doubleToAtom(unpack_double(src));
            }
            case BUILTIN_int:
            {
                return core->intToAtom((int32_t)*(const intptr_t*)src);
            }
            case BUILTIN_uint:
            {
                return core->uintToAtom((uint32_t)*(const uintptr_t*)src);
            }
            case BUILTIN_boolean:
            {
                return *(const intptr_t*)src ? trueAtom : falseAtom;
            }
            case BUILTIN_any:
            case BUILTIN_object:
            case BUILTIN_void:
            {
                return *(const Atom*)src;
            }
            case BUILTIN_string:
            {
                return (*(const Stringp*)src)->atom();
            }
            case BUILTIN_namespace:
            {
                return (*(const Namespacep*)src)->atom();
            }
            default:
            {
                return (*(ScriptObject**)src)->atom();
            }
        }
    }

#ifdef DEBUGGER

    /*static*/ DebuggerMethodInfo* DebuggerMethodInfo::create(AvmCore* core, int32_t local_count, uint32_t codeSize, int32_t max_scopes)
    {
        MMgc::GC* gc = core->GetGC();
        const uint32_t extra = (local_count <= 1) ? 0 : (sizeof(Stringp)*(local_count-1));

        DebuggerMethodInfo* dmi = new (gc, extra) DebuggerMethodInfo(local_count, codeSize, max_scopes);
        const Stringp undef = core->kundefined;
        for (int32_t i=0; i<local_count; i++)
        {
            WBRC(gc, dmi, &dmi->localNames[i], undef);
        }
        return dmi;
    }

    AbcFile* MethodInfo::file() const
    {
        DebuggerMethodInfo* dmi = this->dmi();
        return dmi ? (AbcFile*)(dmi->file) : NULL;
    }

    int32_t MethodInfo::firstSourceLine() const
    {
        DebuggerMethodInfo* dmi = this->dmi();
        return dmi ? dmi->firstSourceLine : 0;
    }

    int32_t MethodInfo::lastSourceLine() const
    {
        DebuggerMethodInfo* dmi = this->dmi();
        return dmi ? dmi->lastSourceLine : 0;
    }

    int32_t MethodInfo::offsetInAbc() const
    {
        DebuggerMethodInfo* dmi = this->dmi();
        return dmi ? dmi->offsetInAbc : 0;
    }

    uint32_t MethodInfo::codeSize() const
    {
        DebuggerMethodInfo* dmi = this->dmi();
        return dmi ? dmi->codeSize : 0;
    }

    int32_t MethodInfo::local_count() const
    {
        DebuggerMethodInfo* dmi = this->dmi();
        return dmi ? dmi->local_count : 0;
    }

    int32_t MethodInfo::max_scopes() const
    {
        DebuggerMethodInfo* dmi = this->dmi();
        return dmi ? dmi->max_scopes : 0;
    }

    void MethodInfo::setFile(AbcFile* file)
    {
        DebuggerMethodInfo* dmi = this->dmi();
        if (dmi)
            dmi->file = file;
    }

    Stringp MethodInfo::getArgName(int32_t index)
    {
        return getRegName(index);
    }

    Stringp MethodInfo::getLocalName(int32_t index)
    {
        return getRegName(index+getMethodSignature()->param_count());
    }

    void MethodInfo::updateSourceLines(int32_t linenum, int32_t offset)
    {
        DebuggerMethodInfo* dmi = this->dmi();
        if (dmi)
        {
            if (dmi->firstSourceLine == 0 || linenum < dmi->firstSourceLine)
                dmi->firstSourceLine = linenum;

            if (dmi->offsetInAbc == 0 || offset < dmi->offsetInAbc)
                dmi->offsetInAbc = offset;

            if (dmi->lastSourceLine == 0 || linenum > dmi->lastSourceLine)
                dmi->lastSourceLine = linenum;
        }
    }

    // Note: dmi() can legitimately return NULL (for MethodInfo's that were synthesized by Traits::genInitBody,
    // or for MethodInfo's that have no body, e.g. interface methods).
    DebuggerMethodInfo* MethodInfo::dmi() const
    {
        // rely on the fact that not-in-pool MethodInfo returns -1,
        // which will always be > methodCount as uint32
        const uint32_t method_id = uint32_t(this->method_id());
        AvmAssert(_pool->core->debugger() != NULL);
        // getDebuggerMethodInfo quietly returns NULL for out-of-range.
        return _pool->getDebuggerMethodInfo(method_id);
    }

    Stringp MethodInfo::getRegName(int32_t slot) const
    {
        DebuggerMethodInfo* dmi = this->dmi();

        if (dmi && slot >= 0 && slot < dmi->local_count)
            return dmi->localNames[slot];

        return this->pool()->core->kundefined;
    }

    void MethodInfo::setRegName(int32_t slot, Stringp name)
    {
        DebuggerMethodInfo* dmi = this->dmi();

        if (!dmi || slot < 0 || slot >= dmi->local_count)
            return;

        AvmCore* core = this->pool()->core;

        // [mmorearty 5/3/05] temporary workaround for bug 123237: if the register
        // already has a name, don't assign a new one
        if (dmi->localNames[slot] != core->kundefined)
            return;

        WBRC(core->GetGC(), dmi, &dmi->localNames[slot], core->internString(name));
    }

    // note that the "local" can be a true local (0..local_count-1)
    // or an entry on the scopechain (local_count...(local_count+max_scope)-1)
    Atom MethodInfo::boxOneLocal(FramePtr src, int32_t srcPos, const uint8_t* sstArr)
    {
        // if we are running jit then the types are native and we need to box them.
        if (_flags & JIT_IMPL)
        {
            src = FramePtr(uintptr_t(src) + srcPos*8);
            AvmAssert(sstArr != NULL);
            return nativeLocalToAtom(this->pool()->core, src, (SlotStorageType)sstArr[srcPos]);
        }
        else
        {
            AvmAssert(sstArr == NULL);
            src = FramePtr(uintptr_t(src) + srcPos*sizeof(Atom));
            return *(const Atom*)src;
        }
    }


    /**
     * convert ap[start]...ap[start+count-1] entries from their native types into
     * Atoms.  The result is placed into out[to]...out[to+count-1].
     *
     * The traitArr is used to determine the type of conversion that should take place.
     * traitArr[start]...traitArr[start+count-1] are used.
     *
     * If the method is interpreted then we just copy the Atom, no conversion is needed.
     */
    void MethodInfo::boxLocals(FramePtr src, int32_t srcPos, const uint8_t* sstArr, Atom* dest, int32_t destPos, int32_t length)
    {
        for (int32_t i = srcPos, n = srcPos+length; i < n; i++)
        {
#ifdef VMCFG_AOT
            AvmAssert(i >= 0 && (dmi() == NULL || i < local_count()));
#else
            AvmAssert(i >= 0 && i < getMethodSignature()->local_count());
#endif
            dest[destPos++] = boxOneLocal(src, i, sstArr);
        }
    }


    // note that the "local" can be a true local (0..local_count-1)
    // or an entry on the scopechain (local_count...(local_count+max_scope)-1)
    void MethodInfo::unboxOneLocal(Atom src, FramePtr dst, int32_t dstPos, const uint8_t* sstArr)
    {
        if (_flags & JIT_IMPL)
        {
            AvmAssert(sstArr != NULL);
            dst = FramePtr(uintptr_t(dst) + dstPos*8);
            switch ((SlotStorageType)sstArr[dstPos])
            {
                case SST_double:
                {
                    *(double*)dst = AvmCore::number_d(src);
                    break;
                }
                case SST_int32:
                {
                    *(int32_t*)dst = AvmCore::integer_i(src);
                    break;
                }
                case SST_uint32:
                {
                    *(uint32_t*)dst = AvmCore::integer_u(src);
                    break;
                }
                case SST_bool32:
                {
                    *(int32_t*)dst = (int32_t)atomGetBoolean(src);
                    break;
                }
                case SST_atom:
                {
                    *(Atom*)dst = src;
                    break;
                }
                default: // SST_string, SST_namespace, SST_scriptobject
                {
                    // ScriptObject, String, Namespace, or Null
                    *(void**)dst = atomPtr(src);
                    break;
                }
            }
        }
        else
        {
            AvmAssert(sstArr == NULL);
            dst = FramePtr(uintptr_t(dst) + dstPos*sizeof(Atom));
            *(Atom*)dst = src;
        }
    }

    /**
     * convert in[0]...in[count-1] entries from Atoms to native types placing them
     * in ap[start]...out[start+count-1].
     *
     * The traitArr is used to determine the type of conversion that should take place.
     * traitArr[start]...traitArr[start+count-1] are used.
     *
     * If the method is interpreted then we just copy the Atom, no conversion is needed.
     */
    void MethodInfo::unboxLocals(const Atom* src, int32_t srcPos, const uint8_t* sstArr, FramePtr dest, int32_t destPos, int32_t length)
    {
        for (int32_t i = destPos, n = destPos+length; i < n; i++)
        {
#ifdef VMCFG_AOT
            AvmAssert(i >= 0 && (dmi() == NULL || i < local_count()));
#else
            AvmAssert(i >= 0 && i < getMethodSignature()->local_count());
#endif
            unboxOneLocal(src[srcPos++], dest, i, sstArr);
        }
    }

#endif //DEBUGGER


#ifdef AVMPLUS_VERBOSE
    Stringp MethodInfo::format(AvmCore* core) const
    {
        Stringp n = getMethodName();
        return n ?
                n->appendLatin1("()") :
                core->newConstantStringLatin1("?()");
    }
#endif // AVMPLUS_VERBOSE

    bool MethodInfo::makeMethodOf(Traits* traits)
    {
        AvmAssert(!isResolved());
// begin AVMPLUS_UNCHECKED_HACK
        AvmAssert(!(_flags & PROTOFUNC));
// end AVMPLUS_UNCHECKED_HACK

        if (_declarer.getTraits() == NULL)
        {
            _declarer.setTraits(pool()->core->GetGC(), this, traits);
            _flags |= NEED_CLOSURE;
            return true;
        }
        else
        {
            #ifdef AVMPLUS_VERBOSE
            if (pool()->isVerbose(VB_parse))
                pool()->core->console << "WARNING: method " << this << " was already bound to " << declaringTraits() << "\n";
            #endif

            return false;
        }
    }

    void MethodInfo::makeIntoPrototypeFunction(const Toplevel* toplevel, const ScopeTypeChain* fscope)
    {
        AvmAssert(_declarer.getScope() == NULL);

        _declarer.setScope(toplevel->core()->GetGC(), this, fscope);

// begin AVMPLUS_UNCHECKED_HACK
        this->_flags |= PROTOFUNC;
// end AVMPLUS_UNCHECKED_HACK

        // make sure param & return types are fully resolved.
        // this does not set the verified flag, so real verification will
        // still happen before the function runs the first time.
        resolveSignature(toplevel);
    }

    /**
     * convert native args to atoms.  argc is the number of
     * args, not counting the instance which is arg[0].  the
     * layout is [instance][arg1..argN]
     */
    void MethodSignature::boxArgs(AvmCore* core, int32_t argc, const uint32_t* ap, Atom* out) const
    {
        MMGC_STATIC_ASSERT(sizeof(Atom) == sizeof(void*));  // if this ever changes, this function needs smartening
        typedef const Atom* ConstAtomPtr;
        // box the typed args, up to param_count
        const int32_t param_count = this->param_count();
        for (int32_t i=0; i <= argc; i++)
        {
            const BuiltinType bt = (i <= param_count) ? this->paramTraitsBT(i) : BUILTIN_any;
            out[i] = nativeArgToAtom(core, (void*)ap, bt);
            ap += (bt == BUILTIN_number ? sizeof(double) : sizeof(Atom)) / sizeof(int32_t);
        }
    }

    static uint32_t argSize(Traits* t) { return Traits::getBuiltinType(t) == BUILTIN_number ? sizeof(double) : sizeof(Atom); }

    MethodSignature* FASTCALL MethodInfo::_buildMethodSignature(const Toplevel* toplevel)
    {
        PoolObject* pool = this->pool();
        AvmCore* core = pool->core;
        GC* gc = core->GetGC();

        const byte* pos = this->_abc_info_pos;
        const uint32_t param_count = pos ? AvmCore::readU32(pos) : 0;
        uint32_t optional_count = 0;
        uint32_t rest_offset = 0;
        Traits* returnType;
        Traits* receiverType;

        // we only need param_count+optional_count extra, but we don't know
        // optional_count yet, so over-allocate for the worst case. (we know optional_count<=param_count).
        // this wastes space, but since we only cache a few dozen of these, it's preferable to walking the data twice
        // to get an exact count.
        const uint32_t extra = sizeof(MethodSignature::AtomOrType) * (param_count + (hasOptional() ? param_count : 0));
        MethodSignature* ms = new (gc, extra) MethodSignature();
        if (pos)
        {
            returnType = pool->resolveTypeName(pos, toplevel, /*allowVoid=*/true);
            receiverType = (_flags & NEED_CLOSURE) ? declaringTraits() : core->traits.object_itraits;
            rest_offset = argSize(receiverType);
// begin AVMPLUS_UNCHECKED_HACK
            uint32_t untyped_args = 0;
// end AVMPLUS_UNCHECKED_HACK
            for (uint32_t i=1; i <= param_count; i++)
            {
                Traits* argType = pool->resolveTypeName(pos, toplevel);
                WB(gc, ms, &ms->_args[i].paramType, argType);
                rest_offset += argSize(argType);
// begin AVMPLUS_UNCHECKED_HACK
                untyped_args += (argType == NULL);
// end AVMPLUS_UNCHECKED_HACK
            }
            AvmCore::skipU32(pos); // name_index;
            pos++; // abcFlags;
// begin AVMPLUS_UNCHECKED_HACK
            // toplevel!=NULL check is so we only check when resolveSignature calls us (not subsequently)
            if (toplevel != NULL && (_flags & PROTOFUNC))
            {
                // HACK - compiler should do this, and only to toplevel functions
                // that meet the E4 criteria for being an "unchecked function"
                // the tests below are too loose

                // if all params and return types are Object then make all params optional=undefined
                if (param_count == 0)
                    _flags |= IGNORE_REST;
                if (!hasOptional() && returnType == NULL && param_count > 0 && untyped_args == param_count)
                {
                    _flags |= HAS_OPTIONAL | IGNORE_REST | UNCHECKED;
                    // oops -- the ms we allocated is too small, has no space for optional values
                    // but it's easy to re-create, especially since we know the types are all null
                    const uint32_t extra = sizeof(MethodSignature::AtomOrType) * (param_count + param_count);
                    ms = new (gc, extra) MethodSignature();
                    // don't need to re-set paramTypes: they are inited to NULL which is the right value
                }
            }
            if (_flags & UNCHECKED)
            {
                optional_count = param_count;
                for (uint32_t j=0; j < optional_count; j++)
                {
                    //WBATOM(gc, ms, &ms->_args[param_count+1+j].defaultValue, undefinedAtom);
                    // since we know we're going from NULL->undefinedAtom we can skip the WBATOM call
                    ms->_args[param_count+1+j].defaultValue = undefinedAtom;
                }
            }
            else
// end AVMPLUS_UNCHECKED_HACK
            if (hasOptional())
            {
                optional_count = AvmCore::readU32(pos);
                for (uint32_t j=0; j < optional_count; j++)
                {
                    const int32_t param = param_count-optional_count+1+j;
                    const int32_t index = AvmCore::readU32(pos);
                    CPoolKind kind = (CPoolKind)*pos++;

                    // check that the default value is legal for the param type
                    Traits* argType = ms->_args[param].paramType;
                    const Atom value = pool->getLegalDefaultValue(toplevel, index, kind, argType);
                    WBATOM(gc, ms, &ms->_args[param_count+1+j].defaultValue, value);
                }
            }

            if (!isNative())
            {
                const byte* body_pos = this->abc_body_pos();
                if (body_pos)
                {
                    ms->_max_stack = AvmCore::readU32(body_pos);
                    ms->_local_count = AvmCore::readU32(body_pos);
                    const int32_t init_scope_depth = AvmCore::readU32(body_pos);
                    ms->_max_scope = AvmCore::readU32(body_pos) - init_scope_depth;
                #ifdef VMCFG_WORDCODE
                #else
                    ms->_abc_code_start = body_pos;
                    AvmCore::skipU32(ms->_abc_code_start); // code_length
                #endif
                }
            }
        }
        else
        {
            // this is a synthesized MethodInfo from genInitBody
            AvmAssert(param_count == 0);
            rest_offset = sizeof(Atom);
            returnType = pool->core->traits.void_itraits;
            receiverType = declaringTraits();
            // values derived from Traits::genInitBody()
            const int32_t max_stack = 2;
            const int32_t local_count = 1;
            const int32_t init_scope_depth = 1;
            const int32_t max_scope_depth = 1;
            ms->_max_stack = max_stack;
            ms->_local_count = local_count;
            ms->_max_scope = max_scope_depth - init_scope_depth;
        #if defined(VMCFG_WORDCODE) || defined(VMCFG_AOT)
        #else
            ms->_abc_code_start = this->abc_body_pos();
            AvmCore::skipU32(ms->_abc_code_start, 5);
        #endif
        }
        ms->_frame_size = ms->_local_count + ms->_max_scope + ms->_max_stack;
        #ifndef AVMPLUS_64BIT
        // The interpreter wants this to be padded to a doubleword boundary because
        // it allocates two objects in a single alloca() request - the frame and
        // auxiliary storage, in that order - and wants the second object to be
        // doubleword aligned.
        ms->_frame_size = (ms->_frame_size + 1) & ~1;
        #endif
        ms->_param_count = param_count;
        ms->_optional_count = optional_count;
        ms->_rest_offset = rest_offset;
        ms->_flags = this->_flags;
        WB(gc, ms, &ms->_returnTraits, returnType);
        WB(gc, ms, &ms->_args[0].paramType, receiverType);

        AvmAssert(_msref->get() == NULL);
        _msref = ms->GetWeakRef();
        core->msCache()->add(ms);
        return ms;
    }

    MethodSignature* FASTCALL MethodInfo::_getMethodSignature()
    {
        AvmAssert(this->isResolved());
        // note: MethodSignaturep are always built the first time in resolveSignature; this is only
        // executed for subsequent re-buildings. Thus we pass NULL for toplevel (it's only used
        // for verification errors, but those will have been caught prior to this) and for
        // abcGen and imtBuilder (since those only need to be done once).
        MethodSignature* ms = _buildMethodSignature(NULL);
        return ms;
    }

    void MethodInfo::update_max_stack(int32_t max_stack)
    {
        MethodSignature* ms = (MethodSignature*)_msref->get();
        if (ms)
        {
            ms->_max_stack = max_stack;
        }
    }

    void MethodInfo::resolveSignature(const Toplevel* toplevel)
    {
        if (!(_flags & RESOLVED))
        {
            MethodSignature* ms = _buildMethodSignature(toplevel);

            if (!isNative())
            {
                if (ms->frame_size() < 0 || ms->local_count() < 0 || ms->max_scope() < 0)
                    toplevel->throwVerifyError(kCorruptABCError);
            }

            if (ms->paramTraits(0) != NULL && ms->paramTraits(0)->isInterface())
                _flags |= ABSTRACT_METHOD;

            _flags |= RESOLVED;

            if (ms->returnTraitsBT() == BUILTIN_number && _implGPR == MethodInfo::verifyEnterGPR)
                _implFPR = MethodInfo::verifyEnterFPR;
        }
    }

    Traits* MethodInfo::resolveActivation(const Toplevel* toplevel)
    {
        Traits* atraits = this->activationTraits();

        const ScopeTypeChain* ascope = this->declaringScope()->cloneWithNewTraits(pool()->core->GetGC(), atraits);

        atraits->resolveSignatures(toplevel);
        atraits->setDeclaringScopes(ascope);

        _activation.setScope(pool()->core->GetGC(), this, ascope);

        return atraits;
    }

#ifdef DEBUGGER
    uint64_t MethodInfo::bytesUsed()
    {
        uint64_t size = sizeof(MethodInfo);
        size += getMethodSignature()->param_count() * 2 * sizeof(Atom);
        size += codeSize();
        return size;
    }
#endif

    bool MethodInfo::usesCallerContext() const
    {
        return pool()->isBuiltin && (!isNative() || (_flags & NEEDS_CODECONTEXT));
    }

    // Builtin + non-native functions always need the dxns code emitted
    // Builtin + native functions have flags to specify if they need the dxns code
    bool MethodInfo::usesDefaultXmlNamespace() const
    {
        return pool()->isBuiltin && (!isNative() || (_flags & NEEDS_DXNS));
    }

#if VMCFG_METHOD_NAMES
    Stringp MethodInfo::getMethodName(bool includeAllNamespaces) const
    {
        Stringp methodName = NULL;

#ifdef AVMPLUS_SAMPLER
        // We cache method names, because the profiler requests them over and
        // over.  (Bug 2547382)
        methodName = _methodName;
#endif

        if (!methodName)
        {
            Traits* declaringTraits = this->declaringTraits();

            methodName = getMethodNameWithTraits(declaringTraits, includeAllNamespaces);

#ifdef AVMPLUS_SAMPLER
            Sampler* sampler = declaringTraits ? declaringTraits->core->get_sampler() : NULL;
            if (sampler && sampler->sampling())
                _methodName = methodName;
#endif
        }

        return methodName;
    }

    Stringp MethodInfo::getMethodNameWithTraits(Traits* t, bool includeAllNamespaces) const
    {
        Stringp name = NULL;
        const int32_t method_id = this->method_id();

        PoolObject* pool = this->pool();
        AvmCore* core = pool->core;
        if (core->config.methodNames)
        {
            name = pool->getMethodInfoName(method_id);
            if (name && name->length() == 0)
            {
                name = core->kanonymousFunc;
            }

            if (t)
            {
                Stringp tname = t->format(core, includeAllNamespaces);
                if (core->config.oldVectorMethodNames)
                {
                    // Tamarin used to incorrectly return the internal name of these
                    // Vector types rather than the "official" name due to initialization
                    // order. Names on the left are "more correct" but old builds might
                    // require the "classic" name for compatibility purposes, so check.
                    struct NameMapRec { const char* n; const char* o; };
                    static const NameMapRec kNameMap[4] =
                    {
                        { "Vector.<Number>", "Vector$double" },
                        { "Vector.<int>", "Vector$int" },
                        { "Vector.<uint>", "Vector$uint" },
                        { "Vector.<*>", "Vector$object" },
                    };
                    for (int32_t i = 0; i < 4; ++i)
                    {
                        if (tname->equalsLatin1(kNameMap[i].n))
                        {
                            tname = core->newConstantStringLatin1(kNameMap[i].o);
                            break;
                        }
                    }
                };

                if (this == t->init)
                {
                    // careful, name could be null, that's ok for init methods
                    if (t->posType() == TRAITSTYPE_SCRIPT)
                    {
                        name = tname->appendLatin1("$init");
                    }
                    else if (t->posType() == TRAITSTYPE_CLASS)
                    {
                        name = tname->appendLatin1("cinit");
                    }
                    else
                    {
                        AvmAssert(t->isInstanceType() || t->isActivationTraits());
                        name = tname;
                    }
                }
                else if (name)
                {
                    const char* sep;
                    if (_flags & IS_GETTER)
                        sep = "/get ";
                    else if (_flags & IS_SETTER)
                        sep = "/set ";
                    else
                        sep = "/";
                    name = tname->appendLatin1(sep)->append(name);
                }
            }
        }

        // if config.methodNames isn't set, might as well still return a non-null result
        if (name == NULL)
            name = core->concatStrings(core->newConstantStringLatin1("MethodInfo-"), core->intToString(method_id));

        return name;
    }
#endif

}
