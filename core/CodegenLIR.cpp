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
 *   leon.sha@sun.com
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

#if defined(WIN32) && defined(AVMPLUS_ARM)
#include <cmnintrin.h>
#endif

#if defined AVMPLUS_IA32 || defined AVMPLUS_AMD64
# define SSE2_ONLY(...) __VA_ARGS__
#else
# define SSE2_ONLY(...)
#endif

#ifdef _MSC_VER
    #if !defined (AVMPLUS_ARM)
    extern "C"
    {
        int __cdecl _setjmp3(jmp_buf jmpbuf, int arg);
    }
    #else
    #include <setjmp.h>
    #undef setjmp
    extern "C"
    {
        int __cdecl setjmp(jmp_buf jmpbuf);
    }
    #endif // AVMPLUS_ARM
#endif // _MSC_VER

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

#ifdef PERFM
#define DOPROF
#endif /* PERFM */

//#define DOPROF
#include "../vprof/vprof.h"

#ifdef AVMPLUS_64BIT
#define AVMCORE_integer         AvmCore::integer64
#define AVMCORE_integer_d       AvmCore::integer64_d
#define AVMCORE_integer_d_sse2  AvmCore::integer64_d_sse2
#define PTR_SCALE 3
#else
#define AVMCORE_integer         AvmCore::integer
#define AVMCORE_integer_d       AvmCore::integer_d
#define AVMCORE_integer_d_sse2  AvmCore::integer_d_sse2
#define PTR_SCALE 2
#endif

#define IS_ALIGNED(x, size) ((uintptr_t(x) & ((size)-1)) == 0)

namespace avmplus
{
        #define COREADDR(f) coreAddr((int (AvmCore::*)())(&f))
        #define GCADDR(f) gcAddr((int (MMgc::GC::*)())(&f))
        #define ENVADDR(f) envAddr((int (MethodEnv::*)())(&f))
		#define METHODINFOADDR(f) methodInfoAddr((int (MethodInfo::*)()(&f)))
        #define ARRAYADDR(f) arrayAddr((int (ArrayObject::*)())(&f))
        #define VECTORINTADDR(f) vectorIntAddr((int (IntVectorObject::*)())(&f))
        #define VECTORUINTADDR(f) vectorUIntAddr((int (UIntVectorObject::*)())(&f))
        #define VECTORDOUBLEADDR(f) vectorDoubleAddr((int (DoubleVectorObject::*)())(&f))
        #define VECTOROBJADDR(f) vectorObjAddr((int (ObjectVectorObject::*)())(&f))
        #define EFADDR(f)   efAddr((int (ExceptionFrame::*)())(&f))
        #define DEBUGGERADDR(f)   debuggerAddr((int (Debugger::*)())(&f))
        #define FUNCADDR(addr) (uintptr)addr

   #ifdef VTUNE
       extern void VTune_RegisterMethod(AvmCore* core, JITCodeInfo* inf);
   #endif  // VTUNE

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

		intptr_t  methodInfoAddr( int (MethodInfo::*f)() )
        {
            RETURN_METHOD_PTR(MethodInfo, f);
        }

    #ifdef DEBUGGER
        intptr_t  debuggerAddr( int (Debugger::*f)() )
        {
            RETURN_METHOD_PTR(Debugger, f);
        }
    #endif /* DEBUGGER */

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

    using namespace MMgc;
    using namespace nanojit;

    #if defined _MSC_VER && !defined AVMPLUS_ARM
    #  define SETJMP ((uintptr)_setjmp3)
    #elif defined AVMPLUS_MAC_CARBON
    #  define SETJMP setjmpAddress
    #else
    #  define SETJMP ((uintptr)VMPI_setjmpNoUnwind)
    #endif // _MSC_VER

    #include "../core/jit-calls.h"

#if NJ_EXPANDED_LOADSTORE_SUPPORTED && defined(VMCFG_UNALIGNED_INT_ACCESS) && defined(VMCFG_LITTLE_ENDIAN)
    #define VMCFG_MOPS_USE_EXPANDED_LOADSTORE_INT
#endif

#if NJ_EXPANDED_LOADSTORE_SUPPORTED && defined(VMCFG_UNALIGNED_FP_ACCESS) && defined(VMCFG_LITTLE_ENDIAN)
    #define VMCFG_MOPS_USE_EXPANDED_LOADSTORE_FP
#endif

    // AccSet conventions
    // nanojit currently supports very coarse grained access-set tags
    // for loads, stores, and calls.  See the comments in LIR.h for a
    // detailed description.  Here we define a set of constants to use that
    // are more fine grained, and then map them to AccessSets that nanojit supports.
    //
    // If you aren't sure what to use, use ACC_LOAD_ANY or ACC_STORE_ANY.
    // Warning: if you annotate a load or store with something other
    // than ACC_*_ANY, and get it wrong, you will introduce subtle and
    // hard to find bugs: "if you lie to the compiler, it will get its revenge"
    //
    // we define new names for the alias sets that we will use here, but we
    // intentionally do *not* define more than one new name that maps to the
    // same nanojit AccSet value.  Why?  Because that would be a lie -- a person
    // reading the code would see two different names in use and conclude the
    // loads don't alias.  But since the names map to the same AccSet value,
    // they *do* alias.
    //
    // on the other hand, we don't just use the predefined AccSet names because
    // our new names have concrete meaning in the context of this VM, whereas
    // the predefined AccSet names in LIR.h do not.
    //
    // we use ACC_OTHER for a catchall that does not overlap with any other
    // predefined alias set.  Future work should subdivide this set where
    // the improvements outweigh the cost of additional alias sets.

    static const AccSet ACC_VARS = ACC_STACK;   // values of local variables
    static const AccSet ACC_TAGS = ACC_RSTACK;  // BuiltinTraits tags for local variables

    struct MopsInfo
    {
        uint32_t size;
        LOpcode op;
        const CallInfo* call;
    };

    static const MopsInfo kMopsLoadInfo[7] = {
        { 1, LIR_ldc2i,   FUNCTIONID(mop_lix8) },
        { 2, LIR_lds2i,   FUNCTIONID(mop_lix16) },
        { 1, LIR_lduc2ui, FUNCTIONID(mop_liz8) },
        { 2, LIR_ldus2ui, FUNCTIONID(mop_liz16) },
        { 4, LIR_ldi,     FUNCTIONID(mop_li32) },
        { 4, LIR_ldf2d,   FUNCTIONID(mop_lf32) },
        { 8, LIR_ldd,     FUNCTIONID(mop_lf64) }
    };

    static const MopsInfo kMopsStoreInfo[5] = {
        { 1, LIR_sti2c, FUNCTIONID(mop_si8) },
        { 2, LIR_sti2s, FUNCTIONID(mop_si16) },
        { 4, LIR_sti,   FUNCTIONID(mop_si32) },
        { 4, LIR_std2f, FUNCTIONID(mop_sf32) },
        { 8, LIR_std,   FUNCTIONID(mop_sf64) }
     };

    class MopsRangeCheckFilter: public LirWriter
    {
    private:
        LirWriter* const prolog_out;
        LInsp const env_domainenv;
        LInsp curMemBase;
        LInsp curMemSize;
        LInsp curMopAddr;
        LInsp curRangeCheckLHS;
        LInsp curRangeCheckRHS;
        int32_t curRangeCheckMinValue;
        int32_t curRangeCheckMaxValue;

    private:
        void clearMemBaseAndSize();

        static void extractConstantDisp(LInsp& mopAddr, int32_t& curDisp);
        LIns* safeIns2(LOpcode op, LIns*, int32_t);
        void safeRewrite(LIns* ins, int32_t);

    public:
        MopsRangeCheckFilter(LirWriter* out, LirWriter* prolog_out, LInsp env_domainenv);

        LInsp emitRangeCheck(LInsp& mopAddr, int32_t const size, int32_t* disp, LInsp& br);
        void flushRangeChecks();

        // overrides from LirWriter
        LIns* ins0(LOpcode v);
        LIns* insCall(const CallInfo* call, LInsp args[]);
    };

    inline MopsRangeCheckFilter::MopsRangeCheckFilter(LirWriter* out, LirWriter* prolog_out, LInsp env_domainenv) :
        LirWriter(out),
        prolog_out(prolog_out),
        env_domainenv(env_domainenv),
        curMemBase(NULL),
        curMemSize(NULL),
        curMopAddr(NULL),
        curRangeCheckLHS(NULL),
        curRangeCheckRHS(NULL),
        curRangeCheckMinValue(int32_t(0x7fffffff)),
        curRangeCheckMaxValue(int32_t(0x80000000))
    {
        clearMemBaseAndSize();
    }

    void MopsRangeCheckFilter::clearMemBaseAndSize()
    {
        curMemBase = curMemSize = NULL;
    }

    void MopsRangeCheckFilter::flushRangeChecks()
    {
        AvmAssert((curRangeCheckLHS != NULL) == (curRangeCheckRHS != NULL));
        if (curRangeCheckLHS)
        {
            curRangeCheckLHS = curRangeCheckRHS = curMopAddr = NULL;
            curRangeCheckMinValue = int32_t(0x7fffffff);
            curRangeCheckMaxValue = int32_t(0x80000000);
            // but don't clearMemBaseAndSize()!
        }
        else
        {
            AvmAssert(curMopAddr == NULL);
            AvmAssert(curRangeCheckMinValue == int32_t(0x7fffffff));
            AvmAssert(curRangeCheckMaxValue == int32_t(0x80000000));
        }
    }

    static bool sumFitsInInt32(int32_t a, int32_t b)
    {
        return int64_t(a) + int64_t(b) == int64_t(a + b);
    }

    /*static*/ void MopsRangeCheckFilter::extractConstantDisp(LInsp& mopAddr, int32_t& curDisp)
    {
        // mopAddr is an int (an offset from globalMemoryBase) on all archs.
        // if mopAddr is an expression of the form
        //      expr+const
        //      const+expr
        //      expr-const
        //      (but not const-expr)
        // then try to pull the constant out and return it as a displacement to
        // be used in the instruction as an addressing-mode offset.
        // (but only if caller requests it that way.)
        for (;;)
        {
            LOpcode const op = mopAddr->opcode();
            if (op != LIR_addi && op != LIR_subi)
                break;

            int32_t imm;
            LInsp nonImm;
            if (mopAddr->oprnd2()->isImmI())
            {
                imm = mopAddr->oprnd2()->immI();
                nonImm = mopAddr->oprnd1();

                if (op == LIR_subi)
                    imm = -imm;
            }
            else if (mopAddr->oprnd1()->isImmI())
            {
                // don't try to optimize const-expr
                if (op == LIR_subi)
                    break;

                imm = mopAddr->oprnd1()->immI();
                nonImm = mopAddr->oprnd2();
            }
            else
            {
                break;
            }

            if (!sumFitsInInt32(curDisp, imm))
                break;

            curDisp += imm;
            mopAddr = nonImm;
        }
    }

    LInsp MopsRangeCheckFilter::emitRangeCheck(LInsp& mopAddr, int32_t const size, int32_t* disp, LInsp& br)
    {
        int32_t offsetMin = 0;
        if (disp != NULL)
        {
            *disp = 0;
            extractConstantDisp(mopAddr, *disp);
            offsetMin = *disp;
        }

        int32_t offsetMax = offsetMin + size;

        AvmAssert((curRangeCheckLHS != NULL) == (curRangeCheckRHS != NULL));

        AvmAssert(mopAddr != NULL);
        if (curRangeCheckLHS != NULL && curMopAddr == mopAddr)
        {
            int32_t n_curRangeCheckMin = curRangeCheckMinValue;
            if (n_curRangeCheckMin > offsetMin)
                n_curRangeCheckMin = offsetMin;
            int32_t n_curRangeCheckMax = curRangeCheckMaxValue;
            if (n_curRangeCheckMax < offsetMax)
                n_curRangeCheckMax = offsetMax;

            if ((n_curRangeCheckMax - n_curRangeCheckMin) <= DomainEnv::GLOBAL_MEMORY_MIN_SIZE)
            {
                if (curRangeCheckMinValue != n_curRangeCheckMin)
                    safeRewrite(curRangeCheckLHS, curRangeCheckMinValue);

                if ((n_curRangeCheckMax - n_curRangeCheckMin) != (curRangeCheckMaxValue - curRangeCheckMinValue))
                    safeRewrite(curRangeCheckRHS, curRangeCheckMaxValue - curRangeCheckMinValue);

                curRangeCheckMinValue = n_curRangeCheckMin;
                curRangeCheckMaxValue = n_curRangeCheckMax;
            }
            else
            {
                // if collapsed ranges get too large, pre-emptively flush, so that the
                // range-checking code can always assume the range is within minsize
                flushRangeChecks();
            }
        }
        else
        {
            flushRangeChecks();
        }

        if (!curMemBase)
        {
            //AvmAssert(curMemSize == NULL);
            curMemBase = out->insLoad(LIR_ldp, env_domainenv, offsetof(DomainEnv,m_globalMemoryBase), ACC_OTHER);
            curMemSize = out->insLoad(LIR_ldi, env_domainenv, offsetof(DomainEnv,m_globalMemorySize), ACC_OTHER);
        }

        AvmAssert((curRangeCheckLHS != NULL) == (curRangeCheckRHS != NULL));

        if (!curRangeCheckLHS)
        {
            AvmAssert(!curMopAddr);
            curMopAddr = mopAddr;
            curRangeCheckMinValue = offsetMin;
            curRangeCheckMaxValue = offsetMax;

            AvmAssert(env_domainenv != NULL);

            // we want to pass range-check if
            //
            //      (curMopAddr+curRangeCheckMin >= 0 && curMopAddr+curRangeCheckMax <= mopsMemorySize)
            //
            // which is the same as
            //
            //      (curMopAddr >= -curRangeCheckMin && curMopAddr <= mopsMemorySize - curRangeCheckMax)
            //
            // which is the same as
            //
            //      (curMopAddr >= -curRangeCheckMin && curMopAddr < mopsMemorySize - curRangeCheckMax + 1)
            //
            // and since (x >= min && x < max) is equivalent to (unsigned)(x-min) < (unsigned)(max-min)
            //
            //      (unsigned(curMopAddr + curRangeCheckMin) < unsigned(mopsMemorySize - curRangeCheckMax + 1 + curRangeCheckMin))
            //
            // from there, you'd think you could do
            //
            //      (curMopAddr < mopsMemorySize - curRangeCheckMax + 1))
            //
            // but that is only valid if you are certain that curMopAddr>0, due to the unsigned casting...
            // and curMopAddr could be anything, which is really the point of this whole exercise. Instead, settle for
            //
            //      (unsigned(curMopAddr + curRangeCheckMin) <= unsigned(mopsMemorySize - (curRangeCheckMax-curRangeCheckMin)))
            //

            AvmAssert(curRangeCheckMaxValue > curRangeCheckMinValue);
            AvmAssert(curRangeCheckMaxValue - curRangeCheckMinValue <= DomainEnv::GLOBAL_MEMORY_MIN_SIZE);

            curRangeCheckLHS = safeIns2(LIR_addi, curMopAddr, curRangeCheckMinValue);
            curRangeCheckRHS = safeIns2(LIR_subi, curMemSize, curRangeCheckMaxValue - curRangeCheckMinValue);

            LInsp cond = this->ins2(LIR_leui, curRangeCheckLHS, curRangeCheckRHS);
            br = this->insBranch(LIR_jf, cond, NULL);
        }

        return curMemBase;
    }

    // workaround for WE2569232: don't let these adds get specialized or CSE'd.
    LIns* MopsRangeCheckFilter::safeIns2(LOpcode op, LIns* lhs, int32_t rhsConst)
    {
        LIns* rhs = prolog_out->insImmI(rhsConst);
        LIns* ins = out->ins2(op, lhs, rhs);
        AvmAssert(ins->isop(op) && ins->oprnd1() == lhs && ins->oprnd2() == rhs);
        return ins;
    }

    // rewrite the instruction with a new rhs constant
    void MopsRangeCheckFilter::safeRewrite(LIns* ins, int32_t rhsConst)
    {
        LIns* rhs = prolog_out->insImmI(rhsConst);
        AvmAssert(ins->isop(LIR_addi) || ins->isop(LIR_subi));
        ins->initLInsOp2(ins->opcode(), ins->oprnd1(), rhs);
    }

    LIns* MopsRangeCheckFilter::ins0(LOpcode v)
    {
        if (v == LIR_label)
        {
            flushRangeChecks();
            clearMemBaseAndSize();
        }
        return LirWriter::ins0(v);
    }

    LInsp MopsRangeCheckFilter::insCall(const CallInfo *ci, LInsp args[])
    {
        // calls could potentially resize globalMemorySize, so we
        // can't collapse range checks across them
        if (!ci->_isPure)
        {
            flushRangeChecks();
            clearMemBaseAndSize();
        }
        return LirWriter::insCall(ci, args);
    }

    /**
     * ---------------------------------
     * Instruction convenience functions
     * ---------------------------------
     */

    // address calc instruction
    LIns* CodegenLIR::leaIns(int32_t disp, LIns* base) {
        return lirout->ins2(LIR_addp, base, InsConstPtr((void*)disp));
    }

    // call
    LIns* LirHelper::callIns(const CallInfo *ci, uint32_t argc, ...)
    {
        va_list ap;
        va_start(ap, argc);
        LIns* ins = vcallIns(ci, argc, ap);
        va_end(ap);
        return ins;
    }

    LIns* LirHelper::vcallIns(const CallInfo *ci, uint32_t argc, va_list ap)
    {
        AvmAssert(argc <= MAXARGS);
        AvmAssert(argc == ci->count_args());
        LInsp argIns[MAXARGS];
        for (uint32_t i=0; i < argc; i++)
            argIns[argc-i-1] = va_arg(ap, LIns*);
        return lirout->insCall(ci, argIns);
    }

    LIns* CodegenLIR::localCopy(int i)
    {
        switch (bt(state->value(i).traits)) {
        case BUILTIN_number:
            return localGetf(i);
        case BUILTIN_boolean:
        case BUILTIN_int:
        case BUILTIN_uint:
            return localGet(i);
        default:
            return localGetp(i);
        }
    }

    // returns true if mask has exactly one bit set
    // see http://aggregate.org/MAGIC/#Is%20Power%20of%202
    REALLY_INLINE bool exactlyOneBit(uint32_t m)
    {
        AvmAssert(m != 0);
        return (m & (m-1)) == 0;
    }

    void CodegenLIR::localSet(int i, LIns* o, Traits* type)
    {
        BuiltinType tag = bt(type);
        SlotStorageType sst = valueStorageType(tag);
#ifdef DEBUG
        jit_sst[i] = uint8_t(1 << sst);
#endif
        lirout->insStore(o, vars, i * 8, ACC_VARS);
        lirout->insStore(LIR_sti2c, InsConst(sst), tags, i, ACC_TAGS);
    }

    LIns* CodegenLIR::atomToNativeRep(int i, LIns* atom)
    {
        return atomToNativeRep(state->value(i).traits, atom);
    }

    LIns* CodegenLIR::ptrToNativeRep(Traits*t, LIns* ptr)
    {
        return t->isMachineType() ? addp(ptr, kObjectType) : ptr;
    }

#ifdef _DEBUG
    bool CodegenLIR::isPointer(int i)   {
        return !state->value(i).traits->isMachineType();
    }
#endif

    LIns* CodegenLIR::loadAtomRep(int i)
    {
        return nativeToAtom(localCopy(i), state->value(i).traits);
    }

    LIns* LirHelper::nativeToAtom(LIns* native, Traits* t)
    {
        switch (bt(t)) {
        case BUILTIN_number:
            return callIns(FUNCTIONID(doubleToAtom), 2, coreAddr, native);

        case BUILTIN_any:
        case BUILTIN_object:
        case BUILTIN_void:
            return native;  // value already represented as Atom

        case BUILTIN_int:
            if (native->isImmI()) {
                int32_t val = native->immI();
                if (atomIsValidIntptrValue(val))
                    return InsConstAtom(atomFromIntptrValue(val));
            }
            return callIns(FUNCTIONID(intToAtom), 2, coreAddr, native);

        case BUILTIN_uint:
            if (native->isImmI()) {
                uint32_t val = native->immI();
                if (atomIsValidIntptrValue_u(val))
                    return InsConstAtom(atomFromIntptrValue_u(val));
            }
            return callIns(FUNCTIONID(uintToAtom), 2, coreAddr, native);

        case BUILTIN_boolean:
            return ui2p(addi(lshi(native, 3), kBooleanType));

        case BUILTIN_string:
            return addp(native, kStringType);

        case BUILTIN_namespace:
            return addp(native, kNamespaceType);

        default:
            return addp(native, kObjectType);
        }
    }

    LIns* CodegenLIR::storeAtomArgs(int count, int index)
    {
        LIns* ap = insAlloc(sizeof(Atom)*count);
        for (int i=0; i < count; i++)
            stp(loadAtomRep(index++), ap, i * sizeof(Atom), ACC_OTHER);
        return ap;
    }

    LIns* CodegenLIR::storeAtomArgs(LIns* receiver, int count, int index)
    {
        #ifdef NJ_VERBOSE
        if (verbose())
            core->console << "          store args\n";
        #endif
        LIns* ap = insAlloc(sizeof(Atom)*(count+1));
        stp(receiver, ap, 0, ACC_OTHER);
        for (int i=1; i <= count; i++)
        {
            LIns* v = loadAtomRep(index++);
            stp(v, ap, sizeof(Atom)*i, ACC_OTHER);
        }
        return ap;
    }

    // initialize the code manager the first time we jit any method for this PoolObject.
    CodeMgr* initCodeMgr(PoolObject *pool) {
        if (!pool->codeMgr) {
            CodeMgr *mgr = mmfx_new( CodeMgr() );
            pool->codeMgr = mgr;
#ifdef NJ_VERBOSE
            mgr->log.core = pool->core;
            mgr->log.lcbits = pool->verbose_vb;
#endif
        }
        return pool->codeMgr;
    }

#ifdef NJ_VERBOSE
    void AvmLogControl::printf( const char* format, ... )
    {
        AvmAssert(core!=NULL);

        va_list vargs;
        va_start(vargs, format);

        char str[1024];
        VMPI_vsnprintf(str, sizeof(str), format, vargs);
        va_end(vargs);

        core->console << str;
    }
#endif

    CodegenLIR::CodegenLIR(MethodInfo* i) :
        LirHelper(i->pool()),
        overflow(false),
        abcStart(NULL),
        abcEnd(NULL),
		/*
#ifdef VTUNE
        hasDebugInfo(false),
        jitInfoList(i->core()->gc),
        jitPendingRecords(i->core()->gc),
#endif
		*/
        info(i),
        ms(i->getMethodSignature()),
        pool(i->pool()),
        state(NULL),
        mopsRangeCheckFilter(NULL),
        interruptable(true),
        npe_label("npe"),
        interrupt_label("interrupt"),
        mop_rangeCheckFailed_label("mop_rangeCheckFailed"),
        catch_label("catch"),
        call_cache_builder(*alloc1, *initCodeMgr(pool)),
        get_cache_builder(*alloc1, *pool->codeMgr),
        set_cache_builder(*alloc1, *pool->codeMgr),
        prolog(NULL),
        blockLabels(NULL)
        DEBUGGER_ONLY(, haveDebugger(core->debugger() != NULL) )
    {
        #ifdef AVMPLUS_MAC_CARBON
        setjmpInit();
        #endif

        verbose_only(
            if (pool->isVerbose(VB_jit)) {
                core->console << "codegen " << i;
                core->console <<
                    " required=" << ms->requiredParamCount() <<
                    " optional=" << (ms->param_count() - ms->requiredParamCount()) << "\n";
            })
    }

    CodegenLIR::~CodegenLIR() {
        cleanup();
    }

    void CodegenLIR::cleanup()
    {
        finddef_cache_builder.cleanup();
        LirHelper::cleanup();
    }

    #ifdef AVMPLUS_MAC_CARBON
    int CodegenLIR::setjmpAddress = 0;

    extern "C" int __setjmp();

    asm int CodegenLIR::setjmpDummy(jmp_buf buf)
    {
        b __setjmp;
    }

    void CodegenLIR::setjmpInit()
    {
        // CodeWarrior defies all reasonable efforts to get
        // the address of __vec_setjmp.  So, we resort to
        // a crude hack: We'll search the actual code
        // of setjmpDummy for the branch instruction.
        if (setjmpAddress == 0)
        {
            setjmpAddress = *((int*)&setjmpDummy);
        }
    }
    #endif

    LIns* CodegenLIR::atomToNativeRep(Traits* t, LIns* atom)
    {
        return atomToNative(bt(t), atom);
    }

    LIns* LirHelper::atomToNative(BuiltinType bt, LIns* atom)
    {
        switch (bt)
        {
        case BUILTIN_any:
        case BUILTIN_object:
        case BUILTIN_void:
            return atom;

        case BUILTIN_number:
            if (atom->isImmP())
                return lirout->insImmD(AvmCore::number_d((Atom)atom->immP()));
            else
                return callIns(FUNCTIONID(number_d), 1, atom);

        case BUILTIN_int:
            if (atom->isImmP())
                return InsConst(AvmCore::integer_i((Atom)atom->immP()));
            else
                return callIns(FUNCTIONID(integer_i), 1, atom);

        case BUILTIN_uint:
            if (atom->isImmP())
                return InsConst(AvmCore::integer_u((Atom)atom->immP()));
            else
                return callIns(FUNCTIONID(integer_u), 1, atom);

        case BUILTIN_boolean:
            if (atom->isImmI())
                return InsConst((int32_t)atomGetBoolean((Atom)atom->immP()));
            else
                return p2i(rshup(atom, 3));

        default:
            // pointer type
            if (atom->isImmP())
                return InsConstPtr(atomPtr((Atom)atom->immP()));
            else
                return andp(atom, ~7);
        }

#ifdef __GNUC__
        return 0;// satisfy GCC, although we should never get here
#endif
    }

    bool isNullable(Traits* t) {
        BuiltinType bt = Traits::getBuiltinType(t);
        return bt != BUILTIN_int && bt != BUILTIN_uint && bt != BUILTIN_boolean && bt != BUILTIN_number;
    }

    /**
     * Eliminates redundant loads within a block, and tracks the nullability of pointers
     * within blocks and across edges.  CodegenLIR will inform VarTracker that a
     * pointer is not null by calling setNotNull(ptr, type) either when the Verifier's
     * FrameState.Value is not null, in localGetp(), or after a null check in emitNullCheck().
     *
     * Within a block, we track nullability of references to instructions; when references
     * are copied, we know the copies are not null.
     *
     * At block boundaries, different values may flow together, so we track nullability
     * in variable slots instead of specific instruction references.
     */
    class VarTracker: public LirWriter
    {
        Allocator &alloc;               // Allocator for the lifetime of this filter
        LIns** varTracker;              // remembers the last value stored in each var
        LIns** tagTracker;              // remembers the last tag stored in each var
        HashMap<LIns*, bool> *checked;  // pointers we know are not null.
        nanojit::BitSet *notnull;       // stack locations we know are not null
        LIns* vars;                     // LIns that defines the vars[] array
        LIns* tags;                     // LIns that defines the tags[] array
        int nvar;                       // this method's frame size.

        // false after an unconditional control flow instruction (jump, throw, return),
        // true from the start and after we start a block via trackLabel()
        bool reachable;

#ifdef DEBUGGER
        bool haveDebugger;              // true if debugger is currently enabled
#else
        static const bool haveDebugger = false;
#endif
#ifdef AVMPLUS_VERBOSE
        bool verbose;                   // true when generating verbose output
#else
        static const bool verbose = false;
#endif

    public:
        VarTracker(MethodInfo* info, Allocator& alloc, LirWriter *out, int nvar)
            : LirWriter(out), alloc(alloc),
              vars(NULL), tags(NULL), nvar(nvar), reachable(true)
#ifdef DEBUGGER
            , haveDebugger(info->pool()->core->debugger() != NULL)
#endif
#ifdef AVMPLUS_VERBOSE
            , verbose(info->pool()->isVerbose(VB_jit))
#endif
        {
            (void) info; // suppress warning if !DEBUGGER && !AVMPLUS_VERBOSE
            varTracker = new (alloc) LIns*[nvar];
            tagTracker = new (alloc) LIns*[nvar];
            checked = new (alloc) HashMap<LIns*,bool>(alloc, nvar);
            notnull = new (alloc) nanojit::BitSet(alloc, nvar);
            clearState();
        }

        void init(LIns *vars, LIns* tags) {
            this->vars = vars;
            this->tags = tags;
        }

        void setNotNull(LIns* ins, Traits* t) {
            if (isNullable(t))
                checked->put(ins, true);
        }

        bool isNotNull(LIns* ins) {
            return checked->containsKey(ins);
        }

        void initNotNull(const FrameState* state) {
            syncNotNull(notnull, state);
        }

        // We're at the start of an AS3 basic block; syncronize our
        // notnull bits for that block with ones from the verifier.
        void syncNotNull(nanojit::BitSet* bits, const FrameState* state) {
            int scopeTop = state->verifier->scopeBase + state->scopeDepth;
            int stackBase = state->verifier->stackBase;
            int stackTop = stackBase + state->stackDepth;
            if (state->targetOfBackwardsBranch) {
                // clear any bits that are not set in verifier state
                for (int i=0, n=nvar; i < n; i++) {
                    const Value& v = state->value(i);
                    bool stateNotNull = v.notNull && isNullable(v.traits);
                    if (!stateNotNull || (i >= scopeTop && i < stackBase) || (i >= stackTop))
                        bits->clear(i);
                    else
                        bits->set(i);
                }
                printNotNull(bits, "loop label");
            } else {
                // set any bits that are set in verifier state
                for (int i=0, n=nvar; i < n; i++) {
                    const Value& v = state->value(i);
                    bool stateNotNull = v.notNull && isNullable(v.traits);
                    if ((i >= scopeTop && i < stackBase) || (i >= stackTop)) {
                        bits->clear(i);
                    } else if (stateNotNull) {
                        bits->set(i);
                        if (varTracker[i])
                            checked->put(varTracker[i], true);
                    }
                }
                printNotNull(bits, "forward label");
            }
        }

        // model a control flow edge by merging our current state with the
        // state saved at the target.  Used for forward branches and exception
        // edges.
        void trackForwardEdge(CodegenLabel& target) {
            AvmAssert(target.labelIns == NULL);  // illegal to call trackEdge on backedge
            for (int i=0, n=nvar; i < n; i++) {
                if (varTracker[i]) {
                    if (checked->containsKey(varTracker[i]))
                        notnull->set(i);
                    else
                        AvmAssert(!notnull->get(i));
                }
            }
            if (!target.notnull) {
                //printf("save state\n");
                target.notnull = new (alloc) nanojit::BitSet(alloc, nvar);
                target.notnull->setFrom(*notnull);
            } else {
                // target.notnull &= notnull
                for (int i=0, n=nvar; i < n; i++)
                    if (!notnull->get(i))
                        target.notnull->clear(i);
            }
        }

#ifdef AVMPLUS_VERBOSE
        void printNotNull(nanojit::BitSet* bits, const char* title) {
            if (verbose) {
                if (bits) {
                    printf("%s notnull = ", title);
                    for (int i=0, n=nvar; i < n; i++)
                        if (bits->get(i))
                            printf("%d ", i);
                    printf("\n");
                } else {
                    printf("%s notnull = null\n", title);
                }
            }
        }
#else
        void printNotNull(nanojit::BitSet*, const char*)
        {}
#endif

#ifdef DEBUG
        void checkBackEdge(CodegenLabel& target, const FrameState* state) {
            AvmAssert(target.labelIns != NULL);
            if (target.notnull) {
                printNotNull(notnull, "current");
                printNotNull(target.notnull, "target");
                int scopeTop = state->verifier->scopeBase + state->scopeDepth;
                int stackBase = state->verifier->stackBase;
                int stackTop = stackBase + state->stackDepth;
                // make sure our notnull bits at the target of the backedge were safe.
                for (int i=0, n=nvar; i < n; i++) {
                    if ((i >= scopeTop && i < stackBase) || i >= stackTop)
                        continue; // skip empty locations
                    if (!isNullable(state->value(i).traits))
                        continue; // skip non-nullable types in current state
                    //  current   target    assert(!target || current)
                    //  -------   ------    ------
                    //  false     false     true
                    //  false     true      false (assertion fires)
                    //  true      false     true
                    //  true      true      true
                    bool currentNotNull = (varTracker[i] ? isNotNull(varTracker[i]) : false) || notnull->get(i);
                    AvmAssert(!target.notnull->get(i) || currentNotNull);
                }
            }
        }
#else
        void checkBackEdge(CodegenLabel&, const FrameState*)
        {}
#endif

        // starts a new block.  if the new label is reachable from here,
        // merge our state with it.  then initialize from the new merged state.
        void trackLabel(CodegenLabel& label, const FrameState* state) {
            if (reachable)
                trackForwardEdge(label); // model the fall-through path as an edge
            clearState();
            label.labelIns = out->ins0(LIR_label);
            // load state saved at label
            if (label.notnull) {
                syncNotNull(label.notnull, state);
                notnull->setFrom(*label.notnull);
                printNotNull(notnull, "merged label");
            } else {
                syncNotNull(notnull, state);
                printNotNull(notnull, "first-time label");
            }
            reachable = true;
        }

        // Clear the var and tag expression states, but do not clear the nullability
        // state.  Called around debugger safe points to ensure that we reload values
        // that are possibly modified by the debugger.  Clearing the nullability state
        // correctly must be done at the verifier level, and at that level, it must always
        // be done or never be done (can't be conditional on debugging).
        // FIXME: bug 544238: clearing only the var state has questionable validity
        void clearVarState() {
            VMPI_memset(varTracker, 0, nvar*sizeof(LInsp));
            VMPI_memset(tagTracker, 0, nvar*sizeof(LInsp));
        }

        // clear all nullability and var/tag tracking state at branch targets
        void clearState() {
            clearVarState();
            checked->clear();
            notnull->reset();
        }

        REALLY_INLINE int varOffsetToIndex(int offset) {
            AvmAssert(IS_ALIGNED(offset, 8));
            return offset >> 3;
        }

        // keep track of the value stored in var d and update notnull
        void trackVarStore(LIns *value, int i) {
            varTracker[i] = value;
            if (checked->containsKey(value))
                notnull->set(i);
            else
                notnull->clear(i);
        }

        // keep track of the tag stored in var i.
        void trackTagStore(LIns *value, int i) {
            tagTracker[i] = value;
        }

        // The first time we see a load from variable i, remember it,
        // and if we know that slot is nonnull, add the load instruction to the nonnull set.
        void trackVarLoad(LIns* value, int i) {
            varTracker[i] = value;
            if (notnull->get(i))
                checked->put(value, true);
        }

        // first time we read a tag for variable i, remember it.
        void trackTagLoad(LIns* value, int i) {
            tagTracker[i] = value;
        }

        // monitor loads emitted by the LIR generator, track access to vars and tags
        LIns *insLoad(LOpcode op, LIns *base, int32_t d, AccSet accSet) {
            if (base == vars) {
                int i = varOffsetToIndex(d);
                LIns *val = varTracker[i];
                if (!val) {
                    val = out->insLoad(op, base, d, accSet);
                    trackVarLoad(val, i);
                }
                return val;
            }
            if (base == tags) {
                int i = d; // 1 byte per tag
                LIns *tag = tagTracker[i];
                if (!tag) {
                    tag = out->insLoad(op, base, d, accSet);
                    trackTagLoad(tag, i);
                }
                return tag;
            }
            return out->insLoad(op, base, d, accSet);
        }

        // monitor all stores emitted by LIR generator, update our tracking state
        // when we see stores to vars or tags.
        LIns *insStore(LOpcode op, LIns *value, LIns *base, int32_t d, AccSet accSet) {
            if (base == vars)
                trackVarStore(value, varOffsetToIndex(d));
            else if (base == tags)
                trackTagStore(value, d);
            return out->insStore(op, value, base, d, accSet);
        }

        // we expect the frontend to use CodegenLabels and call trackLabel for all
        // LIR label creation.  Assert to prevent unknown label generation.
        LIns *ins0(LOpcode op) {
            AvmAssert(op != LIR_label); // trackState must be called directly to generate a label.
            return out->ins0(op);
        }

        // set reachable = false after return instructions
        LIns* ins1(LOpcode op, LIns* a) {
            if (isRetOpcode(op))
                reachable = false;
            return out->ins1(op, a);
        }

        // set reachable = false after unconditional jumps
        LIns *insBranch(LOpcode v, LInsp cond, LInsp to) {
            if (v == LIR_j)
                reachable = false;
            return out->insBranch(v, cond, to);
        }

        // set reachable = false after LIR_jtbl which has explicit targets for all cases
        LIns *insJtbl(LIns* index, uint32_t size) {
            reachable = false;
            return out->insJtbl(index, size);
        }

        // returns true for functions that are known to always throw an exception.
        bool alwaysThrows(const CallInfo* call)
        {
            return call == FUNCTIONID(throwAtom) ||
                call == FUNCTIONID(npe) ||
                call == FUNCTIONID(mop_rangeCheckFailed) ||
                call == FUNCTIONID(handleInterruptMethodEnv);
        }

        // assume any non-pure function can throw an exception, and that pure functions cannot.
        bool canThrow(const CallInfo* call)
        {
            return !call->_isPure;
        }

        // if debugging is attached, clear our tracking state when calling side-effect
        // fucntions, which are effectively debugger safe points.
        // also set reachable = false if the function is known to always throw, and never return.
        LIns *insCall(const CallInfo *call, LInsp args[]) {
            if (haveDebugger && canThrow(call))
                clearVarState(); // debugger might have modified locals, so make sure we reload after call.
            if (alwaysThrows(call))
                reachable = false;
            return out->insCall(call, args);
        }
    };

    LIns* CodegenLIR::localGet(int i) {
#ifdef DEBUG
        const Value& v = state->value(i);
        AvmAssert((v.sst_mask == (1 << SST_int32) && v.traits == INT_TYPE) ||
                  (v.sst_mask == (1 << SST_uint32) && v.traits == UINT_TYPE) ||
                  (v.sst_mask == (1 << SST_bool32) && v.traits == BOOLEAN_TYPE));
#endif
        return lirout->insLoad(LIR_ldi, vars, i*8, ACC_VARS);
    }

    LIns* CodegenLIR::localGetf(int i) {
#ifdef DEBUG
        const Value& v = state->value(i);
        AvmAssert(v.sst_mask == (1<<SST_double) && v.traits == NUMBER_TYPE);
#endif
        return lirout->insLoad(LIR_ldd, vars, i*8, ACC_VARS);
    }

    // load a pointer-sized var, and update null tracking state if the verifier
    // informs us that it is not null via FrameState.value.
    LIns* CodegenLIR::localGetp(int i)
    {
        const Value& v = state->value(i);
        LIns* ins;
        if (exactlyOneBit(v.sst_mask)) {
            // pointer or atom
            AvmAssert(!(v.sst_mask == (1 << SST_int32) && v.traits == INT_TYPE) &&
                      !(v.sst_mask == (1 << SST_uint32) && v.traits == UINT_TYPE) &&
                      !(v.sst_mask == (1 << SST_bool32) && v.traits == BOOLEAN_TYPE) &&
                      !(v.sst_mask == (1 << SST_double) && v.traits == NUMBER_TYPE));
            ins = lirout->insLoad(LIR_ldp, vars, i*8, ACC_VARS);
        } else {
            // more than one representation is possible: convert to atom using tag found at runtime.
            AvmAssert(bt(v.traits) == BUILTIN_any || bt(v.traits) == BUILTIN_object);
            LIns* tag = lirout->insLoad(LIR_lduc2ui, tags, i, ACC_TAGS);
            LIns* varAddr = leaIns(i*8, vars);
            ins = callIns(FUNCTIONID(makeatom), 3, coreAddr, varAddr, tag);
        }
        if (v.notNull)
            varTracker->setNotNull(ins, v.traits);
        return ins;
    }

    LIns* CodegenLIR::callIns(const CallInfo *ci, uint32_t argc, ...)
    {
        const byte* code = state->verifier->code_pos;
        const byte* pc = code + state->pc;

        // each exception edge needs to be tracked to make sure we correctly
        // model the notnull state at the starts of catch blocks.  Treat any function
        // with side effects as possibly throwing an exception.

        // we must Ignore catch blocks that the verifier has determined are not reachable,
        // because we emit a call to debugExit (modeled as possibly throwing) as part of
        // OP_returnvoid/returnvalue, which ordinarily don't throw.
        if (!ci->_isPure && pc >= state->verifier->tryFrom && pc < state->verifier->tryTo) {
            // inside exception handler range, calling a function that could throw
            ExceptionHandlerTable *exTable = info->abc_exceptions();
            for (int i=0, n=exTable->exception_count; i < n; i++) {
                ExceptionHandler* handler = &exTable->exceptions[i];
                if (pc >= code + handler->from && pc < code + handler->to && state->verifier->hasFrameState(handler->target))
                    varTracker->trackForwardEdge(getCodegenLabel(handler->target));
            }
        }

        va_list ap;
        va_start(ap, argc);
        LIns* ins = LirHelper::vcallIns(ci, argc, ap);
        va_end(ap);
        return ins;
    }

    void LirHelper::emitStart(Allocator& alloc, LirBuffer *lirbuf, LirWriter* &lirout) {
        (void)alloc; (void)lirbuf;
        debug_only(
            // catch problems before they hit the writer pipeline
            lirout = validate1 = new (alloc) ValidateWriter(lirout, lirbuf->printer, "emitStart");
        )
        lirout->ins0(LIR_start);

        // create params for saved regs -- processor specific
        for (int i=0; i < NumSavedRegs; i++) {
            LIns *p = lirout->insParam(i, 1); (void) p;
            verbose_only(if (lirbuf->printer)
                lirbuf->printer->lirNameMap->addName(p, regNames[Assembler::savedRegs[i]]);)
        }
    }

    /**
     * Specializer holds specializations of certian calls into inline code sequences.
     * this could just as easily be a standalone filter instead of subclassing
     * ExprFilter, however having one less pipeline stage saves 5% of verify
     * time for esc (2000 methods).  when/if this subclassing becomes painful
     * then a separate stage is waranted.
     */
    class Specializer: public ExprFilter
    {
        const nanojit::Config& config;
    public:
        Specializer(LirWriter *out, const nanojit::Config& config) : ExprFilter(out), config(config)
        {}

        bool isPromote(LOpcode op) {
            return op == LIR_ui2d || op == LIR_i2d;
        }

        LIns *imm2Int(LIns* imm) {
            // return LIns* if we can fit the constant into a i32
            if (imm->isImmI())
                ; // just use imm
            else if (imm->isImmD()) {
                double val = imm->immD();
                double cvt = (int)val;
                if (val == 0 || val == cvt)
                    imm = out->insImmI((int32_t)cvt);
                else
                    imm = 0; // can't convert
            } else {
                imm = 0; // non-imm
            }
            return imm;
        }

        LIns *insCall(const CallInfo *call, LInsp args[]) {
            if (call == FUNCTIONID(integer_d)) {
                LIns *v = args[0];
                LOpcode op = v->opcode();
                if (isPromote(op))
                    return v->oprnd1();
                if (op == LIR_addd || op == LIR_subd || op == LIR_muld) {
                    LIns *a = v->oprnd1();
                    LIns *b = v->oprnd2();
                    a = isPromote(a->opcode()) ? a->oprnd1() : imm2Int(a);
                    b = isPromote(b->opcode()) ? b->oprnd1() : imm2Int(b);
                    if (a && b)
                        return out->ins2(arithOpcodeD2I(op), a, b);
                }
#ifdef AVMPLUS_64BIT
                else if (op == LIR_immq) {
                    // const fold
                    return insImmI(AvmCore::integer_d(v->immD()));
                }
#endif
            }

			/*
			if (call == FUNCTIONID(integer_d)) {
				LIns* doubleValue = args[0];
				return out->ins1(LIR_d2i, doubleValue);
			}
			*/

            SSE2_ONLY(if(config.i386_sse2) {
                if (call == FUNCTIONID(integer_d))
                    call = FUNCTIONID(integer_d_sse2);
                else if (call == FUNCTIONID(doubleToAtom))
                    call = FUNCTIONID(doubleToAtom_sse2);
            })

            return out->insCall(call, args);
        }
    };

#if defined(DEBUGGER) && defined(_DEBUG)
    // The AS debugger requires type information for variables contained
    // in the AS frame regions (i.e. 'vars').  In the interpreter this
    // is not an issues, since the region contains box values (i.e. Atoms)
    // and so the type information is self-contained.  With the jit, this is
    // not the case, and thus 'tags' is used to track the type of each
    // variable in 'vars'.
    // This filter watches stores to 'vars' and 'tags' and upon encountering
    // debugline (i.e. place where debugger can halt), it ensures that the
    // tags entry is consistent with the value stored in 'vars'
    class DebuggerCheck : public LirWriter
    {
        AvmCore* core;
        LInsp *varTracker;
        LInsp *tagTracker;
        LIns *vars;
        LIns *tags;
        int nvar;
    public:
        DebuggerCheck(AvmCore* core, Allocator& alloc, LirWriter *out, int nvar)
            : LirWriter(out), core(core), vars(NULL), tags(NULL), nvar(nvar)
        {
            varTracker = new (alloc) LInsp[nvar];
            tagTracker = new (alloc) LInsp[nvar];
            clearState();
        }

        void init(LIns *vars, LIns *tags) {
            this->vars = vars;
            this->tags = tags;
        }

        void trackVarStore(LIns *value, int d) {
            AvmAssert(IS_ALIGNED(d, 8));
            int i = d >> 3;
            if (i >= nvar)
                return;
            varTracker[i] = value;
        }

        void trackTagStore(LIns *value, int d) {
            int i = d; // 1 byte per tag
            if (i >= nvar)
                return;
            tagTracker[i] = value;
            checkValid(i);
            tagTracker[i] = (LIns*)((intptr_t)value|1); // lower bit => validated;
        }

        void clearState() {
            VMPI_memset(varTracker, 0, nvar * sizeof(LInsp));
            VMPI_memset(tagTracker, 0, nvar * sizeof(LInsp));
        }

        void checkValid(int i) {
            // @pre tagTracker[i] has been previously filled
            LIns* val = varTracker[i];
            LIns* tra = tagTracker[i];
            NanoAssert(val && tra);

            switch ((SlotStorageType) tra->immI()) {
            case SST_double:
                AvmAssert(val->isQorD());
                break;
            case SST_int32:
            case SST_uint32:
            case SST_bool32:
                AvmAssert(val->isI());
                break;
            default:
                AvmAssert(val->isP());
                break;
            }
        }

        void checkState() {
            for (int i=0; i < this->nvar; i++) {
                LIns* val = varTracker[i];
                LIns* tra = tagTracker[i];
                AvmAssert(val && tra);

                // isValid should have already been called on everything
                AvmAssert(((intptr_t)tra&0x1) == 1);
            }
        }

        LIns *insCall(const CallInfo *call, LInsp args[]) {
            if (call == FUNCTIONID(debugLine))
                checkState();
            return out->insCall(call,args);
        }

        LIns *insStore(LOpcode op, LIns *value, LIns *base, int32_t d, AccSet accSet) {
            if (base == vars)
                trackVarStore(value, d);
            else if (base == tags)
                trackTagStore(value, d);
            return out->insStore(op, value, base, d, accSet);
        }

    };
#endif

    // writer for the prolog.  instructions written here dominate code in the
    // body, and so are added to the body's CseFilter
    class PrologWriter : public LirWriter
    {
    public:
        LIns* lastIns;
        LIns* env_scope;
        LIns* env_vtable;
        LIns* env_abcenv;
        LIns* env_domainenv;
        LIns* env_toplevel;

        PrologWriter(LirWriter *out):
            LirWriter(out),
            lastIns(NULL),
            env_scope(NULL),
            env_vtable(NULL),
            env_abcenv(NULL),
            env_domainenv(NULL),
            env_toplevel(NULL)
        {}

        virtual LInsp ins0(LOpcode v) {
            return lastIns = out->ins0(v);
        }
        virtual LInsp ins1(LOpcode v, LIns* a) {
            return lastIns = out->ins1(v, a);
        }
        virtual LInsp ins2(LOpcode v, LIns* a, LIns* b) {
            return lastIns = out->ins2(v, a, b);
        }
        virtual LInsp ins3(LOpcode v, LIns* a, LIns* b, LIns* c) {
            return lastIns = out->ins3(v, a, b, c);
        }
        virtual LInsp insGuard(LOpcode v, LIns *c, GuardRecord *gr) {
            return lastIns = out->insGuard(v, c, gr);
        }
        virtual LInsp insBranch(LOpcode v, LInsp condition, LInsp to) {
            return lastIns = out->insBranch(v, condition, to);
        }
        // arg: 0=first, 1=second, ...
        // kind: 0=arg 1=saved-reg
        virtual LInsp insParam(int32_t arg, int32_t kind) {
            return lastIns = out->insParam(arg, kind);
        }
        virtual LInsp insImmI(int32_t imm) {
            return lastIns = out->insImmI(imm);
        }
#ifdef AVMPLUS_64BIT
        virtual LInsp insImmQ(uint64_t imm) {
            return lastIns = out->insImmQ(imm);
        }
#endif
        virtual LInsp insImmD(double d) {
            return lastIns = out->insImmD(d);
        }
        virtual LInsp insLoad(LOpcode op, LIns* base, int32_t d, AccSet accSet) {
            return lastIns = out->insLoad(op, base, d, accSet);
        }
        virtual LInsp insStore(LOpcode op, LIns* value, LIns* base, int32_t d, AccSet accSet) {
            return lastIns = out->insStore(op, value, base, d, accSet);
        }
        // args[] is in reverse order, ie. args[0] holds the rightmost arg.
        virtual LInsp insCall(const CallInfo *call, LInsp args[]) {
            return lastIns = out->insCall(call, args);
        }
        virtual LInsp insAlloc(int32_t size) {
            NanoAssert(size != 0);
            return lastIns = out->insAlloc(size);
        }
        virtual LInsp insJtbl(LIns* index, uint32_t size) {
            return lastIns = out->insJtbl(index, size);
        }

    };

    // Generate the prolog for a function with this C++ signature:
    //
    //    <return-type> f(MethodEnv* env, int argc, void* args)
    //
    // argc is the number of arguments, not counting the receiver
    // (aka "this"). args points to the arguments in memory:
    //
    //    [receiver] [arg1, arg2, ... ]
    //
    // the arguments in memory are typed according to the AS3 method
    // signature.  types * and Object are represented as Atom, and all
    // other types are native pointers or values.  return-type is whatever
    // the native type is for the AS3 return type; one of double, int32_t,
    // uint32_t, ScriptObject*, String*, Namespace*, Atom, or void.
    //
    // The stack frame layout of a jit-compiled function is determined by
    // the jit backend.  Stack-allocated structs are declared in LIR with
    // a LIR_allocp instruction.  Incoming parameters are declared with LIR_paramp
    // instructions, and any other local variables with function-body scope
    // and lifetime are declared with the expressions that compute them.
    // The backend will also allocate additional stack space for spilled values
    // and callee-saved registers.  The VM and LIR do not currently depend on how
    // the backend organizes the stack frame.
    //
    // Incoming parameters:
    //
    // env_param (LIR_paramp, MethodEnv*) is the incoming MethodEnv* parameter
    // that provides access to the environment for this function and all vm services.
    //
    // argc_param (LIR_paramp, int32_t) the # of arguments that follow.  Ignored
    // when the # of args is fixed, but otherwise used for optional arg processing
    // and/or creating the rest[] or arguments[] arrays for undeclared varargs.
    //
    // ap_param (LIR_paramp, uint32_t*) pointer to (argc+1) incoming arguments.
    // arguments are packed.  doubles are sizeof(double), everything else is sizeof(Atom).
    //
    // Distinguished locals:
    //
    // methodFrame (LIR_allocp, MethodFrame*) is the current MethodFrame.  in the prolog
    // we push this onto the call stack pointed to by AvmCore::currentMethodFrame, and
    // in the epilog we pop it back off.
    //
    // coreAddr (LIR_immi|LIR_immq) constant address of AvmCore*.  used in lots of places.
    // undefConst (LIR_immi|LIR_immq) constant value = undefinedAtom. used all over.
    //
    // vars (LIR_allocp) storage for ABC stack frame variables.  8 bytes per variable,
    // always, laid out according to ABC param/local var numbering.  The total number
    // is local_count + scope_depth + stack_depth, i.e. enough for the whole ABC frame.
    // values at any given point in the jit code are are represented according to the
    // statically known type of the variable at that point in the code.  (the type and
    // representation may change at different points.  verifier->frameState maintains
    // the known static types of variables.
    //
    // tags (LIR_allocp) SlotStorageType of each var in vars, one byte per variable.
    //
    // The contents of vars+tags are up-to-date at all labels and debugging safe points.
    // Inbetween those points, the contents are stale; the JIT optimizes away
    // stores and loads in straightline code.  Additional dead stores
    // are elided by deadvars_analyze() and deadvars_kill().
    //
    // Locals for Debugger use, only present when Debugger is in use:
    //
    // csn (LIR_allocp, CallStackNode).  extra information about this call frame
    // used by the debugger and also used for constructing human-readable stack traces.
    //
    // Locals for Exception-handling, only present when method has try/catch blocks:
    //
    // _save_eip (LIR_allocp, intptr_t) storage for the current ABC-based "pc", used by exception
    // handling to determine which catch blocks are in scope.  The value is an ABC
    // instruction offset, which is how catch handler records are indexed.
    //
    // _ef (LIR_allocp, ExceptionFrame) an instance of struct ExceptionFrame, including
    // a jmp_buf holding our setjmp() state, a pointer to the next outer ExceptionFrame,
    // and other junk.
    //
    // setjmpResult (LIR_call, int) result from calling setjmp; feeds a conditional branch
    // that surrounds the whole function body; logic to pick a catch handler and jump to it
    // is compiled after the function body.  if setjmp returns a nonzero result then we
    // jump forward, pick a catch block, then jump backwards to the catch block.
    //

    void CodegenLIR::writePrologue(const FrameState* state, const byte* /* pc */)
    {
        this->state = state;
        abcStart = state->verifier->code_pos;
        abcEnd   = abcStart + state->verifier->code_length;
        framesize = state->verifier->frameSize;

        frag = new (*lir_alloc) Fragment(abcStart verbose_only(, 0));
        LirBuffer *prolog_buf = frag->lirbuf = new (*lir_alloc) LirBuffer(*lir_alloc);
        prolog_buf->abi = ABI_CDECL;

        lirout = new (*alloc1) LirBufWriter(prolog_buf, core->config.njconfig);

        verbose_only(
            vbNames = 0;
            if (verbose()) {
                vbNames = new (*lir_alloc) LInsPrinter(*lir_alloc);
                vbNames->addrNameMap->addAddrRange(pool->core, sizeof(AvmCore), 0, "core");
                prolog_buf->printer = vbNames;
            }
        )
        debug_only(
            lirout = validate2 = new (*alloc1) ValidateWriter(lirout, prolog_buf->printer,
                                                  "writePrologue(prologue)");
        )
        verbose_only(
            vbWriter = 0;
            if (verbose())
                lirout = vbWriter = new (*alloc1) VerboseWriter(*alloc1, lirout, vbNames, &pool->codeMgr->log, "PROLOG");
        )
        prolog = new (*alloc1) PrologWriter(lirout);
        redirectWriter = lirout = new (*lir_alloc) LirWriter(prolog);
        CseFilter *csefilter = NULL;
        if (core->config.njconfig.cseopt)
            lirout = csefilter = new (*alloc1) CseFilter(lirout, *alloc1);
#if defined(NANOJIT_ARM)
        if (core->config.njconfig.soft_float)
            lirout = new (*alloc1) SoftFloatFilter(lirout);
#endif
        lirout = new (*alloc1) Specializer(lirout, core->config.njconfig);

        #ifdef DEBUGGER
        dbg_framesize = state->verifier->local_count + state->verifier->max_scope;
        #ifdef DEBUG
        DebuggerCheck *checker = NULL;
        if (haveDebugger) {
            checker = new (*alloc1) DebuggerCheck(core, *alloc1, lirout, dbg_framesize);
            lirout = checker;
        }
        #endif // DEBUG
        #endif // DEBUGGER

        emitStart(*alloc1, prolog_buf, lirout);

        // add the VarTracker filter last because we want it to be first in line.
        lirout = varTracker = new (*alloc1) VarTracker(info, *alloc1, lirout, framesize);

        // last pc value that we generated a store for
        lastPcSave = 0;

        //
        // generate lir to define incoming method arguments.  Stack
        // frame allocations follow.
        //

        env_param = lirout->insParam(0, 0);
        argc_param = lirout->insParam(1, 0);
    #ifdef AVMPLUS_64BIT
        argc_param = lirout->ins1(LIR_q2i, argc_param);
    #endif
        ap_param = lirout->insParam(2, 0);

        // allocate room for a MethodFrame structure
        methodFrame = insAlloc(sizeof(MethodFrame));
        verbose_only( if (vbNames) {
            vbNames->lirNameMap->addName(methodFrame, "methodFrame");
        })

        coreAddr = InsConstPtr(core);

        // replicate MethodFrame ctor inline
		/*
		printf("Offset of currentMethodFrame %d\n", offsetof(AvmCore, currentMethodFrame));
		printf("Offset of envOrCodeContext %d\n", offsetof(MethodFrame, envOrCodeContext));
		printf("Offset of next %d\n", offsetof(MethodFrame, next));
		*/

        LIns* currentMethodFrame = loadIns(LIR_ldp, offsetof(AvmCore,currentMethodFrame), coreAddr, ACC_OTHER);
        // save env in MethodFrame.envOrCodeContext
        //     explicitly leave IS_EXPLICIT_CODECONTEXT clear
        //     explicitly leave DXNS_NOT_NULL clear, dxns is effectively null without doing the store here.
        stp(env_param, methodFrame, offsetof(MethodFrame,envOrCodeContext), ACC_OTHER);
        stp(currentMethodFrame, methodFrame, offsetof(MethodFrame,next), ACC_OTHER);
        stp(methodFrame, coreAddr, offsetof(AvmCore,currentMethodFrame), ACC_OTHER);
        #ifdef _DEBUG
        // poison MethodFrame.dxns since it's uninitialized by default
        //stp(InsConstPtr((void*)(uintptr_t)0xdeadbeef), methodFrame, offsetof(MethodFrame,dxns), ACC_OTHER);
        #endif

        // allocate room for our local variables
        vars = insAlloc(framesize * 8);         // sizeof(double)=8 bytes per var
        tags = insAlloc(framesize);             // one tag byte per var
        prolog_buf->sp = vars;
        varTracker->init(vars, tags);

        verbose_only( if (prolog_buf->printer) {
            prolog_buf->printer->lirNameMap->addName(env_param, "env");
            prolog_buf->printer->lirNameMap->addName(argc_param, "argc");
            prolog_buf->printer->lirNameMap->addName(ap_param, "ap");
            prolog_buf->printer->lirNameMap->addName(vars, "vars");
            prolog_buf->printer->lirNameMap->addName(tags, "tags");
        })

        debug_only(
            validate1->setSp(vars);
            validate1->setRp(tags);
            validate2->setSp(vars);
            validate2->setRp(tags);
        )

        // stack overflow check - use methodFrame address as comparison
		/*
        LIns *d = loadIns(LIR_ldp, offsetof(AvmCore, minstack), coreAddr, ACC_OTHER);
        LIns *c = binaryIns(LIR_ltup, methodFrame, d);
        CodegenLabel &begin_label = createLabel("begin");
        branchToLabel(LIR_jf, c, begin_label);
        callIns(FUNCTIONID(handleStackOverflowMethodEnv), 1, env_param);
        emitLabel(begin_label);
		*/

        // we emit the undefined constant here since we use it so often and
        // to ensure it dominates all uses.
        undefConst = InsConstAtom(undefinedAtom);

        // whether this sequence is interruptable or not.
        interruptable = ! info->isNonInterruptible();

        // then space for the exception frame, be safe if its an init stub
        if (info->hasExceptions()) {
            // [_save_eip][ExceptionFrame]
            // offsets of local vars, rel to current ESP
            _save_eip = insAlloc(sizeof(intptr_t));
            _ef       = insAlloc(sizeof(ExceptionFrame));
            verbose_only( if (vbNames) {
                vbNames->lirNameMap->addName(_save_eip, "_save_eip");
                vbNames->lirNameMap->addName(_ef, "_ef");
            })
        } else {
            _save_eip = NULL;
            _ef = NULL;
        }

        #ifdef DEBUGGER
        if (haveDebugger) {
            // tell the sanity checker about vars and tags
            debug_only( checker->init(vars, tags); )

            // Allocate space for the call stack
            csn = insAlloc(sizeof(CallStackNode));
            verbose_only( if (vbNames) {
                vbNames->lirNameMap->addName(csn, "csn");
            })
        }
        #endif

#ifdef DEBUG
        jit_sst = new (*alloc1) uint8_t[framesize];
        memset(jit_sst, 0, framesize);
#endif

        //
        // copy args to local frame
        //

        // copy required args, and initialize optional args.
        // this whole section only applies to functions that actually
        // have arguments.

        const int param_count = ms->param_count();
        const int optional_count = ms->optional_count();
        const int required_count = param_count - optional_count;

        LIns* apArg = ap_param;
        if (info->hasOptional())
        {
            // compute offset of first optional arg
            int offset = 0;
            for (int i=0, n=required_count; i <= n; i++) {
                offset += ms->paramTraitsBT(i) == BUILTIN_number ? sizeof(double) : sizeof(Atom);
            }

            // now copy the default optional values
            LIns* argcarg = argc_param;
            for (int i=0, n=optional_count; i < n; i++)
            {
                // first set the local[p+1] = defaultvalue
                int param = i + required_count; // 0..N
                int loc = param+1;

                LIns* defaultVal = InsConstAtom(ms->getDefaultValue(i));
                defaultVal = atomToNativeRep(loc, defaultVal);
                localSet(loc, defaultVal, state->value(loc).traits);

                // then generate: if (argc > p) local[p+1] = arg[p+1]
                LIns* cmp = binaryIns(LIR_lei, argcarg, InsConst(param));
                CodegenLabel& optional_label = createLabel("param_", i);
                branchToLabel(LIR_jt, cmp, optional_label); // will patch
                copyParam(loc, offset);
                emitLabel(optional_label);
            }
        }
        else
        {
            // !HAS_OPTIONAL
            AvmAssert(optional_count == 0);
        }

        // now set up the required args (we can ignore argc)
        // for (int i=0, n=param_count; i <= n; i++)
        //     framep[i] = argv[i];
        int offset = 0;
        for (int i=0, n=required_count; i <= n; i++)
            copyParam(i, offset);

        if (info->unboxThis())
        {
            localSet(0, atomToNativeRep(0, localGet(0)), state->value(0).traits);
        }

        int firstLocal = 1+param_count;

        // capture remaining args
        if (info->needRest())
        {
            //framep[info->param_count+1] = createRest(env, argv, argc);
            // use csop so if rest value never used, we don't bother creating array
            LIns* rest = callIns(FUNCTIONID(createRestHelper), 3,
                env_param, argc_param, apArg);
            localSet(firstLocal, rest, ARRAY_TYPE);
            firstLocal++;
        }
        else if (info->needArguments())
        {
            //framep[info->param_count+1] = createArguments(env, argv, argc);
            // use csop so if arguments never used, we don't create it
            LIns* arguments = callIns(FUNCTIONID(createArgumentsHelper), 3,
                env_param, argc_param, apArg);
            localSet(firstLocal, arguments, ARRAY_TYPE);
            firstLocal++;
        }

        // set remaining locals to undefined
        for (int i=firstLocal, n = state->verifier->local_count; i < n; i++) {
            AvmAssert(state->value(i).traits == NULL);
            localSet(i, undefConst, NULL); // void would be more precise
        }

        #ifdef DEBUGGER
        if (haveDebugger) {
            for (int i=state->verifier->scopeBase; i<state->verifier->scopeBase+state->verifier->max_scope; ++i) {
                localSet(i, undefConst, VOID_TYPE);
            }

            callIns(FUNCTIONID(debugEnter), 5,
                env_param,
                tags,
                csn,
                vars,
                info->hasExceptions() ? _save_eip : InsConstPtr(0)
                );
        }
        #endif // DEBUGGER

        varTracker->initNotNull(state);

        /// SWITCH PIPELINE FROM PROLOG TO BODY
        verbose_only( if (vbWriter) { vbWriter->flush();} )
        // we have written the prolog to prolog_buf, now create a new
        // LirBuffer to hold the body, and redirect further output to the body.
        LirBuffer *body_buf = new (*lir_alloc) LirBuffer(*lir_alloc);
        LirWriter *body = new (*alloc1) LirBufWriter(body_buf, core->config.njconfig);
        debug_only(
            body = validate3 = new (*alloc1) ValidateWriter(body, vbNames, "writePrologue(body)");
            validate3->setSp(vars);
            validate3->setRp(tags);
        )
        verbose_only(
            if (verbose()) {
                AvmAssert(vbNames != NULL);
                body_buf->printer = vbNames;
                body = vbWriter = new (*alloc1) VerboseWriter(*alloc1, body, vbNames, &pool->codeMgr->log);
            }
        )
        body->ins0(LIR_start);
        redirectWriter->out = body;
        /// END SWITCH CODE

        if (info->hasExceptions()) {
            // _ef.beginTry(core);
            callIns(FUNCTIONID(beginTry), 2, _ef, coreAddr);

            // Exception* setjmpResult = setjmp(_ef.jmpBuf);
            // ISSUE this needs to be a cdecl call
            LIns* jmpbuf = leaIns(offsetof(ExceptionFrame, jmpbuf), _ef);
            setjmpResult = callIns(FUNCTIONID(fsetjmp), 2, jmpbuf, InsConst(0));

            // if (setjmp() != 0) goto catch dispatcher, which we generate in the epilog.
            branchToLabel(LIR_jf, eqi0(setjmpResult), catch_label);
        }
        verbose_only( if (vbWriter) { vbWriter->flush();} )
    }

    void CodegenLIR::copyParam(int i, int& offset) {
        LIns* apArg = ap_param;
        Traits* type = ms->paramTraits(i);
        LIns *arg;
        switch (bt(type)) {
        case BUILTIN_number:
            arg = loadIns(LIR_ldd, offset, apArg, ACC_READONLY);
            offset += sizeof(double);
            break;
        case BUILTIN_int:
        case BUILTIN_uint:
        case BUILTIN_boolean:
            // in the args these are widened to intptr_t or uintptr_t, so truncate here.
            arg = p2i(loadIns(LIR_ldp, offset, apArg, ACC_READONLY));
            offset += sizeof(Atom);
            break;
        default:
            arg = loadIns(LIR_ldp, offset, apArg, ACC_READONLY);
            offset += sizeof(Atom);
            break;
        }
        localSet(i, arg, type);
    }

    void CodegenLIR::emitCopy(int src, int dest) {
        localSet(dest, localCopy(src), state->value(src).traits);
    }

    void CodegenLIR::emitGetscope(int scope_index, int dest)
    {
        Traits* t = info->declaringScope()->getScopeTraitsAt(scope_index);
        LIns* scope = loadEnvScope();
        LIns* scopeobj = loadIns(LIR_ldp, offsetof(ScopeChain,_scopes) + scope_index*sizeof(Atom), scope, ACC_READONLY);
        localSet(dest, atomToNativeRep(t, scopeobj), t);
    }

    void CodegenLIR::emitSwap(int i, int j) {
        LIns* t = localCopy(i);
        localSet(i, localCopy(j), state->value(j).traits);
        localSet(j, t, state->value(i).traits);
    }

    void CodegenLIR::emitKill(int i)
    {
        localSet(i, undefConst, NULL);
    }

    void CodegenLIR::writeBlockStart(const FrameState* state)
    {
        this->state = state;
        // get the saved label for our block start and tie it to this location
        CodegenLabel& label = getCodegenLabel(state->pc);
        emitLabel(label);
        emitSetPc();

#ifdef DEBUG
        memset(jit_sst, 0, framesize);
#endif

        // If this is a backwards branch, generate an interrupt check.
        // current verifier state, includes tack pointer.
        if (interruptable && core->config.interrupts && state->targetOfBackwardsBranch) {
            LIns* interrupted = loadIns(LIR_ldi, offsetof(AvmCore,interrupted), coreAddr, ACC_OTHER);
            LIns* cond = binaryIns(LIR_eqi, interrupted, InsConst(AvmCore::NotInterrupted));
            branchToLabel(LIR_jf, cond, interrupt_label);
        }
    }

    void CodegenLIR::writeOpcodeVerified(const FrameState* state, const byte*, AbcOpcode)
    {
        verbose_only( if (vbWriter) { vbWriter->flush();} )
#ifdef DEBUG
        this->state = NULL; // prevent access to stale state
        int scopeTop = state->verifier->scopeBase + state->scopeDepth;
        for (int i=0, n=state->sp()+1; i < n; i++) {
            if (i >= scopeTop && i < state->verifier->stackBase)
                continue;
            const Value& v = state->value(i);
            AvmAssert(!jit_sst[i] || jit_sst[i] == v.sst_mask);
        }
#else
        (void)state;
#endif
    }

    // this is a no-op for the JIT because we do all label patching in emitLabel().
    void CodegenLIR::writeFixExceptionsAndLabels(const FrameState*, const byte*)
    {}

    void CodegenLIR::write(const FrameState* state, const byte* pc, AbcOpcode opcode, Traits *type)
    {
      //AvmLog("CodegenLIR::write %x\n", opcode);
        this->state = state;
        emitSetPc();
        const byte* nextpc = pc;
        unsigned int imm30=0, imm30b=0;
        int imm8=0, imm24=0;
        AvmCore::readOperands(nextpc, imm30, imm24, imm30b, imm8);
        int sp = state->sp();

        switch (opcode) {
        case OP_nop:
        case OP_pop:
        case OP_label:
            // do nothing
            break;
        case OP_getlocal0:
        case OP_getlocal1:
        case OP_getlocal2:
        case OP_getlocal3:
            imm30 = opcode-OP_getlocal0;
            // hack imm30 and fall through
        case OP_getlocal:
            emitCopy(imm30, sp+1);
            break;
        case OP_setlocal0:
        case OP_setlocal1:
        case OP_setlocal2:
        case OP_setlocal3:
            imm30 = opcode-OP_setlocal0;
            // hack imm30 and fall through
        case OP_setlocal:
            emitCopy(sp, imm30);
            break;
        case OP_pushtrue:
            AvmAssert(type == BOOLEAN_TYPE);
            emitIntConst(sp+1, 1, type);
            break;
        case OP_pushfalse:
            AvmAssert(type == BOOLEAN_TYPE);
            emitIntConst(sp+1, 0, type);
            break;
        case OP_pushnull:
            AvmAssert(type == NULL_TYPE);
            emitPtrConst(sp+1, 0, type);
            break;
        case OP_pushundefined:
            AvmAssert(type == VOID_TYPE);
            emitPtrConst(sp+1, (void*)undefinedAtom, type);
            break;
        case OP_pushshort:
            AvmAssert(type == INT_TYPE);
            emitIntConst(sp+1, (signed short)imm30, type);
            break;
        case OP_pushbyte:
            AvmAssert(type == INT_TYPE);
            emitIntConst(sp+1, (signed char)imm8, type);
            break;
        case OP_pushstring:
            AvmAssert(type == STRING_TYPE);
            emitPtrConst(sp+1, pool->getString(imm30), type);
            break;
        case OP_pushnamespace:
            AvmAssert(type == NAMESPACE_TYPE);
            emitPtrConst(sp+1, pool->cpool_ns[imm30], type);
            break;
        case OP_pushint:
            AvmAssert(type == INT_TYPE);
            emitIntConst(sp+1, pool->cpool_int[imm30], type);
            break;
        case OP_pushuint:
            AvmAssert(type == UINT_TYPE);
            emitIntConst(sp+1, pool->cpool_uint[imm30], type);
            break;
        case OP_pushdouble:
            AvmAssert(type == NUMBER_TYPE);
            emitDoubleConst(sp+1, pool->cpool_double[imm30]);
            break;
        case OP_pushnan:
            AvmAssert(type == NUMBER_TYPE);
            emitDoubleConst(sp+1, (double*)atomPtr(core->kNaN));
            break;
        case OP_lookupswitch:
            emit(opcode, state->pc+imm24, imm30b /*count*/);
            break;
        case OP_throw:
        case OP_returnvalue:
        case OP_returnvoid:
            emit(opcode, sp);
            break;
        case OP_debugfile:
        {
#if defined(DEBUGGER) || defined(VTUNE)
            #ifdef VTUNE
            const bool do_emit = true;
            #else
            const bool do_emit = haveDebugger;
            #endif
            Stringp str = pool->getString(imm30);  // assume been checked already
            if(do_emit) emit(opcode, (uintptr)str);
#endif
            break;
        }
        case OP_dxns:
        {
            Stringp str = pool->getString(imm30);  // assume been checked already
            emit(opcode, (uintptr)str);
            break;
        }
        case OP_dxnslate:
            // codgen will call intern on the input atom.
            emit(opcode, sp);
            break;
        case OP_kill:
            emitKill(imm30);
            break;
        case OP_inclocal:
        case OP_declocal:
            emit(opcode, imm30, opcode==OP_inclocal ? 1 : -1, NUMBER_TYPE);
            break;
        case OP_inclocal_i:
        case OP_declocal_i:
            emit(opcode, imm30, opcode==OP_inclocal_i ? 1 : -1, INT_TYPE);
            break;
        case OP_lessthan:
        case OP_greaterthan:
        case OP_lessequals:
        case OP_greaterequals:
            emit(opcode, 0, 0, BOOLEAN_TYPE);
            break;

        case OP_getdescendants:
        {
            const Multiname *name = pool->precomputedMultiname(imm30);
            emit(opcode, (uintptr)name, 0, NULL);
            break;
        }

        case OP_checkfilter:
            emit(opcode, sp, 0, NULL);
            break;

        case OP_deleteproperty:
        {
            const Multiname *name = pool->precomputedMultiname(imm30);
            emit(opcode, (uintptr)name, 0, BOOLEAN_TYPE);
            break;
        }

        case OP_astype:
        {
            const Multiname *name = pool->precomputedMultiname(imm30);
            Traits *t = pool->getTraits(*name, state->verifier->getToplevel(this));
            emit(OP_astype, (uintptr)t, sp, t && t->isMachineType() ? OBJECT_TYPE : t);
            break;
        }
        case OP_astypelate:
        {
            const Value& classValue = state->peek(1); // rhs - class
            Traits* ct = classValue.traits;
            Traits* t = NULL;
            if (ct && (t=ct->itraits) != 0)
                if (t->isMachineType())
                    t = OBJECT_TYPE;
            emit(opcode, 0, 0, t);
            break;
        }

        case OP_coerce:
        case OP_coerce_b:
        case OP_convert_b:
        case OP_coerce_o:
        case OP_coerce_a:
        case OP_convert_i:
        case OP_coerce_i:
        case OP_convert_u:
        case OP_coerce_u:
        case OP_convert_d:
        case OP_coerce_d:
        case OP_coerce_s:
            AvmAssert(
                    (opcode == OP_coerce    && type != NULL) ||
                    (opcode == OP_coerce_b  && type == BOOLEAN_TYPE) ||
                    (opcode == OP_convert_b && type == BOOLEAN_TYPE) ||
                    (opcode == OP_coerce_o  && type == OBJECT_TYPE) ||
                    (opcode == OP_coerce_a  && type == NULL) ||
                    (opcode == OP_convert_i && type == INT_TYPE) ||
                    (opcode == OP_coerce_i  && type == INT_TYPE) ||
                    (opcode == OP_convert_u && type == UINT_TYPE) ||
                    (opcode == OP_coerce_u  && type == UINT_TYPE) ||
                    (opcode == OP_convert_d && type == NUMBER_TYPE) ||
                    (opcode == OP_coerce_d  && type == NUMBER_TYPE) ||
                    (opcode == OP_coerce_s  && type == STRING_TYPE));
            emitCoerce(sp, type);
            break;

        case OP_istype:
        {
            // expects a CONSTANT_Multiname cpool index
            // used when operator "is" RHS is a compile-time type constant
            //sp[0] = istype(sp[0], itraits);
            const Multiname *name = pool->precomputedMultiname(imm30);
            Traits* itraits = pool->getTraits(*name, state->verifier->getToplevel(this));
            LIns* obj = loadAtomRep(sp);
            LIns* out = callIns(FUNCTIONID(istype), 2, obj, InsConstPtr(itraits));
            localSet(sp, out, BOOLEAN_TYPE);
            break;
        }

        case OP_istypelate:
        {
            // null check for the type value T in (x is T).  This also preserves
            // any side effects from loading T, even if we end up inlining T.itraits() as a const.
            Traits* class_type = state->value(sp).traits;
            emitCheckNull(localCopy(sp), class_type);
            LIns* obj = loadAtomRep(sp-1);
            LIns* istype_result;
            if (class_type && class_type->base == CLASS_TYPE) {
                // (x is T) where T is a class object: get T.itraits as constant.
                istype_result = callIns(FUNCTIONID(istype), 2, obj, InsConstPtr(class_type->itraits));
            } else {
                // RHS is unknown, call general istype
                istype_result = callIns(FUNCTIONID(istypelate), 3, env_param, obj, loadAtomRep(sp));
            }
            localSet(sp-1, istype_result, BOOLEAN_TYPE);
            break;
        }

        case OP_convert_o:
            // NOTE check null has already been done
            break;

        case OP_applytype:
            // * is ok for the type, as Vector classes have no statics
            // when we implement type parameters fully, we should do something here.
            emit(opcode, imm30/*argc*/, 0, NULL);
            break;

        case OP_newobject:
            emit(opcode, imm30, 0, OBJECT_TYPE);
            break;

        case OP_newarray:
            emit(opcode, imm30, 0, ARRAY_TYPE);
            break;

        case OP_newactivation:
            emit(opcode, 0, 0, info->activationTraits());
            break;

        case OP_newcatch:
        {
            ExceptionHandler* handler = &info->abc_exceptions()->exceptions[imm30];
            emit(opcode, 0, 0, handler->scopeTraits);
            break;
        }

        case OP_popscope:
            if (haveDebugger)
                emitKill(ms->local_count()/*scopeBase*/ + state->scopeDepth);
            break;

        case OP_getslot:
        {
            const Value& obj = state->peek(1);
            int index = imm30-1;
            Traits* slotTraits = obj.traits ? obj.traits->getTraitsBindings()->getSlotTraits(index) : NULL;
            emitGetslot(index, sp, slotTraits);
            break;
        }

        case OP_setslot:
            emitSetslot(OP_setslot, imm30-1, sp-1);
            break;

        case OP_dup:
            emitCopy(sp, sp+1);
            break;

        case OP_swap:
            emitSwap(sp, sp-1);
            break;

        case OP_equals:
        case OP_strictequals:
        case OP_instanceof:
        case OP_in:
            emit(opcode, 0, 0, BOOLEAN_TYPE);
            break;

        case OP_not:
            AvmAssert(type == BOOLEAN_TYPE);
            emit(opcode, sp, 0, type);
            break;

        case OP_modulo:
        case OP_subtract:
        case OP_divide:
        case OP_multiply:
            emit(opcode, 0, 0, NUMBER_TYPE);
            break;

        case OP_increment:
        case OP_decrement:
            emit(opcode, sp, opcode == OP_increment ? 1 : -1, NUMBER_TYPE);
            break;

        case OP_increment_i:
        case OP_decrement_i:
            emit(opcode, sp, opcode == OP_increment_i ? 1 : -1, INT_TYPE);
            break;

        case OP_add_i:
        case OP_subtract_i:
        case OP_multiply_i:
            emit(opcode, 0, 0, INT_TYPE);
            break;

        case OP_negate:
            emit(opcode, sp, 0, NUMBER_TYPE);
            break;

        case OP_negate_i:
            emit(opcode, sp, 0, INT_TYPE);
            break;

        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
            emit(opcode, 0, 0, INT_TYPE);
            break;

        case OP_lshift:
        case OP_rshift:
            emit(opcode, 0, 0, INT_TYPE);
            break;

        case OP_urshift:
            emit(opcode, 0, 0, UINT_TYPE);
            break;

        case OP_bitnot:
            emit(opcode, sp, 0, INT_TYPE);
            break;

        case OP_typeof:
            emit(opcode, sp, 0, STRING_TYPE);
            break;

        case OP_debugline:
        {
            #if defined(DEBUGGER) || defined(VTUNE)
            #ifdef VTUNE
            const bool do_emit = true;
            #else
            const bool do_emit = haveDebugger;
            #endif
            // we actually do generate code for these, in debugger mode
            if (do_emit) emit(opcode, imm30);
            #endif
            break;
        }
        case OP_nextvalue:
        case OP_nextname:
            emit(opcode, 0, 0, NULL);
            break;

        case OP_hasnext:
            emit(opcode, 0, 0, INT_TYPE);
            break;

        case OP_hasnext2:
            emit(opcode, imm30, imm30b, BOOLEAN_TYPE);
            break;

        // sign extends
        case OP_sxi1:
        case OP_sxi8:
        case OP_sxi16:
            emit(opcode, sp, 0, INT_TYPE);
            break;

        // loads
        case OP_lix8:
        case OP_lix16:
        case OP_li8:
        case OP_li16:
        case OP_li32:
        case OP_lf32:
        case OP_lf64:
        {
            Traits* result = (opcode == OP_lf32 || opcode == OP_lf64) ? NUMBER_TYPE : INT_TYPE;
            emit(opcode, sp, 0, result);
            break;
        }

        // stores
        case OP_si8:
        case OP_si16:
        case OP_si32:
        case OP_sf32:
        case OP_sf64:
        {
            emit(opcode, 0, 0, VOID_TYPE);
            break;
        }

        case OP_getglobalscope:
            emitGetGlobalScope(sp+1);
            break;

        case OP_add:
        {
            const Value& val1 = state->value(sp-1);
            const Value& val2 = state->value(sp);
            if ((val1.traits == STRING_TYPE && val1.notNull) || (val2.traits == STRING_TYPE && val2.notNull)) {
                // string add
                AvmAssert(type == STRING_TYPE);
                LIns* lhs = convertToString(sp-1);
                LIns* rhs = convertToString(sp);
                LIns* out = callIns(FUNCTIONID(concatStrings), 3, coreAddr, lhs, rhs);
                localSet(sp-1,  out, type);
            } else if (val1.traits && val2.traits && val1.traits->isNumeric() && val2.traits->isNumeric()) {
                // numeric add
                AvmAssert(type == NUMBER_TYPE);
                LIns* num1 = coerceToNumber(sp-1);
                LIns* num2 = coerceToNumber(sp);
                localSet(sp-1, binaryIns(LIR_addd, num1, num2), type);
            } else {
                // any other add
                AvmAssert(type == OBJECT_TYPE);
                LIns* lhs = loadAtomRep(sp-1);
                LIns* rhs = loadAtomRep(sp);
                LIns* out = callIns(FUNCTIONID(op_add), 3, coreAddr, lhs, rhs);
                localSet(sp-1, atomToNativeRep(type, out), type);
                break;
            }
            break;
        }

        case OP_convert_s:
            localSet(sp, convertToString(sp), STRING_TYPE);
            break;

        case OP_esc_xelem:
        case OP_esc_xattr:
            emit(opcode, sp, 0, STRING_TYPE);
            break;

        case OP_debug:
            // ignored
            break;

        default:
            AvmAssertMsg(false, "unhandled opcode in CodegenLIR::write()");
            break;
        }
    }

    // coerce parameter types, starting at firstArg.
    void CodegenLIR::coerceArgs(MethodSignaturep mms, int argc, int firstArg)
    {
        int sp = state->sp();
        for (int arg = argc, n = 1; arg >= firstArg; arg--, n++) {
            Traits* target = (arg <= mms->param_count()) ? mms->paramTraits(arg) : NULL;
            int index = sp - (n - 1);
            emitCoerce(index, target);
        }
    }

    // Coerce parameter types, but not receiver.  The object in the reciever
    // position is the class or function object and we haven't called newInstance yet.
    // In this case, emitCall() will generate a call to newInstance, producing the
    // new object, then call its init function with the coerced arguments.
    void CodegenLIR::emitConstructCall(intptr_t method_id, int argc, LIns* ctor, Traits* ctraits)
    {
        Traits* itraits = ctraits->itraits;
        MethodInfo* m = itraits->init;
        MethodSignaturep mms = m->getMethodSignature();
        AvmAssert(mms->argcOk(argc)); // caller must check this before early binding to ctor

        coerceArgs(mms, argc, 1);
        emitCall(OP_construct, method_id, argc, ctor, ctraits, itraits, mms);
    }

    /**
     * emitCoerceCall is used when the jit finds an opportunity to early bind that the
     * verifier did not.  It does the coersions using the signature of the callee, and
     * does not mutate FrameState.  (using the combination of
     * Verifier::emitCoerceArgs() and writeMethodCall(), from within the jit, is being
     * expressly avoided because we don't want the JIT to mutate FrameState.
     */
    void CodegenLIR::emitCoerceCall(AbcOpcode opcode, intptr_t method_id, int argc, MethodInfo* m)
    {
        AvmAssert(state->value(state->sp() - argc).notNull);  // make sure null check happened
        AvmAssert(opcode != OP_construct);
        MethodSignaturep mms = m->getMethodSignature();

        // should we check this sooner and not early bind?  assert here?
        if (!mms->argcOk(argc))
            state->verifier->verifyFailed(kWrongArgumentCountError, core->toErrorString(m), core->toErrorString(mms->requiredParamCount()), core->toErrorString(argc));

        coerceArgs(mms, argc, 0);
        emitCall(opcode, method_id, argc, mms->returnTraits(), mms);
    }

    void CodegenLIR::emitGetGlobalScope(int dest)
    {
        const ScopeTypeChain* scope = info->declaringScope();
        int captured_depth = scope->size;
        if (captured_depth > 0)
        {
            // enclosing scope
            emitGetscope(0, dest);
        }
        else
        {
            // local scope
            AvmAssert(state->scopeDepth > 0); // verifier checked.
            emitCopy(state->verifier->scopeBase, dest);
        }
    }

    void CodegenLIR::writeOp1(const FrameState* state, const byte *, AbcOpcode opcode, uint32_t opd1, Traits *type)
    {
        this->state = state;
        emitSetPc();
        switch (opcode) {
        case OP_iflt:
        case OP_ifle:
        case OP_ifnlt:
        case OP_ifnle:
        case OP_ifgt:
        case OP_ifge:
        case OP_ifngt:
        case OP_ifnge:
        case OP_ifeq:
        case OP_ifstricteq:
        case OP_ifne:
        case OP_ifstrictne:
        {
            int32_t offset = (int32_t) opd1;
            int lhs = state->sp()-1;
            emitIf(opcode, state->pc+4/*size*/+offset, lhs, lhs+1);
            break;
        }
        case OP_iftrue:
        case OP_iffalse:
        {
            int32_t offset = (int32_t) opd1;
            int sp = state->sp();
            emitIf(opcode, state->pc+4/*size*/+offset, sp, 0);
            break;
        }
        case OP_jump:
        {
            int32_t offset = (int32_t) opd1;
            emit(opcode, state->pc+4/*size*/+offset);
            break;
        }
        case OP_getslot:
            emitGetslot(opd1, state->sp(), type);
            break;
        case OP_getglobalslot: {
            int32_t dest_index = state->sp(); // verifier already incremented it
            uint32_t slot = opd1;
            emitGetGlobalScope(dest_index);
            emitGetslot(slot, dest_index, type /* slot type */);
            break;
        }
        case OP_setglobalslot:
            emitSetslot(OP_setglobalslot, opd1, 0 /* computed or ignored */);
            break;
        case OP_call:
            emit(opcode, opd1 /*argc*/, 0, NULL);
            break;

        case OP_construct:
        {
            const uint32_t argc = opd1;
            int ctor_index = state->sp() - argc;
            Traits* ctraits = state->value(ctor_index).traits;
            LIns* ctor = localCopy(ctor_index);
            emitConstruct(argc, ctor, ctraits);
            break;
        }
        case OP_getouterscope:
            emitGetscope(opd1, state->sp()+1);
            break;
        case OP_getscopeobject:
            emitCopy(opd1+state->verifier->scopeBase, state->sp()+1);
            break;
        case OP_newfunction:
            AvmAssert(pool->getMethodInfo(opd1)->declaringTraits() == type);
            emit(opcode, opd1, state->sp()+1, type);
            break;
        case OP_pushscope:
        case OP_pushwith:
            emitCopy(state->sp(), opd1);
            break;
        case OP_findpropstrict:
        case OP_findproperty:
        {
            const Multiname *name = pool->precomputedMultiname(opd1);
            emit(opcode, (uintptr)name, 0, OBJECT_TYPE);
            break;
        }
        case OP_findpropglobalstrict:
        {
            // NOTE opcode not supported, deoptimizing
            const Multiname *name = pool->precomputedMultiname(opd1);
            emit(OP_findpropstrict, (uintptr)name, 0, OBJECT_TYPE);
            break;
        }
        case OP_findpropglobal:
        {
            // NOTE opcode not supported, deoptimizing
            const Multiname *name = pool->precomputedMultiname(opd1);
            emit(OP_findproperty, (uintptr)name, 0, OBJECT_TYPE);
            break;
        }

        case OP_newclass:
        {
            Traits* ctraits = pool->getClassTraits(opd1);
            AvmAssert(ctraits == type);
            emit(opcode, (uintptr)(void*)ctraits, state->sp(), type);
            break;
        }

        case OP_finddef:
        {
            // opd1=name index
            // type=script->declaringTraits
            const Multiname *multiname = pool->precomputedMultiname(opd1);
            AvmAssert(multiname->isBinding());
            int32_t dest_index = state->sp() + 1;
            // This allocates a cache slot even if the finddef ultimately becomes dead.
            // As long as caches tend to be small compared to size of pool data and code,
            // filtering out dead cache lines isn't worth the complexity.
            LIns* slot = InsConst(finddef_cache_builder.allocateCacheSlot(opd1));
            LIns* out = callIns(FUNCTIONID(finddef_cache), 3, env_param, InsConstPtr(multiname), slot);
            localSet(dest_index, ptrToNativeRep(type, out), type);
            break;
        }

        default:
            // verifier has called writeOp1 improperly
            AvmAssert(false);
            break;
        }
    }

    LIns* CodegenLIR::coerceToString(int index)
    {
        const Value& value = state->value(index);
        Traits* in = value.traits;

        switch (bt(in)) {
        case BUILTIN_null:
        case BUILTIN_string:
            // fine to just load the pointer
            return localGetp(index);
        case BUILTIN_int:
            return callIns(FUNCTIONID(intToString), 2, coreAddr, localGet(index));
        case BUILTIN_uint:
            return callIns(FUNCTIONID(uintToString), 2, coreAddr, localGet(index));
        case BUILTIN_number:
            return callIns(FUNCTIONID(doubleToString), 2, coreAddr, localGetf(index));
        case BUILTIN_boolean: {
            // load "true" or "false" string constant from AvmCore.booleanStrings[]
            LIns *offset = binaryIns(LIR_lshp, i2p(localGet(index)), InsConst(PTR_SCALE));
            LIns *arr = InsConstPtr(&core->booleanStrings);
            return loadIns(LIR_ldp, 0, binaryIns(LIR_addp, arr, offset), ACC_READONLY);
        }
        default:
            if (value.notNull) {
                // not eligible for CSE, and we know it's not null/undefined
                return callIns(FUNCTIONID(string), 2, coreAddr, loadAtomRep(index));
            }
            return callIns(FUNCTIONID(coerce_s), 2, coreAddr, loadAtomRep(index));
        }
    }

    /** emit code for * -> Number conversion */
    LIns* CodegenLIR::coerceToNumber(int index)
    {
        const Value& value = state->value(index);
        Traits* in = value.traits;

        if (in && (in->isNumeric() || in == BOOLEAN_TYPE)) {
            return promoteNumberIns(in, index);
        } else {
            // * -> Number
            return callIns(FUNCTIONID(number), 1, loadAtomRep(index));
        }
    }

    LIns* CodegenLIR::convertToString(int index)
    {
        const Value& value = state->value(index);
        Traits* in = value.traits;
        Traits* stringType = STRING_TYPE;

        if (in != stringType || !value.notNull) {
            if (in && (value.notNull || in->isNumeric() || in == BOOLEAN_TYPE)) {
                // convert is the same as coerce
                return coerceToString(index);
            } else {
                // explicitly convert to string
                return callIns(FUNCTIONID(string), 2, coreAddr, loadAtomRep(index));
            }
        } else {
            // already String*
            return localGetp(index);
        }
    }

    void CodegenLIR::writeNip(const FrameState* state, const byte *)
    {
        this->state = state;
        emitSetPc();
        emitCopy(state->sp(), state->sp()-1);
    }

    void CodegenLIR::writeMethodCall(const FrameState* state, const byte *, AbcOpcode opcode, MethodInfo* m, uintptr_t disp_id, uint32_t argc, Traits *type)
    {
        this->state = state;
        emitSetPc();
        switch (opcode) {
        case OP_callproperty:
        case OP_callproplex:
        case OP_callpropvoid:
            AvmAssert(m->declaringTraits()->isInterface());
            emitTypedCall(OP_callinterface, disp_id /* iid */, argc, type, m);
            break;
        case OP_callmethod:
            emitTypedCall(OP_callmethod, disp_id, argc, type, m);
            break;
        default:
            AvmAssert(false);
            break;
        }
    }

    void CodegenLIR::writeOp2(const FrameState* state, const byte *pc, AbcOpcode opcode, uint32_t opd1, uint32_t opd2, Traits *type)
    {
        this->state = state;
        emitSetPc();
        int sp = state->sp();
        switch (opcode) {

        case OP_constructsuper:
            // opd1=unused, opd2=argc
            emitTypedCall(OP_constructsuper, 0, opd2, VOID_TYPE, info->declaringTraits()->base->init);
            break;

        case OP_setsuper:
        {
            const uint32_t index = opd1;
            const uint32_t n = opd2;
            Traits* base = type;
            int32_t ptrIndex = sp-(n-1);

            const Multiname* name = pool->precomputedMultiname(index);

            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(base, name);
            Traits* propType = state->verifier->readBinding(base, b);
            const TraitsBindingsp basetd = base->getTraitsBindings();

            if (AvmCore::isSlotBinding(b))
            {
                if (AvmCore::isVarBinding(b))
                {
                    int slot_id = AvmCore::bindingToSlotId(b);
                    LIns* value = coerceToType(sp, propType);
                    emitSetslot(OP_setslot, slot_id, ptrIndex, value);
                }
                // else, ignore write to readonly accessor
            }
            else
            if (AvmCore::isAccessorBinding(b))
            {
                if (AvmCore::hasSetterBinding(b))
                {
                    // Invoke the setter
                    int disp_id = AvmCore::bindingToSetterId(b);
                    MethodInfo *f = basetd->getMethod(disp_id);
                    emitCoerceCall(OP_callsuperid, disp_id, 1, f);
                }
                // else, ignore write to readonly accessor
            }
            else {
                emit(opcode, (uintptr)name);
            }
            break;
        }
        case OP_getsuper:
        {
            const uint32_t index = opd1;
            const uint32_t n = opd2;
            Traits* base = type;

            const Multiname* name = pool->precomputedMultiname(index);

            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(base, name);
            Traits* propType = state->verifier->readBinding(base, b);

            if (AvmCore::isSlotBinding(b))
            {
                int slot_id = AvmCore::bindingToSlotId(b);
                emitGetslot(slot_id, state->sp()-(n-1), propType);
            }
            else
            if (AvmCore::hasGetterBinding(b))
            {
                // Invoke the getter
                int disp_id = AvmCore::bindingToGetterId(b);
                const TraitsBindingsp basetd = base->getTraitsBindings();
                MethodInfo *f = basetd->getMethod(disp_id);
                emitCoerceCall(OP_callsuperid, disp_id, 0, f);
            }
            else {
                emit(opcode, (uintptr)name, 0, propType);
            }
            break;
        }
        case OP_callsuper:
        case OP_callsupervoid:
        {
            const uint32_t index = opd1;
            const uint32_t argc = opd2;
            Traits* base = type;
            const TraitsBindingsp basetd = base->getTraitsBindings();

            const Multiname *name = pool->precomputedMultiname(index);

            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(base, name);

            if (AvmCore::isMethodBinding(b))
            {
                int disp_id = AvmCore::bindingToMethodId(b);
                MethodInfo* m = basetd->getMethod(disp_id);
                emitCoerceCall(OP_callsuperid, disp_id, argc, m);
            }
            else {

                // TODO optimize other cases
                emit(opcode, (uintptr)name, argc, NULL);
            }

            break;
        }

        case OP_constructprop:
        {
            const uint32_t argc = opd2;
            const Multiname* name = pool->precomputedMultiname(opd1);

            const Value& obj = state->peek(argc+1); // object
            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(obj.traits, name);

            if (AvmCore::isSlotBinding(b))
            {
                int slot_id = AvmCore::bindingToSlotId(b);
                int ctor_index = state->sp() - argc;
                LIns* ctor = loadFromSlot(ctor_index, slot_id, type);
                emitConstruct(argc, ctor, type);
            }
            else
            {
                emit(opcode, (uintptr)name, argc, NULL);
            }
            break;
        }

        case OP_getproperty:
        {
            // NOTE opd2 is the stack offset to the reciever
            const Multiname* name = pool->precomputedMultiname(opd1);
            const Value& obj = state->peek(opd2); // object
            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(obj.traits, name);

            // early bind accessor
            if (AvmCore::hasGetterBinding(b))
            {
                // Invoke the getter
                int disp_id = AvmCore::bindingToGetterId(b);
                const TraitsBindingsp objtd = obj.traits->getTraitsBindings();
                MethodInfo *f = objtd->getMethod(disp_id);
                AvmAssert(f != NULL);

                if (!obj.traits->isInterface()) {
                    emitTypedCall(OP_callmethod, disp_id, 0, type, f);
                }
                else {
                    emitTypedCall(OP_callinterface, f->iid(), 0, type, f);
                }
                AvmAssert(type == f->getMethodSignature()->returnTraits());
            }
            else {
                emit(OP_getproperty, opd1, 0, type);
            }
            break;
        }

        case OP_setproperty:
        case OP_initproperty:
        {
            // opd2=n the stack offset to the reciever
            const Multiname *name = pool->precomputedMultiname(opd1);
            const Value& obj = state->peek(opd2); // object
            Toplevel* toplevel = state->verifier->getToplevel(this);
            Binding b = toplevel->getBinding(obj.traits, name);

            // early bind accessor
            if (AvmCore::hasSetterBinding(b))
            {
                // invoke the setter
                int disp_id = AvmCore::bindingToSetterId(b);
                const TraitsBindingsp objtd = obj.traits->getTraitsBindings();
                MethodInfo *f = objtd->getMethod(disp_id);
                AvmAssert(f != NULL);

                if (!obj.traits->isInterface()) {
                    emitTypedCall(OP_callmethod, disp_id, 1, type, f);
                }
                else {
                    emitTypedCall(OP_callinterface, f->iid(), 1, type, f);
                }
            }
            else {
				if (opcode == OP_initproperty) {
				}
                emit(opcode, (uintptr)name);
            }
            break;
        }

        case OP_setslot:
            emitSetslot(OP_setslot, opd1, opd2);
            break;

        case OP_abs_jump:
        {
            #ifdef AVMPLUS_64BIT
            const byte* nextpc = pc;
            unsigned int imm30=0, imm30b=0;
            int imm8=0, imm24=0;
            AvmCore::readOperands(nextpc, imm30, imm24, imm30b, imm8);
            const byte* new_pc = (const byte *) (uintptr(opd1) | (((uintptr) opd2) << 32));
            const byte* new_code_end = new_pc + AvmCore::readU32 (nextpc);
            #else
            (void)pc;
            const byte* new_pc = (const byte*) opd1;
            const byte* new_code_end = new_pc + opd2;
            #endif
            this->abcStart = new_pc;
            this->abcEnd = new_code_end;
            break;
        }

        case OP_callproperty:
        case OP_callproplex:
        case OP_callpropvoid:
        {
            emit(opcode, opd1, opd2, NULL);
            break;
        }

        case OP_callstatic: {
            uint32_t method_id = opd1;
            uint32_t argc = opd2;
            emitTypedCall(OP_callstatic, method_id, argc, type, pool->getMethodInfo(method_id));
            break;
        }

        default:
            AvmAssert(false);
            break;
        }
    }

    void CodegenLIR::emitIntConst(int index, int32_t c, Traits* type)
    {
        localSet(index, lirout->insImmI(c), type);
    }

    void CodegenLIR::emitPtrConst(int index, void* c, Traits* type)
    {
        localSet(index, lirout->insImmP(c), type);
    }

    void CodegenLIR::emitDoubleConst(int index, double* pd)
    {
        localSet(index, lirout->insImmD(*pd), NUMBER_TYPE);
    }

    void CodegenLIR::writeCoerce(const FrameState* state, uint32_t loc, Traits* result)
    {
        this->state = state;
        emitSetPc();
        emitCoerce(loc, result);
    }

    void CodegenLIR::emitCoerce(uint32_t loc, Traits* result)
    {
        localSet(loc, coerceToType(loc, result), result);
    }

    LIns* CodegenLIR::coerceToType(int loc, Traits* result)
    {
        const Value& value = state->value(loc);
        Traits* in = value.traits;
        LIns* expr;

        if (result == NULL)
        {
            // coerce to * is simple, we just save the atom rep.
            expr = loadAtomRep(loc);
        }
        else if (result == OBJECT_TYPE)
        {
            if (in == NULL || in == VOID_TYPE)
            {
                // value already boxed but we need to coerce undefined->null
                if (!value.notNull) {
                    // v == undefinedAtom ? nullObjectAtom : v;
                    LIns *v = localGetp(loc);
                    expr = choose(eqp(v, undefConst), nullObjectAtom, v);
                } else {
                    expr = loadAtomRep(loc);
                }
            }
            else
            {
                // value cannot be undefined so just box it
                expr = loadAtomRep(loc);
            }
        }
        else if (!result->isMachineType() && in == NULL_TYPE)
        {
            // it's fine to coerce null to a pointer type, just load the value
            expr = localGetp(loc);
        }
        else if (result == NUMBER_TYPE)
        {
            expr = coerceToNumber(loc);
        }
        else if (result == INT_TYPE)
        {
            if (in == UINT_TYPE || in == BOOLEAN_TYPE || in == INT_TYPE)
            {
                // just load the value
                expr = localGet(loc);
            }
            else if (in == NUMBER_TYPE)
            {
                // narrowing conversion number->int
                LIns* ins = localGetf(loc);
                expr = callIns(FUNCTIONID(integer_d), 1, ins);
            }
            else
            {
                // * -> int
                expr = callIns(FUNCTIONID(integer), 1, loadAtomRep(loc));
            }
        }
        else if (result == UINT_TYPE)
        {
            if (in == INT_TYPE || in == BOOLEAN_TYPE || in == UINT_TYPE)
            {
                // just load the value
                expr = localGet(loc);
            }
            else if (in == NUMBER_TYPE)
            {
                LIns* ins = localGetf(loc);
                expr = callIns(FUNCTIONID(integer_d), 1, ins);
            }
            else
            {
                // * -> uint
                expr = callIns(FUNCTIONID(toUInt32), 1, loadAtomRep(loc));
            }
        }
        else if (result == BOOLEAN_TYPE)
        {
            if (in == BOOLEAN_TYPE)
            {
                expr = localGet(loc);
            }
            else if (in == NUMBER_TYPE)
            {
                expr = callIns(FUNCTIONID(doubleToBool), 1, localGetf(loc));
            }
            else if (in == INT_TYPE || in == UINT_TYPE)
            {
                // int to bool: b = (i==0) == 0
                expr = eqi0(eqi0(localGet(loc)));
            }
            else if (in && !in->notDerivedObjectOrXML())
            {
                // ptr to bool: b = (p==0) == 0
                expr = eqi0(eqp0(localGetp(loc)));
            }
            else
            {
                // * -> Boolean
                expr = callIns(FUNCTIONID(boolean), 1, loadAtomRep(loc));
            }
        }
        else if (result == STRING_TYPE)
        {
            expr = coerceToString(loc);
        }
        else if (in && !in->isMachineType() && !result->isMachineType()
               && in != STRING_TYPE && in != NAMESPACE_TYPE)
        {
            if (!state->verifier->canAssign(result, in)) {
                // coerceobj_obj() is void, but we mustn't optimize it out; we only call it when required
                callIns(FUNCTIONID(coerceobj_obj), 3,
                    env_param, localGetp(loc), InsConstPtr(result));
            }
            // the input pointer has now been checked but it's still the same value.
            expr = localGetp(loc);
        }
        else if (!result->isMachineType() && result != NAMESPACE_TYPE)
        {
            // result is a ScriptObject based type.
            expr = downcast_obj(loadAtomRep(loc), env_param, result);
        }
        else if (result == NAMESPACE_TYPE && in == NAMESPACE_TYPE)
        {
            expr = localGetp(loc);
        }
        else
        {
            LIns* value = loadAtomRep(loc);
            // resultValue = coerce(caller_env, inputValue, traits)
            LIns* out = callIns(FUNCTIONID(coerce), 3,
                env_param, value, InsConstPtr(result));

            // store the result
            expr = atomToNativeRep(result, out);
        }
        return expr;
    }

    void CodegenLIR::writeCheckNull(const FrameState* state, uint32_t index)
    {
        this->state = state;
        emitSetPc();
		(void)index;
		//printf("Checking null");
        //emitCheckNull(localCopy(index), state->value(index).traits);
    }

    void CodegenLIR::emitCheckNull(LIns* ptr, Traits* t)
    {
        // The result is either unchanged or an exception is thrown, so
        // we don't save the result.  This is the null pointer check.
        if (!isNullable(t) || varTracker->isNotNull(ptr))
            return;
        _nvprof("nullcheck",1);
        if (valueStorageType(bt(t)) == SST_atom) {
            _nvprof("nullcheck atom", 1);
            callIns(FUNCTIONID(nullcheck), 2, env_param, ptr);  // checking atom for null or undefined (AvmCore::isNullOrUndefined())
        } else {
            _nvprof("nullcheck ptr", 1);
            branchToLabel(LIR_jt, eqp0(ptr), npe_label);
        }
        varTracker->setNotNull(ptr, t);
    }

    // Save our current PC location for the catch finder later.
    void CodegenLIR::emitSetPc()
    {
        // update bytecode ip if necessary
        if (_save_eip && lastPcSave != state->pc) {
            stp(InsConstPtr((void*)state->pc), _save_eip, 0, ACC_OTHER);
            lastPcSave = state->pc;
        }
    }

    void LirHelper::liveAlloc(LIns* alloc)
    {
        if (alloc->isop(LIR_allocp))
            livep(alloc);
    }

    // This is for VTable->createInstance which is called by OP_construct
    FUNCTION(CALL_INDIRECT, SIG3(V,P,P,P), createInstance)

#ifdef DEBUG
    /**
     * emitTypedCall is used when the Verifier has found an opportunity to early bind,
     * and has already coerced arguments from whatever native type is discovered, to
     * the required types.  emitTypedCall() then just double-checks (via assert) that
     * the arg types are already correct.
     */
    void CodegenLIR::emitTypedCall(AbcOpcode opcode, intptr_t method_id, int argc, Traits* result, MethodInfo* mi)
    {
        AvmAssert(opcode != OP_construct);
        AvmAssert(state->value(state->sp() - argc).notNull); // make sure null check happened

        MethodSignaturep ms = mi->getMethodSignature();
        AvmAssert(ms->argcOk(argc));
        int objDisp = state->sp() - argc;
        for (int arg = 0; arg <= argc && arg <= ms->param_count(); arg++)
            AvmAssert(state->verifier->canAssign(state->value(objDisp+arg).traits, ms->paramTraits(arg)));
        for (int arg = ms->param_count()+1; arg <= argc; arg++) {
            BuiltinType t = bt(state->value(objDisp+arg).traits);
            AvmAssert(valueStorageType(t) == SST_atom);
        }
        emitCall(opcode, method_id, argc, result, ms);
    }
#else
    REALLY_INLINE void CodegenLIR::emitTypedCall(AbcOpcode opcode, intptr_t method_id, int argc, Traits* result, MethodInfo* mi)
    {
        emitCall(opcode, method_id, argc, result, mi->getMethodSignature());
    }
#endif

    void CodegenLIR::emitCall(AbcOpcode opcode, intptr_t method_id, int argc, Traits* result, MethodSignaturep ms)
    {
        int objDisp = state->sp() - argc;
        LIns* obj = localCopy(objDisp);
        Traits* objType = state->value(objDisp).traits;
        emitCall(opcode, method_id, argc, obj, objType, result, ms);
    }

    void CodegenLIR::emitCall(AbcOpcode opcode, intptr_t method_id, int argc, LIns* obj, Traits* objType, Traits* result, MethodSignaturep ms)
    {
        int sp = state->sp();
        int dest = sp-argc;
        int objDisp = dest;

        LIns *method = NULL;
        LIns *iid = NULL;
        switch (opcode)
        {
        case OP_constructsuper:
        {
            // env->vtable->base->init->enter32v(argc, ...);
            LIns* vtable = loadEnvVTable();
            LIns* base = loadIns(LIR_ldp, offsetof(VTable,base), vtable, ACC_READONLY);
            method = loadIns(LIR_ldp, offsetof(VTable,init), base, ACC_READONLY);
            break;
        }
        case OP_callmethod:
        {
            // stack in:  obj arg1..N
            // stack out: result
            // sp[-argc] = callmethod(disp_id, argc, ...);
            // method_id is disp_id of virtual method
            LIns* vtable = loadVTable(obj, objType);
            method = loadIns(LIR_ldp, int32_t(offsetof(VTable,methods)+sizeof(MethodEnv*)*method_id), vtable, ACC_READONLY);
            break;
        }
        case OP_callsuperid:
        {
            // stack in: obj arg1..N
            // stack out: result
            // method_id is disp_id of super method
            LIns* declvtable = loadEnvVTable();
            LIns* basevtable = loadIns(LIR_ldp, offsetof(VTable, base), declvtable, ACC_READONLY);
            method = loadIns(LIR_ldp, int32_t(offsetof(VTable,methods)+sizeof(MethodEnv*)*method_id), basevtable, ACC_READONLY);
            break;
        }
        case OP_callstatic:
        {
            // stack in: obj arg1..N
            // stack out: result
            LIns* abcenv = loadEnvAbcEnv();
            method = loadIns(LIR_ldp, int32_t(offsetof(AbcEnv,m_methods)+sizeof(MethodEnv*)*method_id), abcenv, ACC_READONLY);
            break;
        }
        case OP_callinterface:
        {
            // method_id is pointer to interface method name (multiname)
            int index = int(method_id % VTable::IMT_SIZE);
            LIns* vtable = loadVTable(obj, objType);
            // note, could be MethodEnv* or ImtThunkEnv*
            method = loadIns(LIR_ldp, offsetof(VTable,imt)+sizeof(ImtThunkEnv*)*index, vtable, ACC_READONLY);
            iid = InsConstPtr((void*)method_id);
            break;
        }
        case OP_construct:
        {
            // stack in: ctor arg1..N
            // stack out: newinstance
            LIns* vtable = loadVTable(obj, objType);
            LIns* ivtable = loadIns(LIR_ldp, offsetof(VTable, ivtable), vtable, ACC_READONLY);
            method = loadIns(LIR_ldp, offsetof(VTable, init), ivtable, ACC_READONLY);
            LIns* createInstance = loadIns(LIR_ldp, offsetof(VTable, createInstance), ivtable, ACC_OTHER);
            obj = callIns(FUNCTIONID(createInstance), 3, createInstance, obj, ivtable);
            objType = result;
            // the call below to the init function is void; the expression result we want
            // is the new object, not the result from the init function.  save it now.
            localSet(dest, obj, result);
            break;
        }
        default:
            AvmAssert(false);
        }

        // store args for the call
        LIns* ap = insAlloc(sizeof(Atom)); // we will update this size, below
        int disp = 0;
        int pad = 0;

        int param_count = ms->param_count();
        // LIR_allocp of any size >= 8 is always 8-aligned.
        // if the first double arg would be unaligned, add padding to align it.
    #if !defined AVMPLUS_64BIT
        for (int i=0; i <= argc && i <= param_count; i++) {
            if (ms->paramTraits(i) == NUMBER_TYPE) {
                if ((disp&7) != 0) {
                    // this double would be unaligned, so add some padding
                    pad = 8-(disp&7); // should be 4
                }
                break;
            }
            else {
                disp += sizeof(Atom);
            }
        }
    #endif

        disp = pad;
        for (int i=0, index=objDisp; i <= argc; i++, index++) {
            Traits* paramType = i <= param_count ? ms->paramTraits(i) : NULL;
            LIns* v;
            switch (bt(paramType)) {
            case BUILTIN_number:
                v = (i == 0) ? obj : lirout->insLoad(LIR_ldd, vars, index*8, ACC_VARS);
                std(v, ap, disp, ACC_OTHER);
                disp += sizeof(double);
                break;
            case BUILTIN_int:
                v = (i == 0) ? obj : lirout->insLoad(LIR_ldi, vars, index*8, ACC_VARS);
                stp(i2p(v), ap, disp, ACC_OTHER);
                disp += sizeof(intptr_t);
                break;
            case BUILTIN_uint:
            case BUILTIN_boolean:
                v = (i == 0) ? obj : lirout->insLoad(LIR_ldi, vars, index*8, ACC_VARS);
                stp(ui2p(v), ap, disp, ACC_OTHER);
                disp += sizeof(uintptr_t);
                break;
            default:
                v = (i == 0) ? obj : lirout->insLoad(LIR_ldp, vars, index*8, ACC_VARS);
                stp(v, ap, disp, ACC_OTHER);
                disp += sizeof(void*);
                break;
            }
        }

        // patch the size to what we actually need
        ap->setSize(disp);

#ifdef VMCFG_METHODENV_IMPL32
        LIns* target = loadIns(LIR_ldp, offsetof(MethodEnvProcHolder,_implGPR), method, ACC_OTHER);
#else
        LIns* meth = loadIns(LIR_ldp, offsetof(MethodEnvProcHolder, method), method, ACC_OTHER);
        LIns* target = loadIns(LIR_ldp, offsetof(MethodInfoProcHolder, _implGPR), meth, ACC_OTHER);
#endif
        LIns* apAddr = leaIns(pad, ap);

        LIns *out;
        BuiltinType rbt = bt(result);
        if (!iid) {
            const CallInfo *fid;
            switch (rbt) {
            case BUILTIN_number:
                fid = FUNCTIONID(fcalli);
                break;
            case BUILTIN_int: case BUILTIN_uint: case BUILTIN_boolean:
                fid = FUNCTIONID(icalli);
                break;
            default:
                fid = FUNCTIONID(acalli);
                break;
            }
            out = callIns(fid, 4, target, method, InsConst(argc), apAddr);
        } else {
            const CallInfo *fid;
            switch (rbt) {
            case BUILTIN_number:
                fid = FUNCTIONID(fcallimt);
                break;
            case BUILTIN_int: case BUILTIN_uint: case BUILTIN_boolean:
                fid = FUNCTIONID(icallimt);
                break;
            default:
                fid = FUNCTIONID(acallimt);
                break;
            }
            out = callIns(fid, 5, target, method, InsConst(argc), apAddr, iid);
        }

        // ensure the stack-allocated args are live until after the call
        liveAlloc(ap);

        if (opcode != OP_constructsuper && opcode != OP_construct)
            localSet(dest, out, result);
    }

    LIns* CodegenLIR::loadFromSlot(int ptr_index, int slot, Traits* slotType)
    {
        Traits *t = state->value(ptr_index).traits;
        LIns *ptr = localGetp(ptr_index);
        AvmAssert(state->value(ptr_index).notNull);
        AvmAssert(isPointer((int)ptr_index)); // obj

        AvmAssert(t->isResolved());
        const TraitsBindingsp tb = t->getTraitsBindings();
        int offset = tb->getSlotOffset(slot);

        // get
        LOpcode op;

		BuiltinType builtinSlotType = bt(slotType);
		switch (builtinSlotType) {
        case BUILTIN_number:    op = LIR_ldd;   break;
        case BUILTIN_int:
        case BUILTIN_uint:
        case BUILTIN_boolean:   op = LIR_ldi;    break;
        default:                op = LIR_ldp;   break;
        }
        return loadIns(op, offset, ptr, ACC_OTHER);
    }

    void CodegenLIR::emitGetslot(int slot, int ptr_index, Traits *slotType)
    {
        localSet(ptr_index, loadFromSlot(ptr_index, slot, slotType), slotType);
    }

    void CodegenLIR::emitSetslot(AbcOpcode opcode, int slot, int ptr_index)
    {
        emitSetslot(opcode, slot, ptr_index, localCopy(state->sp()));
    }

    void CodegenLIR::emitSetslot(AbcOpcode opcode, int slot, int ptr_index, LIns* value)
    {
        Traits* t;
        LIns* ptr;

        if (opcode == OP_setslot)
        {
            t = state->value(ptr_index).traits;
            ptr = localGetp(ptr_index);
            AvmAssert(state->value(ptr_index).notNull);
            AvmAssert(isPointer((int)ptr_index)); // obj
        }
        else
        {
            // setglobalslot
            const ScopeTypeChain* scopeTypes = info->declaringScope();
            if (scopeTypes->size == 0)
            {
                // no captured scopes, so global is local scope 0
                ptr_index = state->verifier->scopeBase;
                t = state->value(ptr_index).traits;
                ptr = localGetp(ptr_index);
                AvmAssert(state->value(ptr_index).notNull);
                AvmAssert(isPointer((int)ptr_index)); // obj
            }
            else
            {
                // global is outer scope 0
                t = scopeTypes->getScopeTraitsAt(0);
                LIns* scope = loadEnvScope();
                LIns* scopeobj = loadIns(LIR_ldp, offsetof(ScopeChain,_scopes) + 0*sizeof(Atom), scope, ACC_OTHER);
                ptr = atomToNativeRep(t, scopeobj);
            }
        }

        AvmAssert(t->isResolved());
        const TraitsBindingsp tb = t->getTraitsBindings();
        int offset = tb->getSlotOffset(slot);

        LIns *unoffsetPtr = ptr;

        // if storing to a pointer-typed slot, inline a WB
        Traits* slotType = tb->getSlotTraits(slot);

        if (!slotType || !slotType->isMachineType() || slotType == OBJECT_TYPE)
        {
            // slot type is Atom (for *, Object) or RCObject* (String, Namespace, or other user types)
            const CallInfo *wbAddr = FUNCTIONID(privateWriteBarrierRC);
            if (slotType == NULL || slotType == OBJECT_TYPE) {
                // use fast atom wb
                wbAddr = FUNCTIONID(atomWriteBarrier);
            }
            callIns(wbAddr, 4,
                    InsConstPtr(core->GetGC()),
                    unoffsetPtr,
                    leaIns(offset, ptr),
                    value);
        }
        else if (slotType == NUMBER_TYPE) {
            // slot type is double or int
            std(value, ptr, offset, ACC_OTHER);
        } else {
            AvmAssert(slotType == INT_TYPE || slotType == UINT_TYPE || slotType == BOOLEAN_TYPE);
            sti(value, ptr, offset, ACC_OTHER);
        }
    }

    /**
     * emit a constructor call, or a late bound constructor call.
     * early binding is possible when we know the constructor (class) being
     * used, and we know that it doesn't use custom native instance initializer
     * code, as indicated by the itraits->hasCustomConstruct flag.
     */
    void CodegenLIR::emitConstruct(int argc, LIns* ctor, Traits* ctraits)
    {
        // attempt to early bind to constructor method.
        Traits* itraits = NULL;
        if (ctraits) {
            itraits = ctraits->itraits;
            if (itraits && !itraits->hasCustomConstruct) {
                // Cannot resolve signatures now because that could cause a premature verification failure,
                // one that should occur in the class's script-init.
                // If it's already resolved then we're good to go.
                if (itraits->init && itraits->init->isResolved() && itraits->init->getMethodSignature()->argcOk(argc)) {
                    emitCheckNull(ctor, ctraits);
                    emitConstructCall(0, argc, ctor, ctraits);
                    return;
                }
            }
        }

        // generic path, could not early bind to a constructor method
        // stack in: ctor-object arg1..N
        // sp[-argc] = construct(env, sp[-argc], argc, null, arg1..N)
        int ctor_index = state->sp() - argc;
        LIns* func = nativeToAtom(ctor, ctraits);
        LIns* args = storeAtomArgs(InsConstAtom(nullObjectAtom), argc, ctor_index+1);
        LIns* newobj = callIns(FUNCTIONID(op_construct), 4, env_param, func, InsConst(argc), args);
        liveAlloc(args);
        localSet(ctor_index, atomToNativeRep(itraits, newobj), itraits);
    }

    typedef const CallInfo *CallInfop;

    void CodegenLIR::emit(AbcOpcode opcode, uintptr op1, uintptr op2, Traits* result)
    {
        int sp = state->sp();

        switch (opcode)
        {
            // sign extends
            case OP_sxi1:
            case OP_sxi8:
            case OP_sxi16:
            {
                // straightforward shift based sign extension
                static const uint8_t kShiftAmt[3] = { 31, 24, 16 };
                int32_t index = (int32_t) op1;
                LIns* val = localGet(index);
                if ((opcode == OP_sxi8 && val->opcode() == LIR_ldc2i) ||
                    (opcode == OP_sxi16 && val->opcode() == LIR_lds2i))
                {
                    // if we are sign-extending the result of a load-and-sign-extend
                    // instruction, no need to do anything.
                    break;
                }
                LIns* sh = InsConst(kShiftAmt[opcode - OP_sxi1]);
                LIns* shl = binaryIns(LIR_lshi, val, sh);
                LIns* res = binaryIns(LIR_rshi, shl, sh);
                localSet(index, res, result);
                break;
            }

            // loads
            case OP_lix8:
            case OP_lix16:
            case OP_li8:
            case OP_li16:
            case OP_li32:
            {
                int32_t index = (int32_t) op1;
                LIns* mopAddr = localGet(index);
                const MopsInfo& mi = kMopsLoadInfo[opcode-OP_lix8];
            #ifdef VMCFG_MOPS_USE_EXPANDED_LOADSTORE_INT
                int32_t disp = 0;
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, &disp);
                LIns* i2 = loadIns(mi.op, disp, realAddr, ACC_OTHER);
            #else
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, NULL);
                LIns* i2 = callIns(mi.call, 1, realAddr);
            #endif
                localSet(index, i2, result);
                break;
            }

            case OP_lf32:
            case OP_lf64:
            {
                int32_t index = (int32_t) op1;
                LIns* mopAddr = localGet(index);
                const MopsInfo& mi = kMopsLoadInfo[opcode-OP_lix8];
            #ifdef VMCFG_MOPS_USE_EXPANDED_LOADSTORE_FP
                int32_t disp = 0;
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, &disp);
                LIns* i2 = loadIns(mi.op, disp, realAddr, ACC_OTHER);
            #else
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, NULL);
                LIns* i2 = callIns(mi.call, 1, realAddr);
            #endif
                localSet(index, i2, result);
                break;
            }

            // stores
            case OP_si8:
            case OP_si16:
            case OP_si32:
            {
                LIns* svalue = localGet(sp-1);
                LIns* mopAddr = localGet(sp);
                const MopsInfo& mi = kMopsStoreInfo[opcode-OP_si8];
            #ifdef VMCFG_MOPS_USE_EXPANDED_LOADSTORE_INT
                int32_t disp = 0;
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, &disp);
                lirout->insStore(mi.op, svalue, realAddr, disp, ACC_OTHER);
            #else
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, NULL);
                callIns(mi.call, 2, realAddr, svalue);
            #endif
                break;
            }

            case OP_sf32:
            case OP_sf64:
            {
                LIns* svalue = localGetf(sp-1);
                LIns* mopAddr = localGet(sp);
                const MopsInfo& mi = kMopsStoreInfo[opcode-OP_si8];
            #ifdef VMCFG_MOPS_USE_EXPANDED_LOADSTORE_FP
                int32_t disp = 0;
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, &disp);
                lirout->insStore(mi.op, svalue, realAddr, disp, ACC_OTHER);
            #else
                LIns* realAddr = mopAddrToRangeCheckedRealAddrAndDisp(mopAddr, mi.size, NULL);
                callIns(mi.call, 2, realAddr, svalue);
            #endif
                break;
            }

            case OP_jump:
            {
                // spill everything first
                int targetpc_off = (int)op1;

#ifdef DEBUGGER
                Sampler* s = core->get_sampler();
                if (s && s->sampling() && targetpc_off < state->pc)
                {
                    emitSampleCheck();
                }
#endif

                branchToAbcPos(LIR_j, 0, targetpc_off);
                break;
            }

            case OP_lookupswitch:
            {
                //int index = integer(*(sp--));
                //pc += readS24(index < readU16(pc+4) ?
                //  (pc+6+3*index) :    // matched case
                //  (pc+1));            // default
                int count = int(1 + op2);
                int targetpc_off = (int)op1;

                AvmAssert(state->value(sp).traits == INT_TYPE);
                AvmAssert(count >= 0);

                // Compute address of jump table
                const byte* pc = 4 + abcStart + state->pc;
                AvmCore::readU32(pc);  // skip count

                // Delete any trailing table entries that == default case (effective for asc output)
                while (count > 0 && targetpc_off == (state->pc + AvmCore::readS24(pc+3*(count-1))))
                    count--;

                if (count > 0) {
                    LIns* index = localGet(sp);
                    LIns* cmp = binaryIns(LIR_ltui, index, InsConst(count));
                    branchToAbcPos(LIR_jf, cmp, targetpc_off);

                    if (NJ_JTBL_SUPPORTED) {
                        // Backend supports LIR_jtbl for jump tables
                        LIns* jtbl = lirout->insJtbl(index, count);
                        for (int i=0; i < count; i++) {
                            int target = state->pc + AvmCore::readS24(pc+3*i);
                            patchLater(jtbl, target, i);
                        }
                    } else {
                        // Backend doesn't support jump tables, use cascading if's
                        for (int i=0; i < count; i++) {
                            int target = state->pc + AvmCore::readS24(pc+3*i);
                            branchToAbcPos(LIR_jt, binaryIns(LIR_eqi, index, InsConst(i)), target);
                        }
                    }
                }
                else {
                    // switch collapses into a single target
                    branchToAbcPos(LIR_j, 0, targetpc_off);
                }
                break;
            }

            case OP_returnvoid:
            case OP_returnvalue:
            {
                // ISSUE if a method has multiple returns this causes some bloat

                #ifdef DEBUGGER
                if (haveDebugger) {
                    callIns(FUNCTIONID(debugExit), 2,
                        env_param, csn);
                    // now we toast the cse and restore contents in order to
                    // ensure that any variable modifications made by the debugger
                    // will be pulled in.
                    //firstCse = ip;
                }
                #endif // DEBUGGER

                if (info->hasExceptions())
                {
                    // _ef.endTry();
                    callIns(FUNCTIONID(endTry), 1, _ef);
                }

                // replicate MethodFrame dtor inline -- must come after endTry call (if any)
                LIns* nextMethodFrame = loadIns(LIR_ldp, offsetof(MethodFrame,next), methodFrame, ACC_OTHER);
                stp(nextMethodFrame, coreAddr, offsetof(AvmCore,currentMethodFrame), ACC_OTHER);

                Traits* t = ms->returnTraits();
                LIns* retvalue;
                if (opcode == OP_returnvalue)
                {
                    // already coerced to required native type
                    // use localCopy() to sniff type and use appropriate load instruction
                    int32_t index = (int32_t) op1;
                    retvalue = localCopy(index);
                }
                else
                {
                    retvalue = undefConst;
                    if (t && t != VOID_TYPE)
                    {
                        // implicitly coerce undefined to the return type
                        retvalue = callIns(FUNCTIONID(coerce), 3,
                            env_param, retvalue, InsConstPtr(t));
                        retvalue = atomToNativeRep(t, retvalue);
                    }
                }
                switch (bt(t)) {
                case BUILTIN_number:
                    Ins(LIR_retd, retvalue);
                    break;
                case BUILTIN_int:
                    retp(i2p(retvalue));
                    break;
                case BUILTIN_uint:
                case BUILTIN_boolean:
                    retp(ui2p(retvalue));
                    break;
                default:
                    retp(retvalue);
                    break;
                }
                break;
            }

            case OP_typeof:
            {
                //sp[0] = typeof(sp[0]);
                int32_t index = (int32_t) op1;
                LIns* value = loadAtomRep(index);
                LIns* i3 = callIns(FUNCTIONID(typeof), 2,
                    coreAddr, value);
                AvmAssert(result == STRING_TYPE);
                localSet(index, i3, result);
                break;
            }

            case OP_not:
            {
                int32_t index = (int32_t) op1;
                AvmAssert(state->value(index).traits == BOOLEAN_TYPE && result == BOOLEAN_TYPE);
                LIns* value = localGet(index); // 0 or 1
                LIns* i3 = eqi0(value); // 1 or 0
                localSet(index, i3, result);
                break;
            }

            case OP_negate: {
                int32_t index = (int32_t) op1;
                localSet(index, Ins(LIR_negd, localGetf(index)),result);
                break;
            }

            case OP_negate_i: {
                //framep[op1] = -framep[op1]
                int32_t index = (int32_t) op1;
                AvmAssert(state->value(index).traits == INT_TYPE);
                localSet(index, Ins(LIR_negi, localGet(index)), result);
                break;
            }

            case OP_increment:
            case OP_decrement:
            case OP_inclocal:
            case OP_declocal: {
                int32_t index = (int32_t) op1;
                int32_t incr = (int32_t) op2; // 1 or -1
                localSet(index, binaryIns(LIR_addd, localGetf(index), i2dIns(InsConst(incr))), result);
                break;
            }

            case OP_inclocal_i:
            case OP_declocal_i:
            case OP_increment_i:
            case OP_decrement_i: {
                int32_t index = (int32_t) op1;
                int32_t incr = (int32_t) op2;
                AvmAssert(state->value(index).traits == INT_TYPE);
                localSet(index, binaryIns(LIR_addi, localGet(index), InsConst(incr)), result);
                break;
            }

            case OP_bitnot: {
                // *sp = core->intToAtom(~integer(*sp));
                int32_t index = (int32_t) op1;
                AvmAssert(state->value(index).traits == INT_TYPE);
                localSet(index, lirout->ins1(LIR_noti, localGet(index)), result);
                break;
            }

            case OP_modulo: {
                LIns* out = callIns(FUNCTIONID(mod), 2,
                    localGetf(sp-1), localGetf(sp));
                localSet(sp-1,  out, result);
                break;
            }

            case OP_divide:
            case OP_multiply:
            case OP_subtract: {
                LOpcode op;
                switch (opcode) {
                    default:
                    case OP_divide:     op = LIR_divd; break;
                    case OP_multiply:   op = LIR_muld; break;
                    case OP_subtract:   op = LIR_subd; break;
                }
                localSet(sp-1, binaryIns(op, localGetf(sp-1), localGetf(sp)), result);
                break;
            }

            case OP_subtract_i:
            case OP_add_i:
            case OP_multiply_i:
            case OP_lshift:
            case OP_rshift:
            case OP_urshift:
            case OP_bitand:
            case OP_bitor:
            case OP_bitxor:
            {
                LOpcode op;
                switch (opcode) {
                    default:
                    case OP_bitxor:     op = LIR_xori;  break;
                    case OP_bitor:      op = LIR_ori;   break;
                    case OP_bitand:     op = LIR_andi;  break;
                    case OP_urshift:    op = LIR_rshui; break;
                    case OP_rshift:     op = LIR_rshi;  break;
                    case OP_lshift:     op = LIR_lshi;  break;
                    case OP_multiply_i: op = LIR_muli;  break;
                    case OP_add_i:      op = LIR_addi;  break;
                    case OP_subtract_i: op = LIR_subi;  break;
                }
                LIns* lhs = localGet(sp-1);
                LIns* rhs = localGet(sp);
                LIns* out = binaryIns(op, lhs, rhs);
                localSet(sp-1, out, result);
                break;
            }

            case OP_throw:
            {
                //throwAtom(*sp--);
                int32_t index = (int32_t) op1;
                callIns(FUNCTIONID(throwAtom), 2, coreAddr, loadAtomRep(index));
                break;
            }

            case OP_getsuper:
            {
                // stack in: obj [ns [name]]
                // stack out: value
                // sp[0] = env->getsuper(sp[0], multiname)
                int objDisp = sp;
                LIns* multi = initMultiname((Multiname*)op1, objDisp);
                AvmAssert(state->value(objDisp).notNull);

                LIns* obj = loadAtomRep(objDisp);

                LIns* i3 = callIns(FUNCTIONID(getsuper), 3,
                    env_param, obj, multi);
                liveAlloc(multi);

                i3 = atomToNativeRep(result, i3);
                localSet(objDisp, i3, result);
                break;
            }

            case OP_setsuper:
            {
                // stack in: obj [ns [name]] value
                // stack out: nothing
                // core->setsuper(sp[-1], multiname, sp[0], env->vtable->base)
                int objDisp = sp-1;
                LIns* multi = initMultiname((Multiname*)op1, objDisp);
                AvmAssert(state->value(objDisp).notNull);

                LIns* obj = loadAtomRep(objDisp);
                LIns* value = loadAtomRep(sp);

                callIns(FUNCTIONID(setsuper), 4,
                    env_param, obj, multi, value);
                liveAlloc(multi);
                break;
            }

            case OP_nextname:
            case OP_nextvalue:
            {
                // sp[-1] = next[name|value](sp[-1], sp[0]);
                LIns* obj = loadAtomRep(sp-1);
                AvmAssert(state->value(sp).traits == INT_TYPE);
                LIns* index = localGet(sp);
                LIns* i1 = callIns((opcode == OP_nextname) ? FUNCTIONID(nextname) : FUNCTIONID(nextvalue), 3,
                                   env_param, obj, index);
                localSet(sp-1, atomToNativeRep(result, i1), result);
                break;
            }

            case OP_hasnext:
            {
                // sp[-1] = hasnext(sp[-1], sp[0]);
                LIns* obj = loadAtomRep(sp-1);
                AvmAssert(state->value(sp).traits == INT_TYPE);
                LIns* index = localGet(sp);
                LIns* i1 = callIns(FUNCTIONID(hasnext), 3,
                    env_param, obj, index);
                AvmAssert(result == INT_TYPE);
                localSet(sp-1, i1, result);
                break;
            }

            case OP_hasnext2:
            {
                // fixme - if obj is already Atom, or index is already int,
                // easier to directly reference space in vars.
                int32_t obj_index = (int32_t) op1;
                int32_t index_index = (int32_t) op2;
                LIns* obj = insAlloc(sizeof(Atom));
                LIns* index = insAlloc(sizeof(int32_t));
                stp(loadAtomRep(obj_index), obj, 0, ACC_STORE_ANY);       // Atom obj
                sti(localGet(index_index), index, 0, ACC_STORE_ANY);      // int32 index
                LIns* i1 = callIns(FUNCTIONID(hasnextproto), 3,
                                     env_param, obj, index);
                localSet(obj_index, loadIns(LIR_ldp, 0, obj, ACC_LOAD_ANY), OBJECT_TYPE);  // Atom obj
                localSet(index_index, loadIns(LIR_ldi, 0, index, ACC_LOAD_ANY), INT_TYPE); // int32 index
                AvmAssert(result == BOOLEAN_TYPE);
                localSet(sp+1, i1, result);
                break;
            }

            case OP_newfunction:
            {
                uint32_t function_id = (uint32_t) op1;
                int32_t index = (int32_t) op2;
                //sp[0] = core->newfunction(env, body, _scopeBase, scopeDepth);
                MethodInfo* func = pool->getMethodInfo(function_id);
                int extraScopes = state->scopeDepth;

                // prepare scopechain args for call
                LIns* ap = storeAtomArgs(extraScopes, state->verifier->scopeBase);

                LIns* outer = loadEnvScope();
                LIns* i3 = callIns(FUNCTIONID(newfunction), 4,
                    env_param, InsConstPtr(func), outer, ap);
                liveAlloc(ap);

                AvmAssert(!result->isMachineType());
                localSet(index, i3, result);
                break;
            }

            case OP_call:
            {
                // stack in: method obj arg1..N
                // sp[-argc-1] = op_call(env, sp[-argc], argc, ...)
                int argc = int(op1);
                int funcDisp = sp - argc - 1;
                int dest = funcDisp;

                // convert args to Atom[] for the call
                LIns* func = loadAtomRep(funcDisp);
                LIns* ap = storeAtomArgs(loadAtomRep(funcDisp+1), argc, funcDisp+2);
                LIns* i3 = callIns(FUNCTIONID(op_call), 4, env_param, func, InsConst(argc), ap);
                liveAlloc(ap);
                localSet(dest, atomToNativeRep(result, i3), result);
                break;
            }

            case OP_callproperty:
            case OP_callproplex:
            case OP_callpropvoid:
            {
                // stack in: obj [ns [name]] arg1..N
                // stack out: result

                // obj = sp[-argc]
                //tempAtom = callproperty(env, name, toVTable(obj), argc, ...);
                //  *(sp -= argc) = tempAtom;
                int argc = int(op2);
                int argv = sp-argc+1;
                int baseDisp = sp-argc;
                const Multiname* name = pool->precomputedMultiname((int)op1);
                LIns* multi = initMultiname(name, baseDisp);
                AvmAssert(state->value(baseDisp).notNull);

                // convert args to Atom[] for the call
                LIns* base = loadAtomRep(baseDisp);
                LIns* receiver = opcode == OP_callproplex ? InsConstAtom(nullObjectAtom) : base;
                LIns* ap = storeAtomArgs(receiver, argc, argv);

                Traits* baseTraits = state->value(baseDisp).traits;
                Binding b = state->verifier->getToplevel(this)->getBinding(baseTraits, name);

                LIns* out;
                if (AvmCore::isSlotBinding(b)) {
                    // can early bind call to closure in slot
                    Traits* slotType = state->verifier->readBinding(baseTraits, b);
                    // todo if funcValue is already a ScriptObject then don't box it, use a different helper.
                    LIns* funcValue = loadFromSlot(baseDisp, AvmCore::bindingToSlotId(b), slotType);
                    LIns* funcAtom = nativeToAtom(funcValue, slotType);
                    out = callIns(FUNCTIONID(op_call), 4, env_param, funcAtom, InsConst(argc), ap);
                }
                else if (!name->isRuntime()) {
                    // use inline cache for late bound call
                    // cache contains: [handler, vtable, [data], Multiname*]
                    // and we call (*cache->handler)(cache, obj, argc, args*, MethodEnv*)
                    CallCache* cache = call_cache_builder.allocateCacheSlot(name);
                    LIns* cacheAddr = InsConstPtr(cache);
                    LIns* handler = loadIns(LIR_ldp, offsetof(CallCache, call_handler), cacheAddr, ACC_OTHER);
                    out = callIns(FUNCTIONID(call_cache_handler), 6,
                        handler, cacheAddr, base, InsConst(argc), ap, env_param);
                }
                else {
                    // generic late bound call to anything
                    out = callIns(FUNCTIONID(callprop_late), 5, env_param, base, multi, InsConst(argc), ap);
                    liveAlloc(multi);
                }
                liveAlloc(ap);
                localSet(baseDisp, atomToNativeRep(result, out), result);
                break;
            }

            case OP_constructprop:
            {
                // stack in: obj [ns [name]] arg1..N
                // stack out: result

                int argc = int(op2);
                // obj = sp[-argc]
                //tempAtom = callproperty(env, name, toVTable(obj), argc, ...);
                //  *(sp -= argc) = tempAtom;
                int argv = sp-argc+1;

                int objDisp = sp-argc;
                LIns* multi = initMultiname((Multiname*)op1, objDisp);
                AvmAssert(state->value(objDisp).notNull);

                // convert args to Atom[] for the call
                LIns* ap = storeAtomArgs(loadAtomRep(objDisp), argc, argv);
                LIns* i3 = callIns(FUNCTIONID(construct_late), 4,
                    env_param, multi, InsConst(argc), ap);
                liveAlloc(multi);
                liveAlloc(ap);

                localSet(objDisp, atomToNativeRep(result, i3), result);
                break;
            }

            case OP_callsuper:
            case OP_callsupervoid:
            {
                // stack in: obj [ns [name]] arg1..N
                // stack out: result
                // null check must have already happened.
                //  tempAtom = callsuper(multiname, obj, sp-argc+1, argc, vtable->base);
                int argc = int(op2);
                int argv = sp - argc + 1;
                int objDisp = sp - argc;
                LIns* multi = initMultiname((Multiname*)op1, objDisp);
                AvmAssert(state->value(objDisp).notNull);

                // convert args to Atom[] for the call
                LIns* obj = loadAtomRep(objDisp);

                LIns* ap = storeAtomArgs(obj, argc, argv);

                LIns* i3 = callIns(FUNCTIONID(callsuper), 4,
                    env_param, multi, InsConst(argc), ap);
                liveAlloc(multi);
                liveAlloc(ap);

                localSet(objDisp, atomToNativeRep(result, i3), result);
                break;
            }

            case OP_applytype:
            {
                // stack in: method arg1..N
                // sp[-argc] = applytype(env, sp[-argc], argc, null, arg1..N)
                int argc = int(op1);
                int funcDisp = sp - argc;
                int dest = funcDisp;
                int arg0 = sp - argc + 1;

                LIns* func = loadAtomRep(funcDisp);

                // convert args to Atom[] for the call
                LIns* ap = storeAtomArgs(argc, arg0);

                LIns* i3 = callIns(FUNCTIONID(op_applytype), 4,
                    env_param, func, InsConst(argc), ap);
                liveAlloc(ap);

                localSet(dest, atomToNativeRep(result, i3), result);
                break;
            }

            case OP_newobject:
            {
                // result = env->op_newobject(sp, argc)
                int argc = int(op1);
                int dest = sp - (2*argc-1);
                int arg0 = dest;

                // convert args to Atom for the call[]
                LIns* ap = storeAtomArgs(2*argc, arg0);

                LIns* i3 = callIns(FUNCTIONID(op_newobject), 3,
                    env_param, leaIns(sizeof(Atom)*(2*argc-1), ap), InsConst(argc));
                liveAlloc(ap);

                localSet(dest, ptrToNativeRep(result, i3), result);
                break;
            }

            case OP_newactivation:
            {
                // result = env->newActivation()
                LIns* activation = callIns(FUNCTIONID(newActivation), 1, env_param);
                localSet(sp+1, ptrToNativeRep(result, activation), result);
                break;
            }

            case OP_newcatch:
            {
                // result = core->newObject(env->activation, NULL);
                int dest = sp+1;

                LIns* activation = callIns(FUNCTIONID(newcatch), 2,
                                         env_param, InsConstPtr(result));

                localSet(dest, ptrToNativeRep(result, activation), result);
                break;
            }

            case OP_newarray:
            {
                // sp[-argc+1] = env->toplevel()->arrayClass->newarray(sp-argc+1, argc)
                int argc = int(op1);
                int arg0 = sp - 1*argc+1;

                // convert array elements to Atom[]
                LIns* ap = storeAtomArgs(argc, arg0);
                LIns* i3 = callIns(FUNCTIONID(newarray), 3, env_param, InsConst(argc), ap);
                liveAlloc(ap);

                AvmAssert(!result->isMachineType());
                localSet(arg0, i3, result);
                break;
            }

            case OP_newclass:
            {
                // sp[0] = core->newclass(env, ctraits, scopeBase, scopeDepth, base)
                Traits* ctraits = (Traits*) op1;
                int localindex = int(op2);
                int extraScopes = state->scopeDepth;

                LIns* outer = loadEnvScope();
                LIns* base = localGetp(localindex);

                // prepare scopechain args for call
                LIns* ap = storeAtomArgs(extraScopes, state->verifier->scopeBase);

                LIns* i3 = callIns(FUNCTIONID(newclass), 5,
                    env_param, InsConstPtr(ctraits), base, outer, ap);
                liveAlloc(ap);

                AvmAssert(!result->isMachineType());
                localSet(localindex, i3, result);
                break;
            }

            case OP_getdescendants:
            {
                // stack in: obj [ns [name]]
                // stack out: value
                //sp[0] = core->getdescendants(sp[0], name);
                int objDisp = sp;
                Multiname* multiname = (Multiname*) op1;

                LIns* multi = initMultiname(multiname, objDisp);
                LIns* obj = loadAtomRep(objDisp);
                AvmAssert(state->value(objDisp).notNull);

                LIns* out = callIns(FUNCTIONID(getdescendants), 3,
                    env_param, obj, multi);
                liveAlloc(multi);

                localSet(objDisp, atomToNativeRep(result, out), result);
                break;
            }

            case OP_checkfilter: {
                int32_t index = (int32_t) op1;
                callIns(FUNCTIONID(checkfilter), 2, env_param, loadAtomRep(index));
                break;
            }

            case OP_findpropstrict:
            case OP_findproperty:
            {
                // stack in: [ns [name]]
                // stack out: obj
                // sp[1] = env->findproperty(scopeBase, scopedepth, name, strict)
                int dest = sp;
                LIns* multi = initMultiname((Multiname*)op1, dest);
                dest++;
                int extraScopes = state->scopeDepth;

                // prepare scopechain args for call
                LIns* ap = storeAtomArgs(extraScopes, state->verifier->scopeBase);

                LIns* outer = loadEnvScope();

                LIns* withBase;
                if (state->withBase == -1)
                {
                    withBase = InsConstPtr(0);
                }
                else
                {
                    withBase = leaIns(state->withBase*sizeof(Atom), ap);
                }

                //      return env->findproperty(outer, argv, extraScopes, name, strict);

                LIns* i3 = callIns(FUNCTIONID(findproperty), 7,
                    env_param, outer, ap, InsConst(extraScopes), multi,
                    InsConst((int32_t)(opcode == OP_findpropstrict)),
                    withBase);
                liveAlloc(multi);
                liveAlloc(ap);

                localSet(dest, atomToNativeRep(result, i3), result);
                break;
            }

            case OP_getproperty:
            {
                // stack in: obj [ns] [name]
                // stack out: value
                // obj=sp[0]
                //sp[0] = env->getproperty(obj, multiname);

                const Multiname* multiname = pool->precomputedMultiname((int)op1);
                bool attr = multiname->isAttr();
                Traits* indexType = state->value(sp).traits;
                int objDisp = sp;

                bool maybeIntegerIndex = !attr && multiname->isRtname() && multiname->containsAnyPublicNamespace();
                if (maybeIntegerIndex && indexType == INT_TYPE)
                {
                    bool valIsAtom = true;
                    LIns* index = localGet(objDisp--);

                    if (multiname->isRtns())
                    {
                        // Discard runtime namespace
                        objDisp--;
                    }

                    Traits* objType = state->value(objDisp).traits;

                    LIns *value;
                    if (objType == ARRAY_TYPE || (objType!= NULL && objType->subtypeof(VECTOROBJ_TYPE)) )
                    {
                        value = callIns((objType==ARRAY_TYPE ?
                                        FUNCTIONID(ArrayObject_getIntProperty) :
                                        FUNCTIONID(ObjectVectorObject_getIntProperty)), 2,
                            localGetp(sp-1), index);
                    }
                    else if( objType == VECTORINT_TYPE || objType == VECTORUINT_TYPE)
                    {
                        if( result == INT_TYPE || result == UINT_TYPE)
                        {
                            value = callIns((objType==VECTORINT_TYPE ?
                                                    FUNCTIONID(IntVectorObject_getNativeIntProperty) :
                                                    FUNCTIONID(UIntVectorObject_getNativeIntProperty)), 2,
                            localGetp(sp-1), index);
                            valIsAtom = false;
                        }
                        else
                        {
                            value = callIns((objType==VECTORINT_TYPE ?
                                                    FUNCTIONID(IntVectorObject_getIntProperty) :
                                                    FUNCTIONID(UIntVectorObject_getIntProperty)), 2,
                            localGetp(sp-1), index);
                        }
                    }
                    else if( objType == VECTORDOUBLE_TYPE )
                    {
                        if( result == NUMBER_TYPE )
                        {
                            value = callIns(FUNCTIONID(DoubleVectorObject_getNativeIntProperty), 2,
                                localGetp(sp-1), index);
                            valIsAtom = false;
                        }
                        else
                        {
                            value = callIns(FUNCTIONID(DoubleVectorObject_getIntProperty), 2,
                                localGetp(sp-1), index);
                        }
                    }
                    else
                    {
                        value = callIns(FUNCTIONID(getpropertylate_i), 3,
                            env_param, loadAtomRep(sp-1), index);
                    }

                    localSet(sp-1, valIsAtom?atomToNativeRep(result, value):value, result);
                }
                else if (maybeIntegerIndex && indexType == UINT_TYPE)
                {
                    bool valIsAtom = true;

                    LIns* index = localGet(objDisp--);

                    if (multiname->isRtns())
                    {
                        // Discard runtime namespace
                        objDisp--;
                    }

                    Traits* objType = state->value(objDisp).traits;

                    LIns *value;
                    if (objType == ARRAY_TYPE || (objType!= NULL && objType->subtypeof(VECTOROBJ_TYPE)))
                    {
                        value = callIns((objType==ARRAY_TYPE ?
                                                FUNCTIONID(ArrayObject_getUintProperty) :
                                                FUNCTIONID(ObjectVectorObject_getUintProperty)), 2,
                            localGetp(sp-1), index);
                    }
                    else if( objType == VECTORINT_TYPE || objType == VECTORUINT_TYPE )
                    {
                        if( result == INT_TYPE || result == UINT_TYPE )
                        {
                            value = callIns((objType==VECTORINT_TYPE ?
                                                    FUNCTIONID(IntVectorObject_getNativeUintProperty) :
                                                    FUNCTIONID(UIntVectorObject_getNativeUintProperty)), 2,
                            localGetp(sp-1), index);
                            valIsAtom = false;
                        }
                        else
                        {
                            value = callIns((objType==VECTORINT_TYPE ?
                                                    FUNCTIONID(IntVectorObject_getUintProperty) :
                                                    FUNCTIONID(UIntVectorObject_getUintProperty)), 2,
                            localGetp(sp-1), index);
                        }
                    }
                    else if( objType == VECTORDOUBLE_TYPE )
                    {
                        if( result == NUMBER_TYPE )//|| result == UINT_TYPE)
                        {
                            value = callIns(FUNCTIONID(DoubleVectorObject_getNativeUintProperty), 2,
                                localGetp(sp-1), index);
                            valIsAtom = false;
                        }
                        else
                        {
                            value = callIns(FUNCTIONID(DoubleVectorObject_getUintProperty), 2,
                                localGetp(sp-1), index);
                        }
                    }
                    else
                    {
                        value = callIns(FUNCTIONID(getpropertylate_u), 3,
                            env_param, loadAtomRep(sp-1), index);
                    }

                    localSet(sp-1, valIsAtom?atomToNativeRep(result, value):value, result);
                }
                else if (maybeIntegerIndex && indexType != STRING_TYPE)
                {
                    LIns* multi = InsConstPtr(multiname); // inline ptr to precomputed name
                    LIns* index = loadAtomRep(objDisp--);
                    AvmAssert(state->value(objDisp).notNull);
                    LIns* obj = loadAtomRep(objDisp);
                    LIns* value = callIns(FUNCTIONID(getprop_index), 4,
                                        env_param, obj, multi, index);

                    localSet(objDisp, atomToNativeRep(result, value), result);
                }
                else
                {
                    LIns* multi = initMultiname(multiname, objDisp);
                    AvmAssert(state->value(objDisp).notNull);

                    LIns* value;
                    LIns* obj = loadAtomRep(objDisp);
                    if (multiname->isRuntime()) {
                        //return getprop_late(obj, name);
                        value = callIns(FUNCTIONID(getprop_late), 3, env_param, obj, multi);
                        liveAlloc(multi);
                    } else {
                        // static name, use property cache
                        GetCache* cache = get_cache_builder.allocateCacheSlot(multiname);
                        LIns* cacheAddr = InsConstPtr(cache);
                        LIns* handler = loadIns(LIR_ldp, offsetof(GetCache, get_handler), cacheAddr, ACC_OTHER);
                        value = callIns(FUNCTIONID(get_cache_handler), 4, handler, cacheAddr, env_param, obj);
                    }

                    localSet(objDisp, atomToNativeRep(result, value), result);
                }
                break;
            }
            case OP_initproperty:
            case OP_setproperty:
            {
                // stack in: obj [ns] [name] value
                // stack out:
                // obj = sp[-1]
                //env->setproperty(obj, multiname, sp[0], toVTable(obj));
                //LIns* value = loadAtomRep(sp);

                const Multiname* multiname = (const Multiname*)op1;
                bool attr = multiname->isAttr();
                Traits* indexType = state->value(sp-1).traits;
                Traits* valueType = state->value(sp).traits;
                int objDisp = sp-1;

                bool maybeIntegerIndex = !attr && multiname->isRtname() && multiname->containsAnyPublicNamespace();

                if (maybeIntegerIndex && indexType == INT_TYPE)
                {
                    LIns* index = localGet(objDisp--);

                    if (multiname->isRtns())
                    {
                        // Discard runtime namespace
                        objDisp--;
                    }

                    Traits* objType = state->value(objDisp).traits;

                    if (objType == ARRAY_TYPE || (objType!= NULL && objType->subtypeof(VECTOROBJ_TYPE)))
                    {
                        LIns* value = loadAtomRep(sp);
                        callIns((objType==ARRAY_TYPE ?
                                        FUNCTIONID(ArrayObject_setIntProperty) :
                                        FUNCTIONID(ObjectVectorObject_setIntProperty)), 3,
                            localGetp(objDisp), index, value);
                    }
                    else if(objType == VECTORINT_TYPE || objType == VECTORUINT_TYPE )
                    {
                        if( valueType == INT_TYPE )
                        {
                            LIns* value = localGet(sp);
                            callIns((objType==VECTORINT_TYPE ?
                                            FUNCTIONID(IntVectorObject_setNativeIntProperty) :
                                            FUNCTIONID(UIntVectorObject_setNativeIntProperty)),
                                            3,
                                            localGetp(objDisp), index, value);
                        }
                        else if( valueType == UINT_TYPE )
                        {
                            LIns* value = localGet(sp);
                            callIns((objType==VECTORINT_TYPE ?
                                            FUNCTIONID(IntVectorObject_setNativeIntProperty) :
                                            FUNCTIONID(UIntVectorObject_setNativeIntProperty)),
                                            3,
                                            localGetp(objDisp), index, value);
                        }
                        else
                        {
                            LIns* value = loadAtomRep(sp);
                            value = callIns((objType==VECTORINT_TYPE ?
                                                    FUNCTIONID(IntVectorObject_setIntProperty) :
                                                    FUNCTIONID(UIntVectorObject_setIntProperty)),
                                                    3,
                                                    localGetp(objDisp), index, value);
                        }
                    }
                    else if(objType == VECTORDOUBLE_TYPE)
                    {
                        if( valueType == NUMBER_TYPE )
                        {
                            LIns* value = localGetf(sp);
                            callIns(FUNCTIONID(DoubleVectorObject_setNativeIntProperty), 3,
                                localGetp(objDisp), index, value);
                        }
                        else
                        {
                            LIns* value = loadAtomRep(sp);
                            value = callIns(FUNCTIONID(DoubleVectorObject_setIntProperty), 3,
                                localGetp(objDisp), index, value);
                        }
                    }
                    else
                    {
                        LIns* value = loadAtomRep(sp);
                        callIns(FUNCTIONID(setpropertylate_i), 4,
                            env_param, loadAtomRep(objDisp), index, value);
                    }
                }
                else if (maybeIntegerIndex && indexType == UINT_TYPE)
                {
                    LIns* index = localGet(objDisp--);

                    if (multiname->isRtns())
                    {
                        // Discard runtime namespace
                        objDisp--;
                    }

                    Traits* objType = state->value(objDisp).traits;

                    if (objType == ARRAY_TYPE || (objType!= NULL && objType->subtypeof(VECTOROBJ_TYPE)))
                    {
                        LIns* value = loadAtomRep(sp);
                        callIns((objType==ARRAY_TYPE ?
                                        FUNCTIONID(ArrayObject_setUintProperty) :
                                        FUNCTIONID(ObjectVectorObject_setUintProperty)), 3,
                            localGetp(objDisp), index, value);
                    }
                    else if(objType == VECTORINT_TYPE || objType == VECTORUINT_TYPE )
                    {
                        if( valueType == INT_TYPE )
                        {
                            LIns* value = localGet(sp);
                            callIns((objType==VECTORINT_TYPE ?
                                            FUNCTIONID(IntVectorObject_setNativeUintProperty) :
                                            FUNCTIONID(UIntVectorObject_setNativeUintProperty)),
                                            3,
                                            localGetp(objDisp), index, value);
                        }
                        else if( valueType == UINT_TYPE )
                        {
                            LIns* value = localGet(sp);
                            callIns((objType==VECTORINT_TYPE ?
                                            FUNCTIONID(IntVectorObject_setNativeUintProperty) :
                                            FUNCTIONID(UIntVectorObject_setNativeUintProperty)),
                                            3,
                                            localGetp(objDisp), index, value);
                        }
                        else
                        {
                            LIns* value = loadAtomRep(sp);
                            value = callIns((objType==VECTORINT_TYPE ?
                                                    FUNCTIONID(IntVectorObject_setUintProperty) :
                                                    FUNCTIONID(UIntVectorObject_setUintProperty)),
                                                    3,
                                                    localGetp(objDisp), index, value);
                        }
                    }
                    else if(objType == VECTORDOUBLE_TYPE)
                    {
                        if( valueType == NUMBER_TYPE )
                        {
                            LIns* value = localGetf(sp);
                            callIns(FUNCTIONID(DoubleVectorObject_setNativeUintProperty), 3,
                                localGetp(objDisp), index, value);
                        }
                        else
                        {
                            LIns* value = loadAtomRep(sp);
                            value = callIns(FUNCTIONID(DoubleVectorObject_setUintProperty), 3,
                                localGetp(objDisp), index, value);
                        }
                    }
                    else
                    {
                        LIns* value = loadAtomRep(sp);
                        callIns(FUNCTIONID(setpropertylate_u), 4,
                            env_param, loadAtomRep(objDisp), index, value);
                    }
                }
                else if (maybeIntegerIndex)
                {
                    LIns* name = InsConstPtr(multiname); // precomputed multiname
                    LIns* value = loadAtomRep(sp);
                    LIns* index = loadAtomRep(objDisp--);
                    AvmAssert(state->value(objDisp).notNull);
                    LIns* obj = loadAtomRep(objDisp);
                    const CallInfo *func = opcode==OP_setproperty ? FUNCTIONID(setprop_index) :
                                                        FUNCTIONID(initprop_index);
                    callIns(func, 5, env_param, obj, name, value, index);
                }
                else
                {
                    LIns* value = loadAtomRep(sp);
                    LIns* multi = initMultiname(multiname, objDisp);
                    AvmAssert(state->value(objDisp).notNull);

                    LIns* obj = loadAtomRep(objDisp);
                    if (opcode == OP_setproperty) {
                        if (!multiname->isRuntime()) {
                            // use inline cache for dynamic setproperty access
                            SetCache* cache = set_cache_builder.allocateCacheSlot(multiname);
                            LIns* cacheAddr = InsConstPtr(cache);
                            LIns* handler = loadIns(LIR_ldp, offsetof(SetCache, set_handler), cacheAddr, ACC_OTHER);
                            callIns(FUNCTIONID(set_cache_handler), 5, handler, cacheAddr, obj, value, env_param);
                        } else {
                            // last resort slow path for OP_setproperty
                            callIns(FUNCTIONID(setprop_late), 4, env_param, obj, multi, value);
                            liveAlloc(multi);
                        }
                    }
                    else
                    {
                        // initproplate is rare in jit code because we typically interpret static
                        // initializers, and constructor initializers tend to early-bind successfully.
                        callIns(FUNCTIONID(initprop_late), 4, env_param, obj, multi, value);
                        liveAlloc(multi);
                    }
                }
                break;
            }

            case OP_deleteproperty:
            {
                // stack in: obj [ns] [name]
                // stack out: Boolean
                //sp[0] = delproperty(sp[0], multiname);
                int objDisp = sp;
                Multiname *multiname = (Multiname*)op1;
                if(!multiname->isRtname()) {
                    LIns* multi = initMultiname(multiname, objDisp, true);

                    LIns* obj = loadAtomRep(objDisp);

                    LIns* i3 = callIns(FUNCTIONID(delproperty), 3,
                        env_param, obj, multi);
                    liveAlloc(multi);

                    localSet(objDisp, atomToNativeRep(result, i3), result);
                } else {
                    LIns* _tempname = copyMultiname(multiname);
                    LIns* index = loadAtomRep(objDisp--);

                    if( !multiname->isRtns() )
                    {
                        // copy the compile-time namespace to the temp multiname
                        LIns* mSpace = InsConstPtr(multiname->ns);
                        stp(mSpace, _tempname, offsetof(Multiname, ns), ACC_OTHER);
                    }
                    else
                    {
                        // intern the runtime namespace and copy to the temp multiname
                        LIns* nsAtom = loadAtomRep(objDisp--);
                        LIns* internNs = callIns(FUNCTIONID(internRtns), 2,
                            env_param, nsAtom);

                        stp(internNs, _tempname, offsetof(Multiname,ns), ACC_OTHER);
                    }
                    liveAlloc(_tempname);

                    AvmAssert(state->value(objDisp).notNull);
                    LIns* obj = loadAtomRep(objDisp);

                    LIns* value = callIns(FUNCTIONID(delpropertyHelper), 4,
                                        env_param, obj, _tempname, index);

                    localSet(objDisp, atomToNativeRep(result, value), result);
                }
                break;
            }

            case OP_esc_xelem: // ToXMLString will call EscapeElementValue
            {
                //sp[0] = core->ToXMLString(sp[0]);
                int32_t index = (int32_t) op1;
                LIns* value = loadAtomRep(index);
                LIns* i3 = callIns(FUNCTIONID(ToXMLString), 2,
                    coreAddr, value);
                AvmAssert(result == STRING_TYPE);
                localSet(index, i3, result);
                break;
            }

            case OP_esc_xattr:
            {
                //sp[0] = core->EscapeAttributeValue(sp[0]);
                int32_t index = (int32_t) op1;
                LIns* value = loadAtomRep(index);
                LIns* i3 = callIns(FUNCTIONID(EscapeAttributeValue), 2,
                    coreAddr, value);
                AvmAssert(result == STRING_TYPE);
                localSet(index, i3, result);
                break;
            }

            case OP_astype:
            {
                // sp[0] = AvmCore::astype(sp[0], traits)
                Traits *type = (Traits*) op1;
                int32_t index = (int32_t) op2;
                LIns* obj = loadAtomRep(index);
                LIns* i1 = callIns(FUNCTIONID(astype), 2, obj, InsConstPtr(type));
                i1 = atomToNativeRep(result, i1);
                localSet(index, i1, result);
                break;
            }

            case OP_astypelate:
            {
                //sp[-1] = astype_late(env, sp[-1], sp[0]);
                LIns* type = loadAtomRep(sp);
                LIns* obj = loadAtomRep(sp-1);
                LIns* i3 = callIns(FUNCTIONID(astype_late), 3, env_param, obj, type);
                i3 = atomToNativeRep(result, i3);
                localSet(sp-1, i3, result);
                break;
            }

            case OP_strictequals:
            {
                AvmAssert(result == BOOLEAN_TYPE);
                localSet(sp-1, cmpEq(FUNCTIONID(stricteq), sp-1, sp), result);
                break;
            }

            case OP_equals:
            {
                AvmAssert(result == BOOLEAN_TYPE);
                localSet(sp-1, cmpEq(FUNCTIONID(equals), sp-1, sp), result);
                break;
            }

            case OP_lessthan:
            {
                AvmAssert(result == BOOLEAN_TYPE);
                localSet(sp-1, cmpLt(sp-1, sp), result);
                break;
            }

            case OP_lessequals:
            {
                AvmAssert(result == BOOLEAN_TYPE);
                localSet(sp-1, cmpLe(sp-1, sp), result);
                break;
            }

            case OP_greaterthan:
            {
                AvmAssert(result == BOOLEAN_TYPE);
                localSet(sp-1, cmpLt(sp, sp-1), result);
                break;
            }

            case OP_greaterequals:
            {
                AvmAssert(result == BOOLEAN_TYPE);
                localSet(sp-1, cmpLe(sp, sp-1), result);
                break;
            }

            case OP_instanceof:
            {
                LIns* lhs = loadAtomRep(sp-1);
                LIns* rhs = loadAtomRep(sp);
                LIns* out = callIns(FUNCTIONID(instanceof), 3, env_param, lhs, rhs);
                out = atomToNativeRep(result, out);
                localSet(sp-1,  out, result);
                break;
            }

            case OP_in:
            {
                LIns* lhs = loadAtomRep(sp-1);
                LIns* rhs = loadAtomRep(sp);
                LIns* out = callIns(FUNCTIONID(op_in), 3, env_param, lhs, rhs);
                out = atomToNativeRep(result, out);
                localSet(sp-1, out, result);
                break;
            }

            case OP_dxns:
            {
                LIns* uri = InsConstPtr((String*)op1); // namespace uri from string pool
                callIns(FUNCTIONID(setDxns), 3, coreAddr, methodFrame, uri);
                break;
            }

            case OP_dxnslate:
            {
                int32_t index = (int32_t) op1;
                LIns* atom = loadAtomRep(index);
                callIns(FUNCTIONID(setDxnsLate), 3, coreAddr, methodFrame, atom);
                break;
            }

            /*
             * debugger instructions
             */
            case OP_debugfile:
            {
            #ifdef DEBUGGER
            if (haveDebugger) {
                // todo refactor api's so we don't have to pass argv/argc
                LIns* debugger = loadIns(LIR_ldp, offsetof(AvmCore, _debugger), coreAddr, ACC_READONLY);
                callIns(FUNCTIONID(debugFile), 2,
                        debugger,
                        InsConstPtr((String*)op1));
            }
            #endif // DEBUGGER
           #ifdef VTUNE
                Ins(LIR_file, InsConstPtr((String*)op1));
           #endif /* VTUNE */
                break;
            }

            case OP_debugline:
            {
            #ifdef DEBUGGER
            if (haveDebugger) {
                // todo refactor api's so we don't have to pass argv/argc
                LIns* debugger = loadIns(LIR_ldp, offsetof(AvmCore, _debugger), coreAddr, ACC_READONLY);
                callIns(FUNCTIONID(debugLine), 2,
                        debugger,
                        InsConst((int32_t)op1));
            }
            #endif // DEBUGGER
            #ifdef VTUNE
                Ins(LIR_line, InsConst((int32_t)op1));
                hasDebugInfo = true;
           #endif /* VTUNE */
                break;
            }

            default:
            {
                AvmAssert(false); // unsupported
            }
        }

    } // emit()

    void CodegenLIR::emitIf(AbcOpcode opcode, int target_off, int a, int b)
    {
#ifdef DEBUGGER
        Sampler* s = core->get_sampler();
        if (s && s->sampling() && target_off < state->pc)
        {
            emitSampleCheck();
        }
#endif

        //
        // compile instructions that cannot throw exceptions before we add exception handling logic
        //

        // op1 = abc opcode target
        // op2 = what local var contains condition

        LIns* cond;
        LOpcode br;

        switch (opcode)
        {
        case OP_iftrue:
            NanoAssert(state->value(a).traits == BOOLEAN_TYPE);
            br = LIR_jf;
            cond = eqi0(localGet(a));
            break;
        case OP_iffalse:
            NanoAssert(state->value(a).traits == BOOLEAN_TYPE);
            br = LIR_jt;
            cond = eqi0(localGet(a));
            break;
        case OP_iflt:
            br = LIR_jt;
            cond = cmpLt(a, b);
            break;
        case OP_ifnlt:
            br = LIR_jf;
            cond = cmpLt(a, b);
            break;
        case OP_ifle:
            br = LIR_jt;
            cond = cmpLe(a, b);
            break;
        case OP_ifnle:
            br = LIR_jf;
            cond = cmpLe(a, b);
            break;
        case OP_ifgt:  // a>b === b<a
            br = LIR_jt;
            cond = cmpLt(b, a);
            break;
        case OP_ifngt: // !(a>b) === !(b<a)
            br = LIR_jf;
            cond = cmpLt(b, a);
            break;
        case OP_ifge:  // a>=b === b<=a
            br = LIR_jt;
            cond = cmpLe(b, a);
            break;
        case OP_ifnge: // !(a>=b) === !(a<=b)
            br = LIR_jf;
            cond = cmpLe(b, a);
            break;
        case OP_ifeq:
            br = LIR_jt;
            cond = cmpEq(FUNCTIONID(equals), a, b);
            break;
        case OP_ifne:
            br = LIR_jf;
            cond = cmpEq(FUNCTIONID(equals), a, b);
            break;
        case OP_ifstricteq:
            br = LIR_jt;
            cond = cmpEq(FUNCTIONID(stricteq), a, b);
            break;
        case OP_ifstrictne:
            br = LIR_jf;
            cond = cmpEq(FUNCTIONID(stricteq), a, b);
            break;
        default:
            AvmAssert(false);
            return;
        }

        if (cond->isImmI()) {
            if ((br == LIR_jt && cond->immI()) || (br == LIR_jf && !cond->immI())) {
                // taken
                br = LIR_j;
                cond = 0;
            }
            else {
                // not taken = no-op
                return;
            }
        }

        branchToAbcPos(br, cond, target_off);
    } // emitIf()

    // Faster compares for ints, uint, doubles
    LIns* CodegenLIR::cmpOptimization(int lhsi, int rhsi, LOpcode icmp, LOpcode ucmp, LOpcode fcmp)
    {
        Traits* lht = state->value(lhsi).traits;
        Traits* rht = state->value(rhsi).traits;

        if (lht == rht && lht == INT_TYPE)
        {
            LIns* lhs = localGet(lhsi);
            LIns* rhs = localGet(rhsi);
            return binaryIns(icmp, lhs, rhs);
        }
        else if (lht == rht && lht == UINT_TYPE)
        {
            LIns* lhs = localGet(lhsi);
            LIns* rhs = localGet(rhsi);
            return binaryIns(ucmp, lhs, rhs);
        }
        else if (lht && lht->isNumeric() && rht && rht->isNumeric())
        {
            // If we're comparing a uint to an int and the int is a non-negative
            // integer constant, don't promote to doubles for the compare
            if ((lht == UINT_TYPE) && (rht == INT_TYPE))
            {
                LIns* lhs = localGet(lhsi);
                LIns* rhs = localGet(rhsi);
            #ifdef AVMPLUS_64BIT
                // 32-bit signed and unsigned values fit in 64-bit registers
                // so we can promote and simply do a signed 64bit compare
                LOpcode qcmp = cmpOpcodeI2Q(icmp);
                NanoAssert((icmp == LIR_eqi && qcmp == LIR_eqq) ||
                           (icmp == LIR_lti && qcmp == LIR_ltq) ||
                           (icmp == LIR_lei && qcmp == LIR_leq));
                return binaryIns(qcmp, ui2p(lhs), i2p(rhs));
            #else
                if (rhs->isImmI() && rhs->immI() >= 0)
                    return binaryIns(ucmp, lhs, rhs);
            #endif
            }
            else if ((lht == INT_TYPE) && (rht == UINT_TYPE))
            {
                LIns* lhs = localGet(lhsi);
                LIns* rhs = localGet(rhsi);
            #ifdef AVMPLUS_64BIT
                // 32-bit signed and unsigned values fit in 64-bit registers
                // so we can promote and simply do a signed 64bit compare
                LOpcode qcmp = cmpOpcodeI2Q(icmp);
                NanoAssert((icmp == LIR_eqi && qcmp == LIR_eqq) ||
                           (icmp == LIR_lti && qcmp == LIR_ltq) ||
                           (icmp == LIR_lei && qcmp == LIR_leq));
                return binaryIns(qcmp, i2p(lhs), ui2p(rhs));
            #else
                if (lhs->isImmI() && lhs->immI() >= 0)
                    return binaryIns(ucmp, lhs, rhs);
            #endif
            }

            LIns* lhs = promoteNumberIns(lht, lhsi);
            LIns* rhs = promoteNumberIns(rht, rhsi);
            return binaryIns(fcmp, lhs, rhs);
        }

        return NULL;
    }

    // set cc's for < operator
    LIns* CodegenLIR::cmpLt(int lhsi, int rhsi)
    {
        LIns *result = cmpOptimization (lhsi, rhsi, LIR_lti, LIR_ltui, LIR_ltd);
        if (result)
            return result;

        AvmAssert(trueAtom == 13);
        AvmAssert(falseAtom == 5);
        AvmAssert(undefinedAtom == 4);
        LIns* lhs = loadAtomRep(lhsi);
        LIns* rhs = loadAtomRep(rhsi);
        LIns* atom = callIns(FUNCTIONID(compare), 3,
            coreAddr, lhs, rhs);

        // caller will use jt for (a<b) and jf for !(a<b)
        // compare          ^8    <8
        // true       1101  0101   y
        // false      0101  1101   n
        // undefined  0100  1100   n

        LIns* c = InsConst(8);
        return binaryIns(LIR_lti, binaryIns(LIR_xori, p2i(atom), c), c);
    }

    LIns* CodegenLIR::cmpLe(int lhsi, int rhsi)
    {
        LIns *result = cmpOptimization (lhsi, rhsi, LIR_lei, LIR_leui, LIR_led);
        if (result)
            return result;

        LIns* lhs = loadAtomRep(lhsi);
        LIns* rhs = loadAtomRep(rhsi);
        LIns* atom = callIns(FUNCTIONID(compare), 3,
            coreAddr, rhs, lhs);

        // assume caller will use jt for (a<=b) and jf for !(a<=b)
        // compare          ^1    <=4
        // true       1101  1100  n
        // false      0101  0100  y
        // undefined  0100  0101  n

        LIns* c2 = InsConst(1);
        LIns* c4 = InsConst(4);
        return binaryIns(LIR_lei, binaryIns(LIR_xori, p2i(atom), c2), c4);
    }

    LIns* CodegenLIR::cmpEq(const CallInfo *fid, int lhsi, int rhsi)
    {
        LIns *result = cmpOptimization (lhsi, rhsi, LIR_eqi, LIR_eqi, LIR_eqd);
        if (result) {
            return result;
        }

        Traits* lht = state->value(lhsi).traits;
        Traits* rht = state->value(rhsi).traits;

        // If we have null and a type that is derived from an Object (but not Object or XML)
        // we can optimize our equal comparison down to a simple ptr comparison. This also
        // works when both types are derived Object types.
        if (((lht == NULL_TYPE) && (rht && !rht->notDerivedObjectOrXML())) ||
            ((rht == NULL_TYPE) && (lht && !lht->notDerivedObjectOrXML())) ||
            ((rht && !rht->notDerivedObjectOrXML()) && (lht && !lht->notDerivedObjectOrXML())))
        {
            LIns* lhs = localGetp(lhsi);
            LIns* rhs = localGetp(rhsi);
            result = binaryIns(LIR_eqp, lhs, rhs);
        }
        else
        {
            LIns* lhs = loadAtomRep(lhsi);
            LIns* rhs = loadAtomRep(rhsi);
            LIns* out = callIns(fid, 3, coreAddr, lhs, rhs);
            result = binaryIns(LIR_eqp, out, InsConstAtom(trueAtom));
        }
        return result;
    }

    void CodegenLIR::writeEpilogue(const FrameState *state)
    {
        this->state = state;
        this->labelCount = state->verifier->getBlockCount();

        if (mop_rangeCheckFailed_label.unpatchedEdges) {
            emitLabel(mop_rangeCheckFailed_label);
            Ins(LIR_regfence);
            callIns(FUNCTIONID(mop_rangeCheckFailed), 1, env_param);
        }

        if (npe_label.unpatchedEdges) {
            emitLabel(npe_label);
            Ins(LIR_regfence);
            callIns(FUNCTIONID(npe), 1, env_param);
        }

        if (interrupt_label.unpatchedEdges) {
            emitLabel(interrupt_label);
            Ins(LIR_regfence);
            callIns(FUNCTIONID(handleInterruptMethodEnv), 1, env_param);
        }

        if (info->hasExceptions()) {
            emitLabel(catch_label);

            // exception case
            LIns *exptr = loadIns(LIR_ldp, offsetof(AvmCore, exceptionAddr), coreAddr, ACC_OTHER);
            LIns *exAtom = loadIns(LIR_ldp, offsetof(Exception, atom), exptr, ACC_OTHER);
            localSet(state->verifier->stackBase, exAtom, NULL);
            // need to convert exception from atom to native rep, at top of
            // catch handler.  can't do it here because it could be any type.

            // _ef.beginCatch()
            LIns* pc = loadIns(LIR_ldp, 0, _save_eip, ACC_OTHER);
            LIns* handler = callIns(FUNCTIONID(beginCatch), 5,
                coreAddr, _ef, InsConstPtr(info), pc, exptr);

            int handler_count = info->abc_exceptions()->exception_count;
            // Jump to catch handler
            LIns *handler_target = loadIns(LIR_ldi, offsetof(ExceptionHandler, target), handler, ACC_OTHER);
            // Do a compare & branch to each possible target.
            for (int i=0; i < handler_count; i++) {
                ExceptionHandler* h = &info->abc_exceptions()->exceptions[i];
                int handler_pc_off = h->target;
                if (state->verifier->hasFrameState(handler_pc_off)) {
                    CodegenLabel& label = getCodegenLabel(handler_pc_off);
                    AvmAssert(label.labelIns != NULL);
                    LIns* cond = binaryIns(LIR_eqi, handler_target, InsConst(handler_pc_off));
                    // don't use branchIns() here because we don't want to check null bits;
                    // this backedge is internal to exception handling and doesn't affect user
                    // variable dataflow.
                    lirout->insBranch(LIR_jt, cond, label.labelIns);
                }
            }
            livep(_ef);
            livep(_save_eip);
        }

        if (prolog->env_scope)      livep(prolog->env_scope);
        if (prolog->env_vtable)     livep(prolog->env_vtable);
        if (prolog->env_abcenv)     livep(prolog->env_abcenv);
        if (prolog->env_domainenv)  livep(prolog->env_domainenv);
        if (prolog->env_toplevel)   livep(prolog->env_toplevel);

        #ifdef DEBUGGER
        if (haveDebugger)
            livep(csn);
        #endif

        // extend live range of critical stuff
        // fixme -- this should be automatic based on live analysis
        livep(methodFrame);
        livep(env_param);
        frag->lastIns = livep(coreAddr);
        prologLastIns = prolog->lastIns;

        info->set_lookup_cache_size(finddef_cache_builder.next_cache);
    }

    // emit code to create a stack-allocated copy of the given multiname.
    // this helper only initializes Multiname.flags and Multiname.next_index
    LIns* CodegenLIR::copyMultiname(const Multiname* multiname)
    {
        LIns* name = insAlloc(sizeof(Multiname));
        sti(InsConst(multiname->ctFlags()), name, offsetof(Multiname, flags), ACC_OTHER);
        sti(InsConst(multiname->next_index), name, offsetof(Multiname, next_index), ACC_OTHER);
        return name;
    }

    LIns* CodegenLIR::initMultiname(const Multiname* multiname, int& csp, bool isDelete /*=false*/)
    {
        if (!multiname->isRuntime()) {
            // use the precomputed multiname
            return InsConstPtr(multiname);
        }

        // create an initialize a copy of the given multiname
        LIns* _tempname = copyMultiname(multiname);

        // then initialize its name and ns|nsset fields.
        LIns* nameAtom = NULL;
        if (multiname->isRtname())
        {
            nameAtom = loadAtomRep(csp--);
        }
        else
        {
            // copy the compile-time name to the temp name
            LIns* mName = InsConstPtr(multiname->name);
            stp(mName, _tempname, offsetof(Multiname,name), ACC_OTHER);
        }

        if (multiname->isRtns())
        {
            // intern the runtime namespace and copy to the temp multiname
            LIns* nsAtom = loadAtomRep(csp--);
            LIns* internNs = callIns(FUNCTIONID(internRtns), 2,
                env_param, nsAtom);

            stp(internNs, _tempname, offsetof(Multiname,ns), ACC_OTHER);
        }
        else
        {
            // copy the compile-time namespace to the temp multiname
            LIns* mSpace = InsConstPtr(multiname->ns);
            stp(mSpace, _tempname, offsetof(Multiname, ns), ACC_OTHER);
        }

        // Call initMultinameLate as the last step, since if a runtime
        // namespace is present, initMultinameLate may clobber it if a
        // QName is provided as index.
        if (nameAtom)
        {
            if (isDelete)
            {
                callIns(FUNCTIONID(initMultinameLateForDelete), 3,
                        env_param, _tempname, nameAtom);
            }
            else
            {
                callIns(FUNCTIONID(initMultinameLate), 3,
                        coreAddr, _tempname, nameAtom);
            }
        }

        return _tempname;
    }

    LIns* CodegenLIR::mopAddrToRangeCheckedRealAddrAndDisp(LIns* mopAddr, int32_t const size, int32_t* disp)
    {
        AvmAssert(size > 0);    // it's signed to help make the int promotion correct

        if (!mopsRangeCheckFilter) {
            // add a MopsRangeCheckFilter to the back end of the lirout pipeline, just after CseFilter.
            // fixme bug Bug 554030: We must put this after CseFilter and ExprFilter so that
            // the range-check expression using LIR_addi/LIR_subi are not modified (by ExprFilter)
            // and no not become referenced by other unrelated code (by CseFilter).
            AvmAssert(lirout == varTracker);
            mopsRangeCheckFilter = new (*alloc1) MopsRangeCheckFilter(redirectWriter->out, prolog, loadEnvDomainEnv());
            redirectWriter->out = mopsRangeCheckFilter;
        }

        // note, mopAddr and disp are both in/out parameters
        LInsp br = NULL;
        LInsp mopsMemoryBase = mopsRangeCheckFilter->emitRangeCheck(mopAddr, size, disp, br);
        if (br)
            patchLater(br, mop_rangeCheckFailed_label);


        // if mopAddr is a compiletime constant, we still have to do the range-check above
        // (since globalMemorySize can vary at runtime), but we might be able to encode
        // the entire address into the displacement (if any)...
        if (mopAddr->isImmI() && disp != NULL && sumFitsInInt32(*disp, mopAddr->immI()))
        {
            *disp += mopAddr->immI();
            return mopsMemoryBase;
        }

        // (yes, i2p, not u2p... it might legitimately be negative due to the
        // displacement optimization in emitCheck().)
        return binaryIns(LIR_addp, mopsMemoryBase, i2p(mopAddr));
    }

    LIns* CodegenLIR::loadEnvScope()
    {
        LIns* scope = prolog->env_scope;
        if (!scope)
        {
            prolog->env_scope = scope = prolog->insLoad(LIR_ldp, env_param, offsetof(MethodEnv, _scope), ACC_READONLY);
            verbose_only( if (vbNames) {
                vbNames->lirNameMap->addName(scope, "env_scope");
            })
            verbose_only( if (vbWriter) { vbWriter->flush(); } )
        }
        return scope;
    }

    LIns* CodegenLIR::loadEnvVTable()
    {
        LIns* vtable = prolog->env_vtable;
        if (!vtable)
        {
            LIns* scope = loadEnvScope();
            prolog->env_vtable = vtable = prolog->insLoad(LIR_ldp, scope, offsetof(ScopeChain, _vtable), ACC_READONLY);
            verbose_only( if (vbNames) {
                vbNames->lirNameMap->addName(vtable, "env_vtable");
            })
            verbose_only( if (vbWriter) { vbWriter->flush(); } )
        }
        return vtable;
    }

    LIns* CodegenLIR::loadEnvAbcEnv()
    {
        LIns* abcenv = prolog->env_abcenv;
        if (!abcenv)
        {
            LIns* scope = loadEnvScope();
            prolog->env_abcenv = abcenv = prolog->insLoad(LIR_ldp, scope, offsetof(ScopeChain, _abcEnv), ACC_READONLY);
            verbose_only( if (vbNames) {
                vbNames->lirNameMap->addName(abcenv, "env_abcenv");
            })
            verbose_only( if (vbWriter) { vbWriter->flush(); } )
        }
        return abcenv;
    }

    LIns* CodegenLIR::loadEnvDomainEnv()
    {
        LIns* domainenv = prolog->env_domainenv;
        if (!domainenv)
        {
            LIns* abcenv = loadEnvAbcEnv();
            prolog->env_domainenv = domainenv = prolog->insLoad(LIR_ldp, abcenv, offsetof(AbcEnv, m_domainEnv), ACC_READONLY);
            verbose_only( if (vbNames) {
                vbNames->lirNameMap->addName(domainenv, "env_domainenv");
            })
            verbose_only( if (vbWriter) { vbWriter->flush(); } )
        }
        return domainenv;
    }

    LIns* CodegenLIR::loadEnvToplevel()
    {
        LIns* toplevel = prolog->env_toplevel;
        if (!toplevel)
        {
            LIns* vtable = loadEnvVTable();
            prolog->env_toplevel = toplevel = prolog->insLoad(LIR_ldp, vtable, offsetof(VTable, _toplevel), ACC_READONLY);
            verbose_only( if (vbNames) {
                vbNames->lirNameMap->addName(toplevel, "env_toplevel");
            })
            verbose_only( if (vbWriter) { vbWriter->flush(); } )
        }
        return toplevel;
    }

    /**
     * given an object and type, produce code that loads the VTable for
     * the object.  Handles all types: primitive vables get loaded from
     * Toplevel, Object and * vtables get loaded by calling the toVTable() helper.
     * ScriptObject* vtables are loaded from the ScriptObject.
     */
    LIns* CodegenLIR::loadVTable(LIns* obj, Traits* t)
    {
        if (t && !t->isMachineType() && t != STRING_TYPE && t != NAMESPACE_TYPE && t != NULL_TYPE)
        {
            // must be a pointer to a scriptobject, and we've done the n
            // all other types are ScriptObject, and we've done the null check
            return loadIns(LIR_ldp, offsetof(ScriptObject, vtable), obj, ACC_READONLY);
        }

        LIns* toplevel = loadEnvToplevel();

        int offset;
        if (t == NAMESPACE_TYPE)    offset = offsetof(Toplevel, namespaceClass);
        else if (t == STRING_TYPE)  offset = offsetof(Toplevel, stringClass);
        else if (t == BOOLEAN_TYPE) offset = offsetof(Toplevel, booleanClass);
        else if (t == NUMBER_TYPE)  offset = offsetof(Toplevel, numberClass);
        else if (t == INT_TYPE)     offset = offsetof(Toplevel, intClass);
        else if (t == UINT_TYPE)    offset = offsetof(Toplevel, uintClass);
        else
        {
            // *, Object or Void
            LIns* atom = nativeToAtom(obj, t);
            return callIns(FUNCTIONID(toVTable), 2, toplevel, atom);
        }

        // now offset != -1 and we are returning a primitive vtable

        LIns* cc = loadIns(LIR_ldp, offset, toplevel, ACC_READONLY);
        LIns* cvtable = loadIns(LIR_ldp, offsetof(ClassClosure, vtable), cc, ACC_READONLY);
        return loadIns(LIR_ldp, offsetof(VTable, ivtable), cvtable, ACC_READONLY);
    }

    LIns* CodegenLIR::promoteNumberIns(Traits* t, int i)
    {
        if (t == NUMBER_TYPE)
        {
            return localGetf(i);
        }
        if (t == INT_TYPE || t == BOOLEAN_TYPE)
        {
            return i2dIns(localGet(i));
        }
        AvmAssert(t == UINT_TYPE);
        return ui2dIns(localGet(i));
    }

    /// set position of a label and patch all pending jumps to point here.
    void CodegenLIR::emitLabel(CodegenLabel& label) {
        varTracker->trackLabel(label, state);

        // patch all unpatched branches to this label
        LIns* labelIns = label.labelIns;
        bool jtbl_forward_target = false;
        for (Seq<InEdge>* p = label.unpatchedEdges; p != NULL; p = p->tail) {
            InEdge& patch = p->head;
            LIns* branchIns = patch.branchIns;
            if (branchIns->isop(LIR_jtbl)) {
                jtbl_forward_target = true;
                branchIns->setTarget(patch.index, labelIns);
            } else {
                AvmAssert(branchIns->isBranch() && patch.index == 0);
                branchIns->setTarget(labelIns);
            }
        }
        if (jtbl_forward_target) {
            // A jtbl (switch) jumps forward to here, creating a situation our
            // register allocator cannot handle; force regs to be loaded at the
            // start of this block.
            Ins(LIR_regfence);
        }

#ifdef NJ_VERBOSE
        if (vbNames && label.name)
            vbNames->lirNameMap->addName(label.labelIns, label.name);
#endif
    }

#ifdef DEBUGGER
    void CodegenLIR::emitSampleCheck()
    {
        /* @todo inline the sample check code, help!  */
        callIns(FUNCTIONID(sampleCheck), 1, coreAddr);
    }
#endif

#ifdef NJ_VERBOSE
    bool CodegenLIR::verbose()
    {
        return pool->isVerbose(VB_jit);
    }
#endif

    // emit a conditional branch to the given label.  If we have already emitted
    // code for that label then the branch is complete.  If not then add a patch
    // record to the label, which will patch the branch when the label position
    // is reached.  cond == NULL for unconditional branches (LIR_j).
    void CodegenLIR::branchToLabel(LOpcode op, LIns *cond, CodegenLabel& label) {
        if (cond) {
            if (!cond->isCmp()) {
                // branching on a non-condition expression, so test (v==0)
                // and invert the sense of the branch.
                cond = eqi0(cond);
                op = LOpcode(op ^ 1);
            }
            if (cond->isImmI()) {
                // the branch condition is constant so we're either always branching
                // or never branching.  handle each case.
                if ((op == LIR_jt && cond->immI()) || (op == LIR_jf && !cond->immI())) {
                    // taken
                    op = LIR_j;
                    cond = 0;
                }
                else {
                    // not taken - no code to emit.
                    return;
                }
            }
        }
        LIns* labelIns = label.labelIns;
        LIns* br = lirout->insBranch(op, cond, labelIns);
        if (br != NULL) {
            if (labelIns != NULL) {
                varTracker->checkBackEdge(label, state);
            } else {
                label.unpatchedEdges = new (*alloc1) Seq<InEdge>(InEdge(br), label.unpatchedEdges);
                varTracker->trackForwardEdge(label);
            }
        } else {
            // branch was optimized away.  do nothing.
        }
    }

    // emit a relative branch to the given ABC pc-offset by mapping pc_off
    // to a corresponding CodegenLabel, and creating a new one if necessary
    void CodegenLIR::branchToAbcPos(LOpcode op, LIns *cond, int pc_off) {
        CodegenLabel& label = getCodegenLabel(pc_off);
        branchToLabel(op, cond, label);
    }

    CodegenLabel& CodegenLIR::createLabel(const char* name) {
        return *(new (*alloc1) CodegenLabel(name));
    }

    CodegenLabel& CodegenLIR::createLabel(const char* prefix, int id) {
        CodegenLabel* label = new (*alloc1) CodegenLabel();
#ifdef NJ_VERBOSE
        if (vbNames) {
            char *name = new (*lir_alloc) char[VMPI_strlen(prefix)+16];
            VMPI_sprintf(name, "%s%d", prefix, id);
            label->name = name;
        }
#else
        (void) prefix;
        (void) id;
#endif
        return *label;
    }

    CodegenLabel& CodegenLIR::getCodegenLabel(int pc) {
        AvmAssert(state->verifier->hasFrameState(pc));
        if (!blockLabels)
            blockLabels = new (*alloc1) HashMap<int,CodegenLabel*>(*alloc1, state->verifier->getBlockCount());
        CodegenLabel* label = blockLabels->get(pc);
        if (!label) {
            label = new (*alloc1) CodegenLabel();
            blockLabels->put(pc, label);
        }
#ifdef NJ_VERBOSE
        if (!label->name && vbNames) {
            char *name = new (*lir_alloc) char[16];
            VMPI_sprintf(name, "B%d", pc);
            label->name = name;
        }
#endif
        return *label;
    }

    /// connect to a label for one entry of a switch
    void CodegenLIR::patchLater(LIns* jtbl, int pc_off, uint32_t index) {
        CodegenLabel& target = getCodegenLabel(pc_off);
        if (target.labelIns != 0) {
            jtbl->setTarget(index, target.labelIns);           // backward edge
            varTracker->checkBackEdge(target, state);
        } else {
            target.unpatchedEdges = new (*alloc1) Seq<InEdge>(InEdge(jtbl, index), target.unpatchedEdges);
            varTracker->trackForwardEdge(target);
        }
    }

    void CodegenLIR::patchLater(LIns *br, CodegenLabel &target) {
        if (!br) return; // occurs if branch was unconditional and thus never emitted.
        if (target.labelIns != 0) {
            br->setTarget(target.labelIns); // backwards edge
            varTracker->checkBackEdge(target, state);
        } else {
            target.unpatchedEdges = new (*alloc1) Seq<InEdge>(InEdge(br), target.unpatchedEdges);
            varTracker->trackForwardEdge(target);
        }
    }

    LIns* CodegenLIR::insAlloc(int32_t size) {
        return lirout->insAlloc(size >= 4 ? size : 4);
    }

    CodeMgr::CodeMgr() : codeAlloc(), bindingCaches(NULL)
    {
        verbose_only( log.lcbits = 0; )
    }

    void CodeMgr::flushBindingCaches()
    {
        // this clears vtable so all kObjectType receivers are invalidated.
        // of course, this field is also "tag" for primitive receivers,
        // but 0 is never a legal value there (and this is asserted when the tag is set)
        // so this should safely invalidate those as well (though we don't really need to invalidate them)
        for (BindingCache* b = bindingCaches; b != NULL; b = b->next)
            b->vtable = NULL;
    }

    // read all of in1, followed by all of in2
    class SeqReader: public LirFilter
    {
        LirReader r1, r2;
#ifdef _DEBUG
        ValidateReader v1, v2;
#endif
        LirFilter *p1, *p2;
    public:
        SeqReader(LIns* lastIns1, LIns* lastIns2)
            : LirFilter(NULL), r1(lastIns1), r2(lastIns2),
#ifdef _DEBUG
            v1(&r1), v2(&r2), p1(&v1), p2(&v2)
#else
            p1(&r1), p2(&r2)
#endif
        {
            in = p1;
        }

        LIns* read()
        {
            LIns* ins = in->read();
            if (ins->isop(LIR_start) && in == p1) {
                in = p2;
                ins = in->read();
            }
            return ins;
        }
    };

    void analyze_edge(LIns* label, nanojit::BitSet &livein,
                      HashMap<LIns*, nanojit::BitSet*> &labels,
                      InsList* looplabels)
    {
        nanojit::BitSet *lset = labels.get(label);
        if (lset) {
            livein.setFrom(*lset);
        } else {
            looplabels->add(label);
        }
    }

    // Treat addp(vars, const) as a load from vars[const]
    // for the sake of dead store analysis.
    void analyze_addp(LIns* ins, LIns* vars, nanojit::BitSet& varlivein)
    {
        AvmAssert(ins->isop(LIR_addp));
        if (ins->oprnd1() == vars && ins->oprnd2()->isImmP()) {
            AvmAssert(IS_ALIGNED(ins->oprnd2()->immP(), 8));
            int d = int(uintptr_t(ins->oprnd2()->immP()) >> 3);
            varlivein.set(d);
        }
    }

    void analyze_call(LIns* ins, LIns* catcher, LIns* vars, DEBUGGER_ONLY(bool haveDebugger, int dbg_framesize,)
            nanojit::BitSet& varlivein, HashMap<LIns*, nanojit::BitSet*> &varlabels,
            nanojit::BitSet& taglivein, HashMap<LIns*, nanojit::BitSet*> &taglabels)
    {
        if (!ins->callInfo()->_isPure) {
            if (catcher) {
                // non-cse call is like a conditional forward branch to the catcher label.
                // this could be made more precise by checking whether this call
                // can really throw, and only processing edges to the subset of
                // reachable catch blocks.  If we haven't seen the catch label yet then
                // the call is to an exception handling helper (eg beginCatch())
                // that won't throw.
                nanojit::BitSet *varlset = varlabels.get(catcher);
                if (varlset)
                    varlivein.setFrom(*varlset);
                nanojit::BitSet *taglset = taglabels.get(catcher);
                if (taglset)
                    taglivein.setFrom(*taglset);
            }
#ifdef DEBUGGER
            if (haveDebugger) {
                // all vars and scopes must be considered "read" by any call
                // the debugger can stop in.  The debugger also will access tags[].
                for (int i = 0, n = dbg_framesize; i < n; i++) {
                    varlivein.set(i);
                    taglivein.set(i);
                }
            }
#endif
        }
        else if (ins->callInfo() == FUNCTIONID(makeatom)) {
            // makeatom(core, &vars[index], tag[index]) => treat as load from &vars[index]
            LIns* varPtrArg = ins->arg(1);  // varPtrArg == vars, OR addp(vars, index)
            if (varPtrArg == vars)
                varlivein.set(0);
            else if (varPtrArg->isop(LIR_addp))
                analyze_addp(varPtrArg, vars, varlivein);
        }
    }

    void CodegenLIR::deadvars_analyze(Allocator& alloc,
            nanojit::BitSet& varlivein, HashMap<LIns*, nanojit::BitSet*> &varlabels,
            nanojit::BitSet& taglivein, HashMap<LIns*, nanojit::BitSet*> &taglabels)
    {
        LIns *catcher = this->catch_label.labelIns;
        InsList looplabels(alloc);

        verbose_only(int iter = 0;)
        bool again;
        do {
            again = false;
            varlivein.reset();
            taglivein.reset();
            SeqReader in(frag->lastIns, prologLastIns);
            for (LIns *i = in.read(); !i->isop(LIR_start); i = in.read()) {
                LOpcode op = i->opcode();
                switch (op) {
                case LIR_reti:
                CASE64(LIR_retq:)
                case LIR_retd:
                    varlivein.reset();
                    taglivein.reset();
                    break;
                CASE64(LIR_stq:)
                case LIR_sti:
                case LIR_std:
                case LIR_sti2c:
                    if (i->oprnd2() == vars) {
                        int d = i->disp() >> 3;
                        varlivein.clear(d);
                    } else if (i->oprnd2() == tags) {
                        int d = i->disp(); // 1 byte per tag
                        taglivein.clear(d);
                    }
                    break;
                case LIR_addp:
                    // treat pointer calculations into vars as a read from vars
                    analyze_addp(i, vars, varlivein);
                    break;
                CASE64(LIR_ldq:)
                case LIR_ldi:
                case LIR_ldd:
                case LIR_lduc2ui: case LIR_ldc2i:
                    if (i->oprnd1() == vars) {
                        int d = i->disp() >> 3;
                        varlivein.set(d);
                    }
                    else if (i->oprnd1() == tags) {
                        int d = i->disp(); // 1 byte per tag
                        taglivein.set(d);
                    }
                    break;
                case LIR_label: {
                    // we're at the top of a block, save livein for this block
                    // so it can be propagated to predecessors
                    nanojit::BitSet *var_lset = varlabels.get(i);
                    if (!var_lset) {
                        var_lset = new (alloc) nanojit::BitSet(alloc, framesize);
                        varlabels.put(i, var_lset);
                    }
                    if (var_lset->setFrom(varlivein) && !again) {
                        for (Seq<LIns*>* p = looplabels.get(); p != NULL; p = p->tail) {
                            if (p->head == i) {
                                again = true;
                                break;
                            }
                        }
                    }
                    nanojit::BitSet *tag_lset = taglabels.get(i);
                    if (!tag_lset) {
                        tag_lset = new (alloc) nanojit::BitSet(alloc, framesize);
                        taglabels.put(i, tag_lset);
                    }
                    if (tag_lset->setFrom(taglivein) && !again) {
                        for (Seq<LIns*>* p = looplabels.get(); p != NULL; p = p->tail) {
                            if (p->head == i) {
                                again = true;
                                break;
                            }
                        }
                    }
                    break;
                }
                case LIR_j:
                    // the fallthrough path is unreachable, clear it.
                    varlivein.reset();
                    taglivein.reset();
                    // fall through to other branch cases
                case LIR_jt:
                case LIR_jf:
                    // merge the LiveIn sets from each successor:  the fall
                    // through case (livein) and the branch case (lset).
                    analyze_edge(i->getTarget(), varlivein, varlabels, &looplabels);
                    analyze_edge(i->getTarget(), taglivein, taglabels, &looplabels);
                    break;
                case LIR_jtbl:
                    varlivein.reset(); // fallthrough path is unreachable, clear it.
                    taglivein.reset(); // fallthrough path is unreachable, clear it.
                    for (uint32_t j=0, n=i->getTableSize(); j < n; j++) {
                        analyze_edge(i->getTarget(j), varlivein, varlabels, &looplabels);
                        analyze_edge(i->getTarget(j), taglivein, taglabels, &looplabels);
                    }
                    break;
                CASE64(LIR_callq:)
                case LIR_calli:
                case LIR_calld:
                    analyze_call(i, catcher, vars, DEBUGGER_ONLY(haveDebugger, dbg_framesize,)
                            varlivein, varlabels, taglivein, taglabels);
                    break;
                }
            }
            verbose_only(iter++;)
        }
        while (again);

        // now make a final pass, modifying LIR to delete dead stores (make them LIR_neartramps)
        verbose_only( if (pool->isVerbose(LC_Liveness))
            AvmLog("killing dead stores after %d LA iterations.\n",iter);
        )
    }

    // return the previous instruction, handling skips appropriately
    static LIns* findPrevIns(LIns* ins)
    {
        // table of LIR instruction sizes (private to this file)
        // TODO this can go away if we turn this kill pass into a LirReader
        // and do the work inline with the assembly pass.
        static const uint8_t lirSizes[] = {
        #define OP___(op, number, repkind, retType, isCse) sizeof(LIns##repkind),
        #include "../nanojit/LIRopcode.tbl"
        #undef OP___
                0
        };
        LIns* prev = (LIns*) (uintptr_t(ins) - lirSizes[ins->opcode()]);
        // Ensure prev doesn't end up pointing to a skip.
        while (prev->isop(LIR_skip)) {
            NanoAssert(prev->prevLIns() != prev);
            prev = prev->prevLIns();
        }
        return prev;
    }

    void CodegenLIR::deadvars_kill(nanojit::BitSet& varlivein, HashMap<LIns*, nanojit::BitSet*> &varlabels,
            nanojit::BitSet& taglivein, HashMap<LIns*, nanojit::BitSet*> &taglabels)
    {
#ifdef NJ_VERBOSE
        LInsPrinter *printer = frag->lirbuf->printer;
        bool verbose = printer && pool->isVerbose(VB_jit);
        InsBuf b;
#endif
        LIns *catcher = this->catch_label.labelIns;
        varlivein.reset();
        taglivein.reset();
        bool tags_touched = false;
        bool vars_touched = false;
        SeqReader in(frag->lastIns, prologLastIns);
        for (LIns *i = in.read(); !i->isop(LIR_start); i = in.read()) {
            LOpcode op = i->opcode();
            switch (op) {
                case LIR_reti:
                CASE64(LIR_retq:)
                case LIR_retd:
                    varlivein.reset();
                    taglivein.reset();
                    break;
                CASE64(LIR_stq:)
                case LIR_sti:
                case LIR_std:
                case LIR_sti2c:
                    if (i->oprnd2() == vars) {
                        int d = i->disp() >> 3;
                        if (!varlivein.get(d)) {
                            verbose_only(if (verbose)
                                AvmLog("- %s\n", printer->formatIns(&b, i));)
                            // erase the store by rewriting it as a skip
                            LIns* prevIns = findPrevIns(i);
                            if (prologLastIns == i)
                                prologLastIns = prevIns;
                            i->initLInsSk(prevIns);
                            continue;
                        } else {
                            varlivein.clear(d);
                            vars_touched = true;
                        }
                    }
                    else if (i->oprnd2() == tags) {
                        int d = i->disp(); // 1 byte per tag
                        if (!taglivein.get(d)) {
                            verbose_only(if (verbose)
                                AvmLog("- %s\n", printer->formatIns(&b, i));)
                            // erase the store by rewriting it as a skip
                            LIns* prevIns = findPrevIns(i);
                            if (prologLastIns == i)
                                prologLastIns = prevIns;
                            i->initLInsSk(prevIns);
                            continue;
                        } else {
                            // keep the store
                            taglivein.clear(d);
                            tags_touched = true;
                        }
                    }
                    break;
                case LIR_addp:
                    // treat pointer calculations into vars as a read from vars
                    analyze_addp(i, vars, varlivein);
                    break;
                CASE64(LIR_ldq:)
                case LIR_ldi:
                case LIR_ldd:
                case LIR_lduc2ui: case LIR_ldc2i:
                    if (i->oprnd1() == vars) {
                        int d = i->disp() >> 3;
                        varlivein.set(d);
                    }
                    else if (i->oprnd1() == tags) {
                        int d = i->disp(); // 1 byte per tag
                        taglivein.set(d);
                    }
                    break;
                case LIR_label: {
                    // we're at the top of a block, save livein for this block
                    // so it can be propagated to predecessors
                    nanojit::BitSet *var_lset = varlabels.get(i);
                    AvmAssert(var_lset != 0); // all labels have been seen by deadvars_analyze()
                    var_lset->setFrom(varlivein);
                    nanojit::BitSet *tag_lset = taglabels.get(i);
                    AvmAssert(tag_lset != 0); // all labels have been seen by deadvars_analyze()
                    tag_lset->setFrom(taglivein);
                    break;
                }
                case LIR_j:
                    // the fallthrough path is unreachable, clear it.
                    varlivein.reset();
                    taglivein.reset();
                    // fall through to other branch cases
                case LIR_jt:
                case LIR_jf:
                    // merge the LiveIn sets from each successor:  the fall
                    // through case (live) and the branch case (lset).
                    analyze_edge(i->getTarget(), varlivein, varlabels, 0);
                    analyze_edge(i->getTarget(), taglivein, taglabels, 0);
                    break;
                case LIR_jtbl:
                    varlivein.reset();
                    taglivein.reset();
                    for (uint32_t j = 0, n = i->getTableSize(); j < n; j++) {
                        analyze_edge(i->getTarget(j), varlivein, varlabels, 0);
                        analyze_edge(i->getTarget(j), taglivein, taglabels, 0);
                    }
                    break;
                CASE64(LIR_callq:)
                case LIR_calli:
                case LIR_calld:
                    analyze_call(i, catcher, vars, DEBUGGER_ONLY(haveDebugger, dbg_framesize,)
                            varlivein, varlabels, taglivein, taglabels);
                    break;
            }
            verbose_only(if (verbose) {
                AvmLog("  %s\n", printer->formatIns(&b, i));
            })
        }
        // if we have not removed all stores to the tags array, mark it live
        // so its live range will span loops.
        if (tags_touched)
            livep(tags);
        if (vars_touched)
            livep(vars);
    }

    /*
     * this is iterative live variable analysis.  We walk backwards through
     * the code.  when we see a load, we mark the variable live, and when
     * we see a store, we mark it dead.  Dead stores are dropped, not returned
     * by read().
     *
     * at labels, we save the liveIn set associated with that label.
     *
     * at branches, we merge the liveIn sets from the fall through case (which
     * is the current set) and the branch case (which was saved with the label).
     * this filter can be run multiple times, which is required to pick up
     * loop-carried live variables.
     *
     * once the live sets are stable, the DeadVars.kill flag is set to cause the filter
     * to not only drop dead stores, but overwrite them as tramps so they'll be
     * ignored by any later passes even without using this filter.
     */

    void CodegenLIR::deadvars()
    {
        // allocator used only for duration of this phase.  no exceptions are
        // thrown while this phase runs, hence no try/catch is necessary.
        Allocator dv_alloc;

        // map of label -> bitset, tracking what is livein at each label.
        // populated by deadvars_analyze, then used by deadvars_kill
        // estimate number of required buckets based on the verifier's labelCount,
        // which is slightly below the actual # of labels in LIR.  being slightly low
        // is okay for a bucket hashtable.  note: labelCount is 0 for simple 1-block
        // methods, so use labelCount+1 as the estimate to ensure we have >0 buckets.
        HashMap<LIns*, nanojit::BitSet*> varlabels(dv_alloc, labelCount + 1);
        HashMap<LIns*, nanojit::BitSet*> taglabels(dv_alloc, labelCount + 1);

        // scratch bitset used by both dv_analyze and dv_kill.  Each resets
        // the bitset before using it.  creating it here saves one allocation.
        nanojit::BitSet varlivein(dv_alloc, framesize);
        nanojit::BitSet taglivein(dv_alloc, framesize);

        deadvars_analyze(dv_alloc, varlivein, varlabels, taglivein, taglabels);
        deadvars_kill(varlivein, varlabels, taglivein, taglabels);
    }

#ifdef AVMPLUS_JITMAX
    int jitcount = 0;
    int jitmin = 1;
    int jitmax = 0x7fffffff;
#endif

#ifdef NJ_VERBOSE
    void listing(const char* title, AvmLogControl &log, Fragment* frag, LIns* prologLastIns)
    {
        SeqReader seqReader(frag->lastIns, prologLastIns);
        Allocator lister_alloc;
        ReverseLister lister(&seqReader, lister_alloc, frag->lirbuf->printer, &log, title);
        for (LIns* ins = lister.read(); !ins->isop(LIR_start); ins = lister.read())
        {}
        lister.finish();
    }
#endif

    void CodegenLIR::emitMD()
    {
        deadvars();  // deadvars_kill() will add livep(vars) or livep(tags) if necessary

        // do this very last so it's after livep(vars)
        frag->lastIns = livep(undefConst);

        PERFM_NTPROF("compile");
        mmfx_delete( alloc1 );
        alloc1 = NULL;

        CodeMgr *mgr = pool->codeMgr;
        #ifdef NJ_VERBOSE
        if (pool->isVerbose(LC_ReadLIR)) {
            StUTF8String name(info->format(core));
            char *title = new (*lir_alloc) char[VMPI_strlen(name.c_str()) + 20];
            VMPI_sprintf(title, "Final LIR %s", name.c_str());
            listing(title, mgr->log, frag, prologLastIns);
        }
        if (pool->isVerbose(LC_Liveness)) {
            Allocator live_alloc;
            SeqReader in(frag->lastIns, prologLastIns);
            nanojit::live(&in, live_alloc, frag, &mgr->log);
        }
        if (pool->isVerbose(LC_AfterDCE | LC_Native)) {
            StUTF8String name(info->format(core));
            mgr->log.printf("jit-assembler %s\n", name.c_str());
        }
        #endif

        Assembler *assm = new (*lir_alloc) Assembler(mgr->codeAlloc, mgr->allocator, *lir_alloc, core, &mgr->log, core->config.njconfig);
        verbose_only( StringList asmOutput(*lir_alloc); )
        verbose_only( assm->_outputCache = &asmOutput; )

        assm->beginAssembly(frag);
        SeqReader seqReader(frag->lastIns, prologLastIns);
        assm->assemble(frag, &seqReader);
        assm->endAssembly(frag);
        PERFM_TPROF_END();

        verbose_only(
            assm->_outputCache = 0;
            for (Seq<char*>* p = asmOutput.get(); p != NULL; p = p->tail) {
                assm->outputf("%s", p->head);
            }
        );

        PERFM_NVPROF("IR-bytes", frag->lirbuf->byteCount());
        PERFM_NVPROF("IR", frag->lirbuf->insCount());

        bool keep = //!info->hasExceptions() &&
            !assm->error();
#ifdef AVMPLUS_JITMAX
        jitcount++;
        keep = keep && (jitcount >= jitmin && jitcount <= jitmax);
        //AvmLog(stderr, "jitcount %d keep %d\n", jitcount, (int)keep);
#endif
        //_nvprof("keep",keep);
        if (keep) {
#if defined AVMPLUS_JITMAX && defined NJ_VERBOSE
            if (verbose())
                AvmLog("keeping %d, loop=%d\n", jitcount, assm->hasLoop);
#endif
            // save pointer to generated code
            union {
                GprMethodProc fp;
                void *vp;
            } u;
            u.vp = frag->code();
#ifdef INTEL_VTUNE
			info->setNativeImpl(u.fp, assm->codeList->size());
#else
            info->setNativeImpl(u.fp);
#endif
            // mark method as been JIT'd
            info->_flags |= MethodInfo::JIT_IMPL;
            InvokerCompiler::initCompilerHook(info);
            _nvprof("JIT method bytes", CodeAlloc::size(assm->codeList));
        } else {
#if defined AVMPLUS_JITMAX && defined NJ_VERBOSE
            if (verbose())
                AvmLog("reverting to interpreter %d assm->error %d \n", jitcount, assm->error());
#endif
            mgr->codeAlloc.freeAll(assm->codeList);
            // assm puked, or we did something untested, so interpret.
            overflow = true;
            if (assm->codeList)
                mgr->codeAlloc.freeAll(assm->codeList);
            PERFM_NVPROF("lir-error",1);
        }
	}

    REALLY_INLINE BindingCache::BindingCache(const Multiname* name, BindingCache* next)
        : name(name), next(next)
    {}

    REALLY_INLINE CallCache::CallCache(const Multiname* name, BindingCache* next)
        : BindingCache(name, next), call_handler(callprop_miss)
    {}

    REALLY_INLINE GetCache::GetCache(const Multiname* name, BindingCache* next)
        : BindingCache(name, next), get_handler(getprop_miss)
    {}

    REALLY_INLINE SetCache::SetCache(const Multiname* name, BindingCache* next)
        : BindingCache(name, next), set_handler(setprop_miss)
    {}

    template <class C>
    C* CacheBuilder<C>::findCacheSlot(const Multiname* name)
    {
        for (Seq<C*> *p = caches.get(); p != NULL; p = p->tail)
            if (p->head->name == name)
                return p->head;
        return NULL;
    }

    // The cache structure is expected to be small in the normal case, so use a
    // linear list.  For some programs, notably classical JS programs, it may however
    // be larger, and we may need a more sophisticated structure.
    template <class C>
    C* CacheBuilder<C>::allocateCacheSlot(const Multiname* name)
    {
        C* c = findCacheSlot(name);
        if (!c) {
            _nvprof("binding cache bytes", sizeof(C));
            c = new (codeMgr.allocator) C(name, codeMgr.bindingCaches);
            codeMgr.bindingCaches = c;
            caches.add(c);
        }
        return c;
    }

    LirHelper::LirHelper(PoolObject* pool) :
        pool(pool),
        core(pool->core),
        alloc1(mmfx_new(Allocator())),
        lir_alloc(mmfx_new(Allocator()))
    { }

    LirHelper::~LirHelper()
    {
        cleanup();
    }

    void LirHelper::cleanup()
    {
        mmfx_delete( alloc1 );
        alloc1 = NULL;
        mmfx_delete( lir_alloc );
        lir_alloc = NULL;
        pool->codeMgr->codeAlloc.markAllExec();
    }

    // check valid pointer and unbox it (returns ScriptObject*)
    LIns* LirHelper::downcast_obj(LIns* atom, LIns* env, Traits* t)
    {
        callIns(FUNCTIONID(coerceobj_atom), 3, env, atom, InsConstPtr(t));
        return andp(atom, ~7);
    }
}

namespace nanojit
{
    int StackFilter::getTop(LInsp /*br*/) {
        AvmAssert(false);
        return 0;
    }

    #ifdef NJ_VERBOSE
    void LInsPrinter::formatGuard(InsBuf*, LIns*) {
        AvmAssert(false);
    }
    void LInsPrinter::formatGuardXov(InsBuf*, LIns*) {
        AvmAssert(false);
    }
    #endif

    void* Allocator::allocChunk(size_t size) {
        return mmfx_alloc(size);
    }

    void Allocator::freeChunk(void* p) {
        return mmfx_free(p);
    }

    void Allocator::postReset() {
    }

    void* CodeAlloc::allocCodeChunk(size_t nbytes) {
        return VMPI_allocateCodeMemory(nbytes);
    }

    void CodeAlloc::freeCodeChunk(void* addr, size_t nbytes) {
        VMPI_freeCodeMemory(addr, nbytes);
    }

    void CodeAlloc::markCodeChunkExec(void* addr, size_t nbytes) {
        //printf("protect   %d %p\n", (int)nbytes, addr);
        VMPI_makeCodeMemoryExecutable(addr, nbytes, true); // RX
    }

    void CodeAlloc::markCodeChunkWrite(void* addr, size_t nbytes) {
        //printf("unprotect %d %p\n", (int)nbytes, addr);
        VMPI_makeCodeMemoryExecutable(addr, nbytes, false); // RW
    }
}

//
// JIT compiler for invoker stubs
//
namespace avmplus
{
    void InvokerCompiler::initCompilerHook(MethodInfo* method)
    {
        MethodSignaturep ms = method->getMethodSignature();
        int32_t rest_offset = ms->rest_offset();
        int32_t param_count = ms->param_count();
        if (rest_offset > int32_t((param_count+1) * sizeof(Atom)) &&
            method->needRestOrArguments()) {
            // situation: natively represented args need more space than provided,
            // and this method uses varargs (rest args or arguments array); this means
            // the declared args must grow, which requires allocating an unknown amount
            // of space since we don't know (at compile time) how many extra varags are present.
            //
            // punt for now.  to better handle this in the future, the cases are:
            //
            // jit function with rest args:
            //   the prolog will create an array with these extra args, so all
            //   we should need is a way to pass in the pointer to them.  (tweak ABI).
            //
            // jit function that needs arguments:
            //   the prolog will create an array with all the args re-boxed as Atom,
            //   plus the extra ones.  (the unboxed copies are still available in the callee).
            //
            // native function with rest args:
            //   the native function abi passes rest args via (argc,Atom*) parameters
            //   which we could adapt to here without shifting or copying, if we could
            //   bypass the normal Gpr/FprMethodProc ABI.
            //
            // native function with arguments:
            //   doesn't happen.  native functions only support rest args.
            //
            // Given the current JIT and native ABI, we can't support shifting and
            // copying the extra unknown number of args.  With changes to the native
            // ABI, we could pass a reference to the extra args without any copying.
            return;
        }

        // install hook that jit-compiles coerceEnter on second call.
        method->_invoker = &InvokerCompiler::jitInvokerNext;
    }

    Atom InvokerCompiler::jitInvokerNext(MethodEnv* env, int argc, Atom* args)
    {
        env->method->setNativeImpl(env->method->_implGPR); // also resets invoker
        AtomMethodProc invoker = env->method->_invoker; // generic stub
        env->method->_invoker = jitInvokerNow; // install stub to compile on next call
        return invoker(env, argc, args);
    }

    // first call after compiling method; compile a custom invoker and run it
    Atom InvokerCompiler::jitInvokerNow(MethodEnv* env, int argc, Atom* args)
    {
        env->method->setNativeImpl(env->method->_implGPR); // also resets invoker
        AtomMethodProc invoker = compile(env->method);
        if (invoker) {
            // success: install generated invoker
            env->method->_invoker = invoker;
        } else {
            // fail: use generic invoker from now on
            invoker = env->method->_invoker;
        }
        return invoker(env, argc, args);
    }

    // compiler driver
    AtomMethodProc InvokerCompiler::compile(MethodInfo* method)
    {
        InvokerCompiler compiler(method);
        compiler.generate_lir();
        return (AtomMethodProc) compiler.assemble();
    }

    InvokerCompiler::InvokerCompiler(MethodInfo* method)
        : LirHelper(method->pool())
        , method(method)
        , ms(method->getMethodSignature())
        , maxargs_br(NULL)
        , minargs_br(NULL)
    {
        this->method = method;
        this->ms = method->getMethodSignature();

        initCodeMgr(method->pool());
        frag = new (*lir_alloc) Fragment(0 verbose_only(, 0));
        LirBuffer* lirbuf = frag->lirbuf = new (*lir_alloc) LirBuffer(*lir_alloc);
        lirbuf->abi = ABI_CDECL;
        LirWriter* lirout = new (*alloc1) LirBufWriter(lirbuf, core->config.njconfig);
        verbose_only(
            if (verbose()) {
                lirbuf->printer = new (*lir_alloc) LInsPrinter(*lir_alloc);
                lirbuf->printer->addrNameMap->addAddrRange(pool->core, sizeof(AvmCore), 0, "core");
            }
        )
        debug_only(
            lirout = validate2 = new (*alloc1) ValidateWriter(lirout, lirbuf->printer, "InvokerCompiler");
        )
        verbose_only(
            if (verbose()) {
                CodeMgr *codeMgr = method->pool()->codeMgr;
                core->console << "compileInvoker " << method << "\n";
                core->console <<
                    " required=" << ms->requiredParamCount() <<
                    " optional=" << (ms->param_count() - ms->requiredParamCount()) << "\n";
                lirout = new (*alloc1) VerboseWriter(*alloc1, lirout, lirbuf->printer, &codeMgr->log);
            }
        )
#if defined(NANOJIT_ARM)
        if (core->config.njconfig.soft_float)
        {
            lirout = new (*alloc1) SoftFloatFilter(lirout);
        }
#endif
        // add other LirWriters here
        this->lirout = lirout;
        emitStart(*alloc1, lirbuf, lirout);
    }

    // recipe for an invoke wrapper:
    //    Atom <generated invoker>(MethodEnv* env, int argc, Atom* argv) {
    //        1. check argc:            env->startCoerce(argc, env->get_ms());
    //        2. unbox args:            unboxCoerceArgs, unrolled for each arg
    //        3. return box(call(...))
    //    }

    // jit-compile an invoker for mi
    void InvokerCompiler::generate_lir()
    {
        // invoker params
        LIns* env_param = param(0, "env");
        LIns* argc_param = p2i(param(1, "argc"));
        LIns* args_param = param(2, "args");
        coreAddr = InsConstPtr(core);

        // if unboxing args will make them expand, allocate more space.
        int32_t rest_offset = ms->rest_offset();
        if (rest_offset > int32_t((ms->param_count()+1)*sizeof(Atom))) {
            AvmAssert(!method->needRestOrArguments());
            args_out = lirout->insAlloc(rest_offset);
        } else {
            // we can do in-place unboxing of args.
            args_out = args_param;
        }

        // 1. check argc
        emit_argc_check(argc_param);

        // 2. unbox & coerce args
        downcast_args(env_param, argc_param, args_param);

        // 3. call, box result, return atom
        call_method(env_param, argc_param);

        // error handler for argc error
        if (minargs_br || maxargs_br) {
            LIns* errlabel = label();
            if (minargs_br) minargs_br->setTarget(errlabel);
            if (maxargs_br) maxargs_br->setTarget(errlabel);
            callIns(FUNCTIONID(argcError), 2, env_param, argc_param);
        }

        // mark the endpoint of generated LIR with an instruction the Assembler allows at the end
        frag->lastIns = livep(env_param);

        // we're done with LIR generation, free up what we can.
        mmfx_delete(alloc1);
        alloc1 = NULL;
    }

    void InvokerCompiler::emit_argc_check(LIns* argc_param)
    {
        int min_argc = ms->requiredParamCount();
        int param_count = ms->param_count();
        if (min_argc == param_count && !ms->argcOk(param_count + 1)) {
            // exactly param_count args required
            // if (argc != param_count) goto error
            maxargs_br = jnei(argc_param, param_count);
        } else {
            if (!ms->argcOk(param_count+1)) {
                // extra params are not allowed, must check for max args
                // if (argc > param_count) goto error
                maxargs_br = jgti(argc_param, param_count);
            }
            if (min_argc > 0) {
                // at least 1 param is required, so check
                // if (argc < min_argc) goto error
                minargs_br = jlti(argc_param, min_argc);
            }
        }
    }

    void InvokerCompiler::downcast_arg(int i, int offset, LIns* env_param, LIns* args_param)
    {
        BuiltinType bt = ms->paramTraitsBT(i);
        if (bt != BUILTIN_any) {
            LIns* atom = ldp(args_param, i*sizeof(Atom), ACC_OTHER);
            LIns* native = downcast_expr(atom, ms->paramTraits(i), env_param);
            lirout->insStore(native, args_out, offset, ACC_OTHER);
        } else if (copyArgs()) {
            LIns* atom = ldp(args_param, i*sizeof(Atom), ACC_OTHER);
            lirout->insStore(atom, args_out, offset, ACC_OTHER);
        }
    }

    int32_t InvokerCompiler::argSize(int i)
    {
        return ms->paramTraitsBT(i) == BUILTIN_number ? sizeof(double) : sizeof(Atom);
    }

    void InvokerCompiler::downcast_args(LIns* env_param, LIns* argc_param, LIns* args_param)
    {
        // the receiver arg (arg0) only needs to be unboxed, not coerced
        verbose_only( if (verbose())
            core->console << "unbox arg 0 " << ms->paramTraits(0) << "\n";
        )
        LIns* atom = ldp(args_param, 0, ACC_OTHER);
        LIns* native = atomToNative(ms->paramTraitsBT(0), atom);
        if (native != atom || copyArgs())
            lirout->insStore(native, args_out, 0, ACC_OTHER);
        int offset = argSize(0);

        // the required args need to be coerced and unboxed
        int i;
        int required_count = ms->requiredParamCount();
        for (i = 1; i <= required_count; i++) {
            verbose_only( if (verbose())
                core->console << "arg " << i << " " << ms->paramTraits(i) << "\n";
            )
            downcast_arg(i, offset, env_param, args_param);
            offset += argSize(i);
        }

        // optional args also need coercing and unboxing when they're present
        int param_count = ms->param_count();
        if (required_count < param_count) {
            int optional_count = param_count - required_count;
            int branch_count = 0;
            LIns** branches = new (*alloc1) LIns*[optional_count];
            for (; i <= param_count; i++) {
                verbose_only( if (verbose())
                    core->console << "optional arg " << i << " " << ms->paramTraits(i) << "\n";
                )
                // if (argc < i) { goto done }
                branches[branch_count++] = jlti(argc_param, i);
                downcast_arg(i, offset, env_param, args_param);
                offset += argSize(i);
            }
            if (branch_count > 0) {
                // done: patch all the optional-arg branches
                LIns* done_label = label();
                for (i = 0; i < branch_count; i++)
                    branches[i]->setTarget(done_label);
            }
        }
    }

    void* InvokerCompiler::assemble()
    {
        CodeMgr* codeMgr = method->pool()->codeMgr;

        verbose_only(if (pool->isVerbose(LC_Liveness)) {
            Allocator live_alloc;
            LirReader in(frag->lastIns);
            nanojit::live(&in, live_alloc, frag, &codeMgr->log);
        })

        Assembler *assm = new (*lir_alloc) Assembler(codeMgr->codeAlloc, codeMgr->allocator, *lir_alloc,
            core, &codeMgr->log, core->config.njconfig);
        verbose_only( StringList asmOutput(*lir_alloc); )
        verbose_only( assm->_outputCache = &asmOutput; )
        LirReader bufreader(frag->lastIns);
        assm->beginAssembly(frag);
        assm->assemble(frag, &bufreader);
        assm->endAssembly(frag);

        verbose_only(
            assm->_outputCache = 0;
            for (Seq<char*>* p = asmOutput.get(); p != NULL; p = p->tail)
                assm->outputf("%s", p->head);
        );
        if (!assm->error()) {
            if (method->isNative()) {
                _nvprof("C++ invoker bytes", CodeAlloc::size(assm->codeList));
            } else {
                _nvprof("JIT invoker bytes", CodeAlloc::size(assm->codeList));
            }
            return frag->code();
        } else {
            return NULL;
        }
    }

    void InvokerCompiler::call_method(LIns* env_param, LIns* argc_param)
    {
        // We know we've called the method at least once, so method->implGPR pointer is correct.
        CallInfo* call = (CallInfo*) lir_alloc->alloc(sizeof(CallInfo));
        call->_isPure = 0;
        call->_storeAccSet = ACC_STORE_ANY;
        call->_abi = ABI_FUNCTION;
        verbose_only( if (verbose()) {
            StUTF8String name(method->getMethodName());
            char *namestr = new (*lir_alloc) char[VMPI_strlen(name.c_str())+1];
            VMPI_strcpy(namestr, name.c_str());
            call->_name = namestr;
        })
        switch (ms->returnTraitsBT()) {
        case BUILTIN_number:
            call->_address = (uintptr_t) method->implFPR();
            call->_typesig = SIG3(F,P,I,P);
            break;
        case BUILTIN_int: case BUILTIN_uint: case BUILTIN_boolean:
            call->_address = (uintptr_t) method->implGPR();
            call->_typesig = SIG3(I,P,I,P);
            break;
        default:
            call->_address = (uintptr_t) method->implGPR();
            call->_typesig = SIG3(A,P,I,P);
            break;
        }
        LIns* result = callIns(call, 3, env_param, argc_param, args_out);
        livep(args_out);
        // box and return the result
        retp(nativeToAtom(result, ms->returnTraits()));
    }

    LIns* InvokerCompiler::downcast_expr(LIns* atom, Traits* t, LIns* env)
    {
        switch (bt(t)) {
        case BUILTIN_object:
            // return (atom == undefinedAtom) ? nullObjectAtom : atom;
            return choose(eqp(atom, undefinedAtom), nullObjectAtom, atom);
        case BUILTIN_int:
            return i2p(callIns(FUNCTIONID(integer), 1, atom));
        case BUILTIN_uint:
            return ui2p(callIns(FUNCTIONID(toUInt32), 1, atom));
        case BUILTIN_number:
            return callIns(FUNCTIONID(number), 1, atom);
        case BUILTIN_boolean:
            return ui2p(callIns(FUNCTIONID(boolean), 1, atom));
        case BUILTIN_string:
            return callIns(FUNCTIONID(coerce_s), 2, InsConstPtr(t->core), atom);
        case BUILTIN_namespace:
            return andp(callIns(FUNCTIONID(coerce), 3, env, atom, InsConstPtr(t)), ~7);
        case BUILTIN_void:
        case BUILTIN_null:
        case BUILTIN_any:
            AvmAssert(false);
        default:
            return downcast_obj(atom, env, t);
        }
    }

#ifdef NJ_VERBOSE
    bool InvokerCompiler::verbose()
    {
        return method->pool()->isVerbose(VB_jit);
    }
#endif
}

#endif // FEATURE_NANOJIT
