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
#include "../vprof/vprof.h"

namespace avmplus
{
#ifdef VMCFG_VERIFYALL
    class VerifyallWriter : public NullWriter {
        MethodInfo *info;
        AvmCore *core;

    public:
        VerifyallWriter(MethodInfo *info, CodeWriter *coder)
            : NullWriter(coder)
            , info(info) {
            core = info->pool()->core;
        }

        void write (const FrameState* state, const byte *pc, AbcOpcode opcode, Traits *type) {
            if (opcode == OP_newactivation)
                core->enqTraits(type);
            coder->write(state, pc, opcode, type);
        }

        void writeOp1(const FrameState* state, const byte *pc, AbcOpcode opcode, uint32_t opd1, Traits *type) {
            if (opcode == OP_newfunction) {
                MethodInfo *f = info->pool()->getMethodInfo(opd1);
                AvmAssert(f->declaringTraits() == type);
                core->enqFunction(f);
                core->enqTraits(type);
            }
            else if (opcode == OP_newclass) {
                core->enqTraits(type);
                core->enqTraits(type->itraits);
            }
            coder->writeOp1(state, pc, opcode, opd1, type);
        }
    };
#endif // VMCFG_VERIFYALL

#ifdef VMCFG_WORDCODE
    inline WordOpcode wordCode(AbcOpcode opcode) {
        return (WordOpcode)opcodeInfo[opcode].wordCode;
    }
#endif

    Verifier::Verifier(MethodInfo* info, Toplevel* toplevel, AbcEnv* abc_env
#ifdef AVMPLUS_VERBOSE
        , bool secondTry
#endif
        ) : tryFrom(NULL), tryTo(NULL)
          , ms(info->getMethodSignature())
          , worklist(NULL)
          , blockStates(NULL)
    {
#ifdef AVMPLUS_VERBOSE
        this->secondTry = secondTry;
#endif
        this->info   = info;
        this->core   = info->pool()->core;
        this->pool   = info->pool();
        this->toplevel = toplevel;
        this->abc_env  = abc_env;

        // do these checks early before we allocate any resources.
        if (!info->abc_body_pos()) {
            // no body was supplied in abc
            toplevel->throwVerifyError(kNotImplementedError, core->toErrorString(info));
        }
        if (info->declaringTraits() == NULL) {
            // scope hasn't been captured yet.
            verifyFailed(kCannotVerifyUntilReferencedError);
        }

        max_stack = ms->max_stack();
        local_count = ms->local_count();
        max_scope = ms->max_scope();

        stackBase = scopeBase + max_scope;
        frameSize = stackBase + max_stack;
        state = NULL;

        #ifdef AVMPLUS_VERBOSE
        verbose = pool->isVerbose(VB_verify);
        #endif

        #ifdef VMCFG_PRECOMP_NAMES
        pool->initPrecomputedMultinames();
        #endif
    }

    Verifier::~Verifier()
    {
        mmfx_delete( this->state );
        if (blockStates) {
            for(int i = 0, n=blockStates->size(); i < n; i++)
                mmfx_delete( blockStates->at(i) );
            delete blockStates;
        }
    }

    void Verifier::parseBodyHeader()
    {
        // note: reading of max_stack, etc (and validating the values)
        // is handled by MethodInfo::resolveSignature.
        const byte* pos = info->abc_body_pos();
        AvmCore::skipU32(pos, 4);
        code_length = AvmCore::readU32(pos);
        code_pos = pos;
        pos += code_length;
    }

    // ScopeWriter implements the scope-type-capturing functionality
    // for OP_newfunction and OP_newclass, as well as error checking
    // to handle illegally capturing incompatible scopes for the same
    // function.  All other opcodes are ignored.  Expected to be used
    // in the verifier's phase 2 when each bytecode is only visited once.
    class ScopeWriter: public NullWriter
    {
        MethodInfo* info;
        Toplevel* toplevel;
    public:
        ScopeWriter(CodeWriter* coder, MethodInfo* info, Toplevel* toplevel)
            : NullWriter(coder), info(info), toplevel(toplevel)
        {}

        void writeOp1(const FrameState* state, const byte* pc, AbcOpcode opcode, uint32_t imm30, Traits* type)
        {
            if (opcode == OP_newfunction) {
                PoolObject* pool = info->pool();
                AvmCore* core = pool->core;
                const ScopeTypeChain* scope = info->declaringScope();
                MethodInfo* f = pool->getMethodInfo(imm30);
                Traits* ftraits = core->traits.function_itraits;
                const ScopeTypeChain* fscope = ScopeTypeChain::create(core->GetGC(), ftraits, scope, state, NULL, NULL);
                // Duplicate function definitions aren't strictly legal, but can occur
                // in otherwise "well formed" ABC due to old, buggy versions of ASC.
                // Specifically, code of the form
                //
                //    public function simpleTest():void
                //    {
                //      var f:Function = function():void { }
                //      f();
                //      f = function functwo (x):void { }
                //      f(8);
                //    }
                //
                // could cause the second interior function ("functwo") to include a bogus, unused OP_newfunction
                // call to itself inside the body of functwo. This caused the scope to get reinitialized
                // and generally caused havok. However, we want to allow existing code of this form to continue
                // to work, so check to see if we already have a declaringScope, and if so, require that
                // it match this one.
                const ScopeTypeChain* curScope = f->declaringScope();
                if (curScope != NULL)
                {
                    if (!curScope->equals(fscope))
                    {
                        // if info->method_id() == imm30, f == info, and therefore
                        // curScope == scope -- don't redefine, don't fail verification,
                        // just accept it. see https://bugzilla.mozilla.org/show_bug.cgi?id=544370
                        if (info->method_id() != int32_t(imm30))
                            toplevel->throwVerifyError(kCorruptABCError);

                        AvmAssert(curScope->equals(scope));
                    }
                    AvmAssert(f->isResolved());
                }
                else
                {
                    f->makeIntoPrototypeFunction(toplevel, fscope);
                }

                #ifdef AVMPLUS_VERBOSE
                if (state->verifier->verbose)
                    state->verifier->printScope("function-scope", fscope);
                #endif
            } else if (opcode == OP_newclass) {
                PoolObject* pool = info->pool();
                AvmCore* core = pool->core;
                const ScopeTypeChain* scope = info->declaringScope();
                Traits* ctraits = type;
                // the actual result type will be the static traits of the new class.
                // make sure the traits came from this pool.  they have to because
                // the class_index operand is resolved from the current pool.
                AvmAssert(ctraits->pool == pool);
                Traits *itraits = ctraits->itraits;

                // add a type constraint for the "this" scope of static methods
                const ScopeTypeChain* cscope = ScopeTypeChain::create(core->GetGC(), ctraits, scope, state, NULL, ctraits);

                if (state->scopeDepth > 0)
                {
                    // innermost scope must be the base class object or else createInstance()
                    // will malfunction because it will use the wrong [base] class object to
                    // construct the instance.  See ScriptObject::createInstance()
                    Traits* baseCTraits = state->scopeValue(state->scopeDepth-1).traits;
                    if (!baseCTraits || baseCTraits->itraits != itraits->base)
                        state->verifier->verifyFailed(kCorruptABCError);
                }

                // add a type constraint for the "this" scope of instance methods
                const ScopeTypeChain* iscope = ScopeTypeChain::create(core->GetGC(), itraits, cscope, NULL, ctraits, itraits);

                ctraits->resolveSignatures(toplevel);
                itraits->resolveSignatures(toplevel);

                const ScopeTypeChain *cur_cscope = ctraits->declaringScope();
                const ScopeTypeChain *cur_iscope = itraits->declaringScope();
                if (!cur_cscope) {
                    // first time we have seen this class, capture scope types
                    ctraits->setDeclaringScopes(cscope);
                    itraits->setDeclaringScopes(iscope);
                } else {
                    // we have captured a scope already for this class.  it better match!
                    if (!cur_cscope->equals(cscope) ||
                        !cur_iscope ||
                        !cur_iscope->equals(iscope)) {
                        toplevel->throwVerifyError(kCorruptABCError);
                    }
                    // use the old ScopeTypeChains, discard the new ones
                    cscope = cur_cscope;
                    iscope = cur_iscope;
                }
                #ifdef AVMPLUS_VERBOSE
                if (state->verifier->verbose)
                    state->verifier->printScope("class-scope", cscope);
                #endif
            }
            coder->writeOp1(state, pc, opcode, imm30, type);
        }
    };

    /**
     * (done) branches stay in code block
     * (done) branches end on even instr boundaries
     * (done) all local var operands stay inside [0..max_locals-1]
     * (done) no illegal opcodes
     * (done) cpool refs are inside [1..cpool_size-1]
     * (done) converging paths have same stack depth
     * (done) operand stack stays inside [0..max_stack-1]
     * (done) locals defined before use
     * (done) scope stack stays bounded
     * (done) getScopeObject never exceeds [0..info->maxScopeDepth()-1]
     * (done) global slots limits obeyed [0..var_count-1]
     * (done) callstatic method limits obeyed [0..method_count-1]
     * (done) cpool refs are correct type
     * (done) make sure we don't fall off end of function
     * (done) slot based ops are ok (slot must be legal)
     * (done) propref ops are ok: usage on actual type compatible with ref type.
     * dynamic lookup ops are ok (type must not have that binding & must be dynamic)
     * dont access superclass state in ctor until super ctor called.
     *
     * pipeline todos:
     * - early binding
     * - copy propagation
     *
     * @param pool
     * @param info
     */
    // Sun's C++ compiler wants "volatile" here because the declaration has it
    // Presumably it's here to remove a warning about variable clobbered by longjmp
    void Verifier::verify(CodeWriter * volatile emitter)
    {
        SAMPLE_FRAME("[verify]", core);
        PERFM_NVPROF("abc-bytes", code_length);

        TRY(core, kCatchAction_Rethrow) {

        CodeWriter stubWriter;
        CodeWriter* coder = &stubWriter;

        #ifdef AVMPLUS_VERBOSE
        if (verbose)
            core->console << "\ntypecheck " << info << '\n';
        secondTry = false;
        #endif

        // Verify in two passes.  Phase 1 does type modelling and
        // iterates to a fixed point to determine the types and nullability
        // of each frame variable at branch targets.  Phase 2 includes the
        // emitter and ScopeWriter, and visits opcodes in linear order.
        // Errors detected by these additional CodeWriters can be reported
        // in phase 2.  In each phase, the CodeWriter protocol is obeyed:
        // writePrologue(), visits to explicit and implicit operations using
        // other writeXXX() methods, then writeEpilogue().

        emitPass = false;
        // phase 1 - iterate to a fixed point
        this->coder = coder;
        parseBodyHeader();          // set code_pos & code_length
        parseExceptionHandlers();   // resolve catch block types
        checkParams();
        #ifdef AVMPLUS_VERBOSE
        if (verbose) {
            printScope("outer-scope", info->declaringScope());
            StringBuffer buf(core);
            printState(buf, state);
        }
        #endif
        coder->writePrologue(state, code_pos);
        if (code_length > 0 && code_pos[0] == OP_label) {
            // a reachable block starts at code_pos; explicitly create it,
            // which puts it on the worklist.
            checkTarget(code_pos-1, code_pos);
        } else {
            // inital sequence of code is only reachable from procedure
            // entry, no block will be created, so verify it explicitly
            verifyBlock(coder, code_pos);
        }
        for (FrameState* succ = worklist; succ != NULL; succ = worklist) {
            worklist = succ->wl_next;
            succ->wl_pending = false;
            verifyBlock(coder, loadBlockState(succ));
        }
        coder->writeEpilogue(state);

        // phase 2 - traverse code in abc order and emit
        mmfx_delete(state);
        coder = emitter;

        #ifdef VMCFG_VERIFYALL
        // push the verifyall filter onto the front of the coder pipeline
        VerifyallWriter verifyallWriter(info, coder);
        if (core->config.verifyall)
            coder = &verifyallWriter;
        #endif

        // save computed ScopeTypeChain for OP_newfunction and OP_newclass
        ScopeWriter scopeWriter(coder, info, toplevel);
        coder = &scopeWriter;

        #ifdef AVMPLUS_VERBOSE
        if (verbose)
            core->console << "\nverify " << info << '\n';
        #endif

        emitPass = true;
        this->coder = coder;
        parseBodyHeader();          // reset code_pos & code_length
        checkParams();
        coder->writePrologue(state, code_pos);
        const byte* end_pos = code_pos;
        // typically, first block is not in blockStates: verify it explicitly
        if (!hasFrameState(0))
            end_pos = verifyBlock(coder, code_pos);
        // visit blocks in linear order (blockStates is sorted by abc address)
        for (int i=0, n=getBlockCount(); i < n; i++) {
            const byte* start_pos = loadBlockState(blockStates->at(i));
            // overlapping blocks indicates a branch to the middle of an instruction
            if (start_pos < end_pos)
                verifyFailed(kInvalidBranchTargetError);
            end_pos = verifyBlock(coder, start_pos);
        }
        coder->writeEpilogue(state);

        } CATCH (Exception *exception) {
            core->throwException(exception); // re-throw
        }
        END_CATCH
        END_TRY
    }

    const byte* Verifier::loadBlockState(FrameState* blk)
    {
        // now load the saved state at this block
        state->init(blk);
        state->targetOfBackwardsBranch = blk->targetOfBackwardsBranch;
        state->pc = blk->pc;

#ifdef AVMPLUS_VERBOSE
        if (verbose) {
            StringBuffer buf(core);
            buf << "B" << blk->pc << ":";
            printState(buf, state);
        }
#endif

        // found the start of a new basic block
        coder->writeBlockStart(state);
        return code_pos + blk->pc;
    }

    void Verifier::checkParams()
    {
        const int param_count = ms->param_count();

        if (local_count < param_count+1) {
            // must have enough locals to hold all parameters including this
            toplevel->throwVerifyError(kCorruptABCError);
        }

        // initial scope chain types
        if (info->declaringTraits()->init != info && info->declaringScope() == NULL) {
            // this can occur when an activation scope inside a class instance method
            // contains a nested getter, setter, or method.  In that case the scope
            // is not captured when the containing function is verified.  This isn't a
            // bug because such nested functions aren't suppported by AS3.  This
            // verify error is how we don't let those constructs run.
            verifyFailed(kNoScopeError, core->toErrorString(info));
        }

        state = mmfx_new( FrameState(this) );

        // initialize method param types.
        // We already verified param_count is a legal register so
        // don't checkLocal(i) inside the loop.
        // MethodInfo::verify takes care of resolving param&return type
        // names to Traits pointers, and resolving optional param default values.
        for (int i=0; i <= param_count; i++)
            state->setType(i, ms->paramTraits(i), i == 0);

        int first_local = param_count+1;
        if (info->needRest() || info->needArguments()) {
            // NEED_REST overrides NEED_ARGUMENTS when both are set
            checkLocal(first_local);  // ensure param_count+1 <= max_reg
            state->setType(first_local, ARRAY_TYPE, true);
            first_local++;
        } else {
            checkLocal(param_count);  // ensure param_count <= max_reg
        }

        for (int i=first_local; i < local_count; i++)
            state->setType(i, NULL); // void would be more precise.
    }

    // verify one superblock, return at the end.  The end of the block is when
    // we reach a terminal opcode (jump, lookupswitch, returnvalue, returnvoid,
    // or throw), or when we fall into the beginning of another block.
    // returns the address of the next instruction after the block end.
    const byte* Verifier::verifyBlock(CodeWriter *coder, const byte* start_pos)
    {
        _nvprof("verify-block", 1);
        ExceptionHandlerTable* exTable = info->abc_exceptions();
        bool isLoopHeader = state->targetOfBackwardsBranch;
        state->targetOfBackwardsBranch = false;
        const byte* code_end = code_pos + code_length;
        for (const byte *pc = start_pos, *nextpc = pc; pc < code_end; pc = nextpc)
        {
            // should we make a new sample frame in this method?
            // SAMPLE_CHECK();
            PERFM_NVPROF("abc-verify", 1);

            coder->writeFixExceptionsAndLabels(state, pc);

            AbcOpcode opcode = (AbcOpcode) *pc;
            if (opcodeInfo[opcode].operandCount == -1)
                verifyFailed(kIllegalOpcodeError, core->toErrorString(info), core->toErrorString(opcode), core->toErrorString((int)(pc-code_pos)));

            int logical_pc = int(pc - code_pos);
            state->pc = logical_pc;

            // test for the start of a new block
            if (pc != start_pos && (opcode == OP_label || hasFrameState(logical_pc))) {
                checkTarget(pc-1, pc);
                return pc;
            }

            int sp = state->sp();

            if (pc < tryTo && pc >= tryFrom &&
                (opcodeInfo[opcode].canThrow || (isLoopHeader && pc == start_pos))) {
                // If this instruction can throw exceptions, treat it as an edge to
                // each in-scope catch handler.  The instruction can throw exceptions
                // if canThrow = true, or if this is the target of a backedge, where
                // the implicit interrupt check can throw an exception.
                for (int i=0, n=exTable->exception_count; i < n; i++) {
                    ExceptionHandler* handler = &exTable->exceptions[i];
                    if (pc >= code_pos + handler->from && pc < code_pos + handler->to) {
                        int saveStackDepth = state->stackDepth;
                        int saveScopeDepth = state->scopeDepth;
                        Value stackEntryZero = saveStackDepth > 0 ? state->stackValue(0) : state->value(0);
                        state->stackDepth = 0;
                        state->scopeDepth = 0;

                        // add edge from try statement to catch block
                        const byte* target = code_pos + handler->target;
                        // atom received as *, will coerce to correct type in catch handler.
                        state->push(NULL); // type * is NULL
                        checkTarget(pc, target);
                        state->pop();

                        state->stackDepth = saveStackDepth;
                        state->scopeDepth = saveScopeDepth;
                        if (saveStackDepth > 0)
                            state->stackValue(0) = stackEntryZero;
                    }
                }
            }

            if (tryFrom >= code_pos && tryTo <= (code_pos + code_length) && pc >= tryFrom) {
                // all catch targets must be after the earliest try range since from <= to <= catch
                for (int i=0, n=exTable->exception_count; i < n; i++) {
                    ExceptionHandler* handler = &exTable->exceptions[i];
                    if (pc == code_pos + handler->target) {
                        AvmAssert(sp == stackBase); // if not, a VerifyError should have already occurred.
                        // FIXME: bug 538639: At the top of a catch block, we generate coerce when an unbox is all that is needed
                        emitCoerce(handler->traits, sp);
                        coder->writeOpcodeVerified(state, pc, opcode);
                    }
                }
            }

            uint32_t imm30=0, imm30b=0;
            int32_t imm8=0, imm24=0;
            AvmCore::readOperands(nextpc, imm30, imm24, imm30b, imm8);

            // make sure U30 operands are within bounds,
            // except for OP_pushshort, whose operand is sign extended from 16 bits,
            // and except for OP_abs_jump which can be full 32bit.
            if (opcode != OP_pushshort && ((imm30|imm30b) & 0xc0000000) && (opcode != OP_abs_jump || (imm30b & 0xc0000000)))
                verifyFailed(kCorruptABCError);
            if (nextpc > code_end)
                verifyFailed(kLastInstExceedsCodeSizeError);

            #ifdef AVMPLUS_VERBOSE
            if (verbose)
                printOpcode(pc);
            #endif

            _nvprof("verify-instr", 1);
            switch (opcode)
            {
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
                checkStack(2,0);
                coder->writeOp1(state, pc, opcode, imm24);
                state->pop(2);
                checkTarget(pc, nextpc+imm24);
                break;

            case OP_iftrue:
            case OP_iffalse:
                checkStack(1,0);
                emitCoerce(BOOLEAN_TYPE, sp);
                coder->writeOp1(state, pc, opcode, imm24);
                state->pop();
                checkTarget(pc, nextpc+imm24);
                break;

            case OP_jump:
                //checkStack(0,0)
                coder->writeOp1(state, pc, opcode, imm24);
                checkTarget(pc, nextpc+imm24);  // target block;
                coder->writeOpcodeVerified(state, pc, opcode);
                return nextpc;

            case OP_lookupswitch:
            {
                checkStack(1,0);
                peekType(INT_TYPE);
                coder->write(state, pc, opcode);
                state->pop();
                checkTarget(pc, pc+imm24);
                uint32_t case_count = 1 + imm30b;
                if (nextpc + 3*case_count > code_end)
                    verifyFailed(kLastInstExceedsCodeSizeError);
                for (uint32_t i=0; i < case_count; i++)
                {
                    int off = AvmCore::readS24(nextpc);
                    checkTarget(pc, pc+off);
                    nextpc += 3;
                }
                coder->writeOpcodeVerified(state, pc, opcode);
                return nextpc;
            }

            case OP_throw:
                checkStack(1,0);
                coder->write(state, pc, opcode);
                state->pop();
                coder->writeOpcodeVerified(state, pc, opcode);
                return nextpc;

            case OP_returnvalue:
                checkStack(1,0);
                emitCoerce(ms->returnTraits(), sp);
                coder->write(state, pc, opcode);
                state->pop();
                coder->writeOpcodeVerified(state, pc, opcode);
                return nextpc;

            case OP_returnvoid:
                //checkStack(0,0)
                coder->write(state, pc, opcode);
                coder->writeOpcodeVerified(state, pc, opcode);
                return nextpc;

            case OP_pushnull:
                checkStack(0,1);
                coder->write(state, pc, opcode, NULL_TYPE);
                state->push(NULL_TYPE);
                break;

            case OP_pushundefined:
                checkStack(0,1);
                coder->write(state, pc, opcode, VOID_TYPE);
                state->push(VOID_TYPE);
                break;

            case OP_pushtrue:
                checkStack(0,1);
                coder->write(state, pc, opcode, BOOLEAN_TYPE);
                state->push(BOOLEAN_TYPE, true);
                break;

            case OP_pushfalse:
                checkStack(0,1);
                coder->write(state, pc, opcode, BOOLEAN_TYPE);
                state->push(BOOLEAN_TYPE, true);
                break;

            case OP_pushnan:
                checkStack(0,1);
                coder->write(state, pc, opcode, NUMBER_TYPE);
                state->push(NUMBER_TYPE, true);
                break;

            case OP_pushshort:
                checkStack(0,1);
                coder->write(state, pc, opcode, INT_TYPE);
                state->push(INT_TYPE, true);
                break;

            case OP_pushbyte:
                checkStack(0,1);
                coder->write(state, pc, opcode, INT_TYPE);
                state->push(INT_TYPE, true);
                break;

            case OP_debugfile:
                //checkStack(0,0)
                #if defined(DEBUGGER) || defined(VTUNE)
                checkCpoolOperand(imm30, kStringType);
                #endif
                coder->write(state, pc, opcode);
                break;

            case OP_dxns:
                //checkStack(0,0)
                if (!info->setsDxns())
                    verifyFailed(kIllegalSetDxns, core->toErrorString(info));
                checkCpoolOperand(imm30, kStringType);
                coder->write(state, pc, opcode);
                break;

            case OP_dxnslate:
                checkStack(1,0);
                if (!info->setsDxns())
                    verifyFailed(kIllegalSetDxns, core->toErrorString(info));
                // codgen will call intern on the input atom.
                coder->write(state, pc, opcode);
                state->pop();
                break;

            case OP_pushstring:
                checkStack(0,1);
                if (imm30 == 0 || imm30 >= pool->constantStringCount)
                    verifyFailed(kCpoolIndexRangeError, core->toErrorString(imm30), core->toErrorString(pool->constantStringCount));
                coder->write(state, pc, opcode, STRING_TYPE);
                state->push(STRING_TYPE, pool->getString(imm30) != NULL);
                break;

            case OP_pushint:
                checkStack(0,1);
                if (imm30 == 0 || imm30 >= pool->constantIntCount)
                    verifyFailed(kCpoolIndexRangeError, core->toErrorString(imm30), core->toErrorString(pool->constantIntCount));
                coder->write(state, pc, opcode, INT_TYPE);
                state->push(INT_TYPE,true);
                break;

            case OP_pushuint:
                checkStack(0,1);
                if (imm30 == 0 || imm30 >= pool->constantUIntCount)
                    verifyFailed(kCpoolIndexRangeError, core->toErrorString(imm30), core->toErrorString(pool->constantUIntCount));
                coder->write(state, pc, opcode, UINT_TYPE);
                state->push(UINT_TYPE,true);
                break;

            case OP_pushdouble:
                checkStack(0,1);
                if (imm30 == 0 || imm30 >= pool->constantDoubleCount)
                    verifyFailed(kCpoolIndexRangeError, core->toErrorString(imm30), core->toErrorString(pool->constantDoubleCount));
                coder->write(state, pc, opcode, NUMBER_TYPE);
                state->push(NUMBER_TYPE, true);
                break;

            case OP_pushnamespace:
                checkStack(0,1);
                if (imm30 == 0 || imm30 >= pool->constantNsCount)
                    verifyFailed(kCpoolIndexRangeError, core->toErrorString(imm30), core->toErrorString(pool->constantNsCount));
                coder->write(state, pc, opcode, NAMESPACE_TYPE);
                state->push(NAMESPACE_TYPE, pool->cpool_ns[imm30] != NULL);
                break;

            case OP_setlocal:
            {
                checkStack(1,0);
                checkLocal(imm30);
                coder->write(state, pc, opcode);
                Value &v = state->stackTop();
                state->setType(imm30, v.traits, v.notNull);
                state->pop();
                break;
            }

            case OP_setlocal0:
            case OP_setlocal1:
            case OP_setlocal2:
            case OP_setlocal3:
            {
                checkStack(1,0);
                int index = opcode-OP_setlocal0;
                checkLocal(index);
                coder->write(state, pc, opcode);
                Value &v = state->stackTop();
                state->setType(index, v.traits, v.notNull);
                state->pop();
                break;
            }
            case OP_getlocal:
            {
                checkStack(0,1);
                Value& v = checkLocal(imm30);
                coder->write(state, pc, opcode);
                state->push(v);
                break;
            }
            case OP_getlocal0:
            case OP_getlocal1:
            case OP_getlocal2:
            case OP_getlocal3:
            {
                checkStack(0,1);
                Value& v = checkLocal(opcode-OP_getlocal0);
                coder->write(state, pc, opcode);
                state->push(v);
                break;
            }
            case OP_kill:
            {
                //checkStack(0,0)
                checkLocal(imm30);
                coder->write(state, pc, opcode, NULL);
                state->setType(imm30, NULL, false);
                break;
            }

            case OP_inclocal:
            case OP_declocal:
                //checkStack(0,0);
                checkLocal(imm30);
                emitCoerce(NUMBER_TYPE, imm30);
                coder->write(state, pc, opcode);
                break;

            case OP_inclocal_i:
            case OP_declocal_i:
                //checkStack(0,0);
                checkLocal(imm30);
                emitCoerce(INT_TYPE, imm30);
                coder->write(state, pc, opcode);
                break;

            case OP_newfunction:
            {
                checkStack(0,1);
                checkMethodInfo(imm30);
                Traits* ftraits = core->traits.function_itraits;
                coder->writeOp1(state, pc, opcode, imm30, ftraits);
                state->push(ftraits, true);
                break;
            }

            case OP_getlex:
            {
                if (state->scopeDepth + info->declaringScope()->size == 0)
                    verifyFailed(kFindVarWithNoScopeError);
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(0, 1, &multiname);
                if (multiname.isRuntime())
                    verifyFailed(kIllegalOpMultinameError, core->toErrorString(opcode), core->toErrorString(&multiname));
                emitFindProperty(OP_findpropstrict, multiname, imm30, pc);
                emitGetProperty(multiname, 1, imm30, pc);
                break;
            }

            case OP_findpropstrict:
            case OP_findproperty:
            {
                if (state->scopeDepth + info->declaringScope()->size == 0)
                    verifyFailed(kFindVarWithNoScopeError);
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(0, 1, &multiname);
                emitFindProperty(opcode, multiname, imm30, pc);
                break;
            }

            case OP_newclass:
            {
                checkStack(1, 1);
                // must be a CONSTANT_Multiname
                Traits* ctraits = checkClassInfo(imm30);
                emitCoerce(CLASS_TYPE, state->sp());
                coder->writeOp1(state, pc, opcode, imm30, ctraits);
                state->pop_push(1, ctraits, true);
                break;
            }

            case OP_finddef:
            {
                // must be a CONSTANT_Multiname.
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(0, 1, &multiname);
                if (!multiname.isBinding())
                {
                    // error, def name must be CT constant, regular name
                    verifyFailed(kIllegalOpMultinameError, core->toErrorString(opcode), core->toErrorString(&multiname));
                }
                MethodInfo* script = pool->getNamedScript(&multiname);
                Traits* resultType;
                if (script != (MethodInfo*)BIND_NONE && script != (MethodInfo*)BIND_AMBIGUOUS) {
                    // found a single matching traits
                    resultType = script->declaringTraits();
                } else {
                    // no traits, or ambiguous reference.  use Object, anticipating
                    // a runtime exception
                    resultType = OBJECT_TYPE;
                }
                coder->writeOp1(state, pc, opcode, imm30, resultType);
                state->push(resultType, true);
                break;
            }

            case OP_setproperty:
            case OP_initproperty:
            {
                // stack in: object [ns] [name] value
                Multiname multiname;
                checkConstantMultiname(imm30, multiname); // CONSTANT_Multiname
                checkStackMulti(2, 0, &multiname);

                uint32_t n=2;
                checkPropertyMultiname(n, multiname);

                Traitsp declarer = NULL;
                Value& obj = state->peek(n);
                Binding b = (opcode == OP_initproperty) ?
                            toplevel->getBindingAndDeclarer(obj.traits, multiname, declarer) :
                            toplevel->getBinding(obj.traits, &multiname);
                Traits* propTraits = readBinding(obj.traits, b);

                emitCheckNull(sp-(n-1));

                if (AvmCore::isSlotBinding(b) &&
                    // it's a var, or a const being set from the init function
                    (!AvmCore::isConstBinding(b) ||
                        (opcode == OP_initproperty && declarer->init == info)))
                {
                    emitCoerce(propTraits, state->sp());
                    coder->writeOp2(state, pc, OP_setslot, (uint32_t)AvmCore::bindingToSlotId(b), sp-(n-1), propTraits);
                    state->pop(n);
                    break;
                }
                // else: setting const from illegal context, fall through

                // If it's an accessor that we can early bind, do so.
                // Note that this cannot be done on String or Namespace,
                // since those are represented by non-ScriptObjects
                if (AvmCore::hasSetterBinding(b))
                {
                    // invoke the setter
                    int disp_id = AvmCore::bindingToSetterId(b);
                    const TraitsBindingsp objtd = obj.traits->getTraitsBindings();
                    MethodInfo *f = objtd->getMethod(disp_id);
                    AvmAssert(f != NULL);
                    MethodSignaturep fms = f->getMethodSignature();
                    emitCoerceArgs(f, 1);
                    Traits* propType = fms->returnTraits();
                    coder->writeOp2(state, pc, opcode, imm30, n, propType);
                    state->pop(n);
                    break;
                }

                if( obj.traits == VECTORINT_TYPE  || obj.traits == VECTORUINT_TYPE ||
                    obj.traits == VECTORDOUBLE_TYPE )
                {
                    bool attr = multiname.isAttr();
                    Traits* indexType = state->value(state->sp()-1).traits;

                    // NOTE a dynamic name should have the same version as the current pool
                    bool maybeIntegerIndex = !attr && multiname.isRtname() && multiname.containsAnyPublicNamespace();
                    if( maybeIntegerIndex && (indexType == UINT_TYPE || indexType == INT_TYPE) )
                    {
                        if(obj.traits == VECTORINT_TYPE)
                            emitCoerce(INT_TYPE, state->sp());
                        else if(obj.traits == VECTORUINT_TYPE)
                            emitCoerce(UINT_TYPE, state->sp());
                        else if(obj.traits == VECTORDOUBLE_TYPE)
                            emitCoerce(NUMBER_TYPE, state->sp());
                    }
                }

                // default - do getproperty at runtime

                coder->writeOp2(state, pc, opcode, imm30, n, propTraits);
                state->pop(n);
                break;
            }

            case OP_getproperty:
            {
                // stack in: object [ns [name]]
                // stack out: value
                Multiname multiname;
                checkConstantMultiname(imm30, multiname); // CONSTANT_Multiname
                checkStackMulti(1, 1, &multiname);

                uint32_t n=1;
                checkPropertyMultiname(n, multiname);
                emitGetProperty(multiname, n, imm30, pc);
                break;
            }

            case OP_getdescendants:
            {
                // stack in: object [ns] [name]
                // stack out: value
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(1, 1, &multiname);

                uint32_t n=1;
                checkPropertyMultiname(n, multiname);
                emitCheckNull(sp-(n-1));
                coder->write(state, pc, opcode);
                state->pop_push(n, NULL);
                break;
            }

            case OP_checkfilter:
                checkStack(1, 1);
                emitCheckNull(sp);
                coder->write(state, pc, opcode);
                break;

            case OP_deleteproperty:
            {
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(1, 1, &multiname);
                uint32_t n=1;
                checkPropertyMultiname(n, multiname);
                emitCheckNull(sp-(n-1));
                coder->write(state, pc, opcode);
                state->pop_push(n, BOOLEAN_TYPE);
                break;
            }

            case OP_astype:
            {
                checkStack(1, 1);
                // resolve operand into a traits, and push that type.
                Traits *t = checkTypeName(imm30); // CONSTANT_Multiname
                int index = sp;
                Traits* rhs = state->value(index).traits;
                if (!canAssign(t, rhs))
                {
                    Traits* resultType = t;
                    // result is typed value or null, so if type can't hold null,
                    // then result type is Object.
                    if (t && t->isMachineType())
                        resultType = OBJECT_TYPE;
                    coder->write(state, pc, opcode);
                    state->pop_push(1, resultType);
                }
                break;
            }

            case OP_astypelate:
            {
                checkStack(2,1);
                Value& classValue = state->peek(1); // rhs - class
                Traits* ct = classValue.traits;
                Traits* t = NULL;
                if (ct && (t=ct->itraits) != 0)
                    if (t->isMachineType())
                        t = OBJECT_TYPE;
                coder->write(state, pc, opcode);
                state->pop_push(2, t);
                break;
            }

            case OP_coerce:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = checkTypeName(imm30);
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_convert_b:
            case OP_coerce_b:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = BOOLEAN_TYPE;
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_coerce_o:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = OBJECT_TYPE;
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_coerce_a:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = NULL;
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_convert_i:
            case OP_coerce_i:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = INT_TYPE;
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_convert_u:
            case OP_coerce_u:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = UINT_TYPE;
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_convert_d:
            case OP_coerce_d:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = NUMBER_TYPE;
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_coerce_s:
            {
                checkStack(1,1);
                Value &v = state->value(sp);
                Traits *type = STRING_TYPE;
                coder->write(state, pc, opcode, type);
                state->setType(sp, type, v.notNull);
                break;
            }
            case OP_istype:
                checkStack(1,1);
                // resolve operand into a traits, and test if value is that type
                checkTypeName(imm30); // CONSTANT_Multiname
                coder->write(state, pc, opcode);
                state->pop(1);
                state->push(BOOLEAN_TYPE);
                break;

            case OP_istypelate:
                checkStack(2,1);
                coder->write(state, pc, opcode);
                // TODO if the only common base type of lhs,rhs is Object, then result is always false
                state->pop_push(2, BOOLEAN_TYPE);
                break;

            case OP_convert_o:
                checkStack(1,1);
                // ISSUE should result be Object, laundering the type?
                // ToObject throws an exception on null and undefined, so after this runs we
                // know the value is safe to dereference.
                emitCheckNull(sp);
                coder->write(state, pc, opcode);
                break;

            case OP_convert_s:
            case OP_esc_xelem:
            case OP_esc_xattr:
                checkStack(1,1);
                // this is the ECMA ToString and ToXMLString operators, so the result must not be null
                // (ToXMLString is split into two variants - escaping elements and attributes)
                coder->write(state, pc, opcode);
                state->pop_push(1, STRING_TYPE, true);
                break;

            case OP_callstatic:
            {
                //  Ensure that the method is eligible for callstatic.
                //  Note: This fails when called by verifyEarly(), since the
                //  data structures being checked have not been initialized.
                //  Need to either rearrange the initialization sequence or
                //  mark this verify pass as "needs late retry."
                if ( ! abc_env->getMethod(imm30) )
                    verifyFailed(kCorruptABCError);

                MethodInfo* m = checkMethodInfo(imm30);
                const uint32_t argc = imm30b;
                checkStack(argc+1, 1);

                MethodSignaturep mms = m->getMethodSignature();
                if (!mms->paramTraits(0))
                {
                    verifyFailed(kDanglingFunctionError, core->toErrorString(m), core->toErrorString(info));
                }

                emitCoerceArgs(m, argc);

                Traits *resultType = mms->returnTraits();
                emitCheckNull(sp-argc);
                coder->writeOp2(state, pc, OP_callstatic, (uint32_t)m->method_id(), argc, resultType);
                state->pop_push(argc+1, resultType);
                break;
            }

            case OP_call:
            {
                const uint32_t argc = imm30;
                checkStack(argc+2, 1);
                // don't need null check, AvmCore::call() uses toFunction() for null check.

                /*
                    TODO optimizations
                        - if this is a class closure for a non-native type, call == coerce
                        - if this is a function closure, try early binding using the traits->call sig
                        - optimize simple cases of casts to builtin types
                */

                coder->writeOp1(state, pc, opcode, argc);
                state->pop_push(argc+2, NULL);
                break;
            }

            case OP_construct:
            {
                const uint32_t argc = imm30;
                checkStack(argc+1, 1);

                // don't need null check, AvmCore::construct() uses toFunction() for null check.
                Traits* ctraits = state->peek(argc+1).traits;
                Traits* itraits = ctraits ? ctraits->itraits : NULL;
                coder->writeOp1(state, pc, opcode, argc);
                state->pop_push(argc+1, itraits, true);
                break;
            }

            case OP_callmethod:
            {
                /*
                    OP_callmethod will always throw a verify error.  that's on purpose, it's a
                    last minute change before we shipped FP9 and was necessary when we added methods to class Object.

                    since then we realized that OP_callmethod need only have failed when used outside
                    of the builtin abc, but it's a moot point now.  We dont have to worry about it.

                    code has since been simplified but existing failure modes preserved.
                */
                const uint32_t argc = imm30b;
                checkStack(argc+1,1);

                const int disp_id = imm30-1;
                if (disp_id >= 0)
                {
                    Value& obj = state->peek(argc+1);
                    if( !obj.traits )
                        verifyFailed(kCorruptABCError);
                    else
                        verifyFailed(kIllegalEarlyBindingError, core->toErrorString(obj.traits));
                }
                else
                {
                    verifyFailed(kZeroDispIdError);
                }
                break;
            }

            case OP_callproperty:
            case OP_callproplex:
            case OP_callpropvoid:
            {
                // stack in: obj [ns [name]] args
                // stack out: result
                const uint32_t argc = imm30b;
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(argc+1, 1, &multiname);
                checkCallMultiname(opcode, &multiname);

                uint32_t n = argc+1; // index of receiver
                checkPropertyMultiname(n, multiname);
                emitCallproperty(opcode, sp, multiname, imm30, imm30b, pc);
                break;
            }

            case OP_constructprop:
            {
                // stack in: obj [ns [name]] args
                const uint32_t argc = imm30b;
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(argc+1, 1, &multiname);
                checkCallMultiname(opcode, &multiname);

                uint32_t n = argc+1; // index of receiver
                checkPropertyMultiname(n, multiname);


                Value& obj = state->peek(n); // make sure object is there
                Binding b = toplevel->getBinding(obj.traits, &multiname);
                Traits* ctraits = readBinding(obj.traits, b);
                emitCheckNull(sp-(n-1));
                coder->writeOp2(state, pc, opcode, imm30, argc, ctraits);

                Traits* itraits = ctraits ? ctraits->itraits : NULL;
                state->pop_push(n, itraits, itraits==NULL?false:true);
                break;
            }

            case OP_applytype:
            {
                // in: factory arg1..N
                // out: type
                const uint32_t argc = imm30;
                checkStack(argc+1, 1);
                coder->write(state, pc, opcode);
                state->pop_push(argc+1, NULL, true);
                break;
            }

            case OP_callsuper:
            case OP_callsupervoid:
            {
                // stack in: obj [ns [name]] args
                const uint32_t argc = imm30b;
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(argc+1, 1, &multiname);

                if (multiname.isAttr())
                    verifyFailed(kIllegalOpMultinameError, core->toErrorString(&multiname));

                uint32_t n = argc+1; // index of receiver
                checkPropertyMultiname(n, multiname);

                Traits* base = emitCoerceSuper(sp-(n-1));
                const TraitsBindingsp basetd = base->getTraitsBindings();

                Binding b = toplevel->getBinding(base, &multiname);

                Traits *resultType = NULL;
                if (AvmCore::isMethodBinding(b))
                {
                    int disp_id = AvmCore::bindingToMethodId(b);
                    MethodInfo* m = basetd->getMethod(disp_id);
                    if( !m ) verifyFailed(kCorruptABCError);
                    MethodSignaturep mms = m->getMethodSignature();
                    resultType = mms->returnTraits();
                }

                emitCheckNull(sp-(n-1));
                coder->writeOp2(state, pc, opcode, imm30, argc, base);
                state->pop_push(n, resultType);

                if (opcode == OP_callsupervoid)
                    state->pop();

                break;
            }

            case OP_getsuper:
            {
                // stack in: obj [ns [name]]
                // stack out: value
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(1, 1, &multiname);
                uint32_t n=1;
                checkPropertyMultiname(n, multiname);

                if (multiname.isAttr())
                    verifyFailed(kIllegalOpMultinameError, core->toErrorString(&multiname));

                Traits* base = emitCoerceSuper(sp-(n-1));
                Binding b = toplevel->getBinding(base, &multiname);
                Traits* propType = readBinding(base, b);
                emitCheckNull(sp-(n-1));
                coder->writeOp2(state, pc, opcode, imm30, n, base);

                if (AvmCore::hasGetterBinding(b))
                {
                    int disp_id = AvmCore::bindingToGetterId(b);
                    const TraitsBindingsp basetd = base->getTraitsBindings();
                        MethodInfo *f = basetd->getMethod(disp_id);
                    AvmAssert(f != NULL);
                    MethodSignaturep fms = f->getMethodSignature();
                    Traits* resultType = fms->returnTraits();
                    state->pop_push(n, resultType);
                }
                else
                {
                    state->pop_push(n, propType);
                }
                break;
            }

            case OP_setsuper:
            {
                // stack in: obj [ns [name]] value
                Multiname multiname;
                checkConstantMultiname(imm30, multiname);
                checkStackMulti(2, 0, &multiname);
                uint32_t n=2;
                checkPropertyMultiname(n, multiname);

                if (multiname.isAttr())
                    verifyFailed(kIllegalOpMultinameError, core->toErrorString(&multiname));

                Traits* base = emitCoerceSuper(sp-(n-1));
                emitCheckNull(sp-(n-1));
                coder->writeOp2(state, pc, opcode, imm30, n, base);
                state->pop(n);
                break;
            }

            case OP_constructsuper:
            {
                // stack in: obj, args ...
                const uint32_t argc = imm30;
                checkStack(argc+1, 0);

                int32_t ptrIndex = sp-argc;
                Traits* baseTraits = emitCoerceSuper(ptrIndex); // check receiver

                MethodInfo *f = baseTraits->init;
                AvmAssert(f != NULL);

                emitCoerceArgs(f, argc);
                emitCheckNull(sp-argc);
                coder->writeOp2(state, pc, opcode, 0, argc, baseTraits);
                state->pop(argc+1);
                break;
            }

            case OP_newobject:
            {
                uint32_t argc = imm30;
                checkStack(2*argc, 1);
                int n=0;
                while (argc-- > 0)
                {
                    n += 2;
                    peekType(STRING_TYPE, n); // name; will call intern on it
                }
                coder->write(state, pc, opcode);
                state->pop_push(n, OBJECT_TYPE, true);
                break;
            }

            case OP_newarray:
                checkStack(imm30, 1);
                coder->write(state, pc, opcode);
                state->pop_push(imm30, ARRAY_TYPE, true);
                break;

            case OP_pushscope:
            {
                checkStack(1,0);
                if (state->scopeDepth+1 > max_scope)
                    verifyFailed(kScopeStackOverflowError);

                Traits* scopeTraits = state->peek().traits;
                const ScopeTypeChain* scope = info->declaringScope();
                if (scope->fullsize > (scope->size+state->scopeDepth))
                {
                    // extra constraints on type of pushscope allowed
                    Traits* requiredType = scope->getScopeTraitsAt(scope->size+state->scopeDepth);
                    if (!scopeTraits || !scopeTraits->subtypeof(requiredType))
                    {
                        verifyFailed(kIllegalOperandTypeError, core->toErrorString(scopeTraits), core->toErrorString(requiredType));
                    }
                }

                emitCheckNull(sp);
                coder->writeOp1(state, pc, opcode, scopeBase+state->scopeDepth);
                state->pop();
                state->setType(scopeBase+state->scopeDepth, scopeTraits, true, false);
                state->scopeDepth++;
                break;
            }

            case OP_pushwith:
            {
                checkStack(1,0);

                if (state->scopeDepth+1 > max_scope)
                    verifyFailed(kScopeStackOverflowError);

                emitCheckNull(sp);
                coder->writeOp1(state, pc, opcode, scopeBase+state->scopeDepth);

                Traits* scopeTraits = state->peek().traits;
                state->pop();
                state->setType(scopeBase+state->scopeDepth, scopeTraits, true, true);

                if (state->withBase == -1)
                    state->withBase = state->scopeDepth;

                state->scopeDepth++;
                break;
            }

            case OP_newactivation:
            {
                checkStack(0, 1);
                if (!info->needActivation())
                    verifyFailed(kInvalidNewActivationError);
                Traits* atraits = info->resolveActivation(toplevel);
                coder->write(state, pc, opcode, atraits);
                state->push(atraits, true);
                break;
            }
            case OP_newcatch:
            {
                checkStack(0, 1);
                if (!info->abc_exceptions() || imm30 >= (uint32_t)info->abc_exceptions()->exception_count)
                    verifyFailed(kInvalidNewActivationError);
                    // FIXME better error msg
                ExceptionHandler* handler = &info->abc_exceptions()->exceptions[imm30];
                coder->write(state, pc, opcode);
                state->push(handler->scopeTraits, true);
                break;
            }
            case OP_popscope:
                //checkStack(0,0)
                if (state->scopeDepth-- <= 0)
                    verifyFailed(kScopeStackUnderflowError);

                coder->write(state, pc, opcode);

                if (state->withBase >= state->scopeDepth)
                    state->withBase = -1;
                break;

            case OP_getscopeobject:
                checkStack(0,1);

                // local scope
                if (imm8 >= state->scopeDepth)
                    verifyFailed(kGetScopeObjectBoundsError, core->toErrorString(imm8));

                coder->writeOp1(state, pc, opcode, imm8);

                // this will copy type and all attributes too
                state->push(state->scopeValue(imm8));
                break;

            case OP_getouterscope:
            {
                checkStack(0,1);
                const ScopeTypeChain* scope = info->declaringScope();
                uint32_t index = imm30;
                int captured_depth = scope->size;
                if (captured_depth > 0)
                {
                    // enclosing scope
                    Traits* t = scope->getScopeTraitsAt(index);
                    coder->writeOp1(state, pc, opcode, index, t);
                    state->push(t, true);
                }
                else
                {
                    #ifdef _DEBUG
                    if (pool->isBuiltin)
                      core->console << "getouterscope >= depth (" << index << " >= " << state->scopeDepth << ")\n";
                    #endif
                    verifyFailed(kGetScopeObjectBoundsError, core->toErrorString(index));
                }
                break;
            }

            case OP_getglobalscope:
                checkStack(0,1);
                coder->write(state, pc, OP_getglobalscope);
                checkGetGlobalScope(); // after coder->write because mutates stack that coder depends on
                break;

            case OP_getglobalslot:
            {
                checkStack(0,1);
                uint32_t slot = imm30-1;
                Traits* globalTraits = checkGetGlobalScope();
                checkEarlySlotBinding(globalTraits); // sets state->value(sp).traits so CodeWriter can see type
                Traits* slotTraits = checkSlot(globalTraits, slot);
                coder->writeOp1(state, pc, OP_getglobalslot, slot, slotTraits);
                state->pop_push(1, slotTraits);
                break;
            }

            case OP_setglobalslot:
            {
                // FIXME need test case
                const ScopeTypeChain* scope = info->declaringScope();
                if (!state->scopeDepth && !scope->size)
                    verifyFailed(kNoGlobalScopeError);
                Traits *globalTraits = scope->size > 0 ? scope->getScopeTraitsAt(0) : state->scopeValue(0).traits;
                checkStack(1,0);
                checkEarlySlotBinding(globalTraits);
                Traits* slotTraits = checkSlot(globalTraits, imm30-1);
                emitCoerce(slotTraits, state->sp());
                coder->writeOp1(state, pc, opcode, imm30-1, slotTraits);
                state->pop();
                break;
            }

            case OP_getslot:
            {
                checkStack(1,1);
                Value& obj = state->peek();
                checkEarlySlotBinding(obj.traits);
                Traits* slotTraits = checkSlot(obj.traits, imm30-1);
                emitCheckNull(state->sp());
                coder->write(state, pc, opcode);
                state->pop_push(1, slotTraits);
                break;
            }

            case OP_setslot:
            {
                checkStack(2,0);
                Value& obj = state->peek(2); // object
                // if code isn't in pool, its our generated init function which we always
                // allow early binding on
                if(pool->isCodePointer(info->abc_body_pos()))
                    checkEarlySlotBinding(obj.traits);
                Traits* slotTraits = checkSlot(obj.traits, imm30-1);
                emitCoerce(slotTraits, state->sp());
                emitCheckNull(state->sp()-1);
                coder->write(state, pc, opcode);
                state->pop(2);
                break;
            }

            case OP_pop:
                checkStack(1,0);
                coder->write(state, pc, opcode);
                state->pop();
                break;

            case OP_dup:
            {
                checkStack(1, 2);
                Value& v = state->peek();
                coder->write(state, pc, opcode);
                state->push(v);
                break;
            }

            case OP_swap:
            {
                checkStack(2,2);
                Value v1 = state->peek(1);
                Value v2 = state->peek(2);
                coder->write(state, pc, opcode);
                state->pop(2);
                state->push(v1);
                state->push(v2);
                break;
            }

            case OP_lessthan:
            case OP_greaterthan:
            case OP_lessequals:
            case OP_greaterequals:
            {
                // if either the LHS or RHS is a number type, then we know
                // it will be a numeric comparison.
                checkStack(2,1);
                Value& rhs = state->peek(1);
                Value& lhs = state->peek(2);
                Traits *lhst = lhs.traits;
                Traits *rhst = rhs.traits;
                if (rhst && rhst->isNumeric() && lhst && !lhst->isNumeric())
                {
                    // convert lhs to Number
                    emitCoerce(NUMBER_TYPE, state->sp()-1);
                }
                else if (lhst && lhst->isNumeric() && rhst && !rhst->isNumeric())
                {
                    // promote rhs to Number
                    emitCoerce(NUMBER_TYPE, state->sp());
                }
                coder->write(state, pc, opcode, BOOLEAN_TYPE);
                state->pop_push(2, BOOLEAN_TYPE);
                break;
            }

            case OP_equals:
            case OP_strictequals:
            case OP_instanceof:
            case OP_in:
                checkStack(2,1);
                coder->write(state, pc, opcode);
                state->pop_push(2, BOOLEAN_TYPE);
                break;

            case OP_not:
                checkStack(1,1);
                emitCoerce(BOOLEAN_TYPE, sp);
                coder->write(state, pc, opcode, BOOLEAN_TYPE);
                state->pop_push(1, BOOLEAN_TYPE);
                break;

            case OP_add:
            {
                checkStack(2,1);

                Value& rhs = state->peek(1);
                Value& lhs = state->peek(2);
                Traits* lhst = lhs.traits;
                Traits* rhst = rhs.traits;
                if ((lhst == STRING_TYPE && lhs.notNull) || (rhst == STRING_TYPE && rhs.notNull))
                {
                    coder->write(state, pc, OP_add, STRING_TYPE);
                    state->pop_push(2, STRING_TYPE, true);
                }
                else if (lhst && lhst->isNumeric() && rhst && rhst->isNumeric())
                {
                    coder->write(state, pc, OP_add, NUMBER_TYPE);
                    state->pop_push(2, NUMBER_TYPE);
                }
                else
                {
                    coder->write(state, pc, OP_add, OBJECT_TYPE);
                    // NOTE don't know if it will return number or string, but
                    // neither will be null
                    state->pop_push(2, OBJECT_TYPE, true);
                }
                break;
            }

            case OP_modulo:
            case OP_subtract:
            case OP_divide:
            case OP_multiply:
                checkStack(2,1);
                emitCoerce(NUMBER_TYPE, sp-1);
                emitCoerce(NUMBER_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop_push(2, NUMBER_TYPE);
                break;

            case OP_negate:
                checkStack(1,1);
                emitCoerce(NUMBER_TYPE, sp);
                coder->write(state, pc, opcode);
                break;

            case OP_increment:
            case OP_decrement:
                checkStack(1,1);
                emitCoerce(NUMBER_TYPE, sp);
                coder->write(state, pc, opcode);
                break;

            case OP_increment_i:
            case OP_decrement_i:
                checkStack(1,1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                break;

            case OP_add_i:
            case OP_subtract_i:
            case OP_multiply_i:
                checkStack(2,1);
                emitCoerce(INT_TYPE, sp-1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop_push(2, INT_TYPE);
                break;

            case OP_negate_i:
                checkStack(1,1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                break;

            case OP_bitand:
            case OP_bitor:
            case OP_bitxor:
                checkStack(2,1);
                emitCoerce(INT_TYPE, sp-1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop_push(2, INT_TYPE);
                break;

            // ISSUE do we care if shift amount is signed or not?  we mask
            // the result so maybe it doesn't matter.
            // CN says see tests e11.7.2, 11.7.3, 9.6
            case OP_lshift:
            case OP_rshift:
                checkStack(2,1);
                emitCoerce(INT_TYPE, sp-1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop_push(2, INT_TYPE);
                break;

            case OP_urshift:
                checkStack(2,1);
                emitCoerce(INT_TYPE, sp-1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop_push(2, UINT_TYPE);
                break;

            case OP_bitnot:
                checkStack(1,1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                break;

            case OP_typeof:
                checkStack(1,1);
                coder->write(state, pc, opcode);
                state->pop_push(1, STRING_TYPE, true);
                break;

            case OP_nop:
                // those show up but will be ignored
            case OP_bkpt:
            case OP_bkptline:
            case OP_timestamp:
                coder->write(state, pc, OP_nop);
                break;

            case OP_debug:
                coder->write(state, pc, opcode);
                break;

            case OP_label:
                coder->write(state, pc, opcode);
                break;

            case OP_debugline:
                coder->write(state, pc, opcode);
                break;

            case OP_nextvalue:
            case OP_nextname:
                checkStack(2,1);
                peekType(INT_TYPE, 1);
                coder->write(state, pc, opcode);
                state->pop_push(2, NULL);
                break;

            case OP_hasnext:
                checkStack(2,1);
                peekType(INT_TYPE,1);
                coder->write(state, pc, opcode);
                state->pop_push(2, INT_TYPE);
                break;

            case OP_hasnext2:
            {
                checkStack(0,1);
                checkLocal(imm30);
                Value& v = checkLocal(imm30b);
                if (imm30 == imm30b)
                    verifyFailed(kInvalidHasNextError);
                if (v.traits != INT_TYPE)
                    verifyFailed(kIllegalOperandTypeError, core->toErrorString(v.traits), core->toErrorString(INT_TYPE));
                coder->write(state, pc, opcode);
                state->setType(imm30, NULL, false);
                state->push(BOOLEAN_TYPE);
                break;
            }

            // sign extends
            case OP_sxi1:
            case OP_sxi8:
            case OP_sxi16:
                checkStack(1,1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop_push(1, INT_TYPE);
                break;

            // loads
            case OP_li8:
            case OP_li16:
                if (pc+1 < code_end &&
                    ((opcode == OP_li8 && pc[1] == OP_sxi8) || (opcode == OP_li16 && pc[1] == OP_sxi16)))
                {
                    checkStack(1,1);
                    emitCoerce(INT_TYPE, sp);
                    coder->write(state, pc, (opcode == OP_li8) ? OP_lix8 : OP_lix16);
                    state->pop_push(1, INT_TYPE);
                    // ++pc; // do not skip the sign-extend; if it's the target
                    // of an implicit label, skipping it would cause verification failure.
                    // instead, just emit it, and rely on LIR to ignore sxi instructions
                    // in these situations.
                    break;
                }
                // else fall thru
            case OP_li32:
            case OP_lf32:
            case OP_lf64:
            {
                Traits* result = (opcode == OP_lf32 || opcode == OP_lf64) ? NUMBER_TYPE : INT_TYPE;
                checkStack(1,1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop_push(1, result);
                break;
            }

            // stores
            case OP_si8:
            case OP_si16:
            case OP_si32:
            case OP_sf32:
            case OP_sf64:
                checkStack(2,0);
                emitCoerce((opcode == OP_sf32 || opcode == OP_sf64) ? NUMBER_TYPE : INT_TYPE, sp-1);
                emitCoerce(INT_TYPE, sp);
                coder->write(state, pc, opcode);
                state->pop(2);
                break;

            case OP_abs_jump:
            {
                // first ensure the executing code isn't user code (only VM generated abc can use this op)
                if (pool->isCodePointer(pc))
                    verifyFailed(kIllegalOpcodeError, core->toErrorString(info), core->toErrorString(OP_abs_jump), core->toErrorString((int)(pc-code_pos)));

                #ifdef AVMPLUS_64BIT
                const byte* new_pc = (const byte *) (uintptr(imm30) | (((uintptr) imm30b) << 32));
                uint32_t new_len = AvmCore::readU32(nextpc);
                #else
                const byte* new_pc = (const byte*) imm30;
                uint32_t new_len = imm30b;
                #endif

                // now ensure target points to within pool's script buffer
                if(!pool->isCodePointer(new_pc))
                    verifyFailed(kIllegalOpcodeError, core->toErrorString(info), core->toErrorString(OP_abs_jump), core->toErrorString((int)(pc-code_pos)));

                const byte* old_pc = pc;
                code_pos = pc = nextpc = new_pc;
                code_length = new_len;
                code_end = pc + new_len;
                parseExceptionHandlers();
                exTable = info->abc_exceptions();
                coder->writeOp2(state, old_pc, opcode, imm30, imm30b);
                break;
            }
            default:
                // size was nonzero, but no case handled the opcode.  someone asleep at the wheel!
                AvmAssertMsg(false, "Unhandled opcode");
            }

            coder->writeOpcodeVerified(state, pc, opcode);
            #ifdef AVMPLUS_VERBOSE
            if (verbose) {
                StringBuffer buf(core);
                printState(buf, state);
            }
            #endif
        }

        verifyFailed(kCannotFallOffMethodError);
        return code_end;
    }

    void Verifier::checkPropertyMultiname(uint32_t &depth, Multiname &multiname)
    {
        if (multiname.isRtname())
        {
            if (multiname.isQName())
            {
                // a.ns::@[name] or a.ns::[name]
                peekType(STRING_TYPE, depth++);
            }
            else
            {
                // a.@[name] or a[name]
                depth++;
            }
        }

        if (multiname.isRtns())
        {
            peekType(NAMESPACE_TYPE, depth++);
        }
    }

    void Verifier::emitCallproperty(AbcOpcode opcode, int& sp, Multiname& multiname, uint32_t multiname_index, uint32_t argc, const byte* pc)
    {
        uint32_t n = argc+1;
        checkPropertyMultiname(n, multiname);
        Traits* t = state->peek(n).traits;

        if (t)
            t->resolveSignatures(toplevel);
        Binding b = toplevel->getBinding(t, &multiname);

        emitCheckNull(sp-(n-1));

        if (emitCallpropertyMethod(opcode, t, b, multiname, multiname_index, argc, pc))
            return;

        if (emitCallpropertySlot(opcode, sp, t, b, argc, pc))
            return;

        coder->writeOp2(state, pc, opcode, multiname_index, argc);

        // If early binding then the state will have been updated, so this will be skipped
        state->pop_push(n, NULL);
        if (opcode == OP_callpropvoid)
            state->pop();
    }

    bool Verifier::emitCallpropertyMethod(AbcOpcode opcode, Traits* t, Binding b, Multiname& multiname, uint32_t multiname_index, uint32_t argc, const byte* pc)
    {
        (void) multiname_index;  // FIXME remove

        if (!AvmCore::isMethodBinding(b))
            return false;

        uint32_t n = argc+1;
        const TraitsBindingsp tb = t->getTraitsBindings();
        if (t == core->traits.math_ctraits)
            b = findMathFunction(tb, multiname, b, argc);
        else if (t == core->traits.string_itraits)
            b = findStringFunction(tb, multiname, b, argc);

        int disp_id = AvmCore::bindingToMethodId(b);
        MethodInfo* m = tb->getMethod(disp_id);
        MethodSignaturep mms = m->getMethodSignature();

        if (!mms->argcOk(argc))
            return false;

        Traits* resultType = mms->returnTraits();

        emitCoerceArgs(m, argc);
        if (!t->isInterface())
        {
            coder->writeMethodCall(state, pc, OP_callmethod, m, disp_id, argc, resultType);
            if (opcode == OP_callpropvoid)
                coder->write(state, pc, OP_pop);
        }
        else
        {
            // NOTE when the interpreter knows how to dispatch through an
            // interface, we can rewrite this call as a 'writeOp2'.
            coder->writeMethodCall(state, pc, opcode, m, m->iid(), argc, resultType);
        }

        state->pop_push(n, resultType);
        if (opcode == OP_callpropvoid)
        {
            state->pop();
        }

        return true;
    }

    bool Verifier::emitCallpropertySlot(AbcOpcode opcode, int& sp, Traits* t, Binding b, uint32_t argc, const byte *pc)
    {
        if (!AvmCore::isSlotBinding(b) || argc != 1)
            return false;

        const TraitsBindingsp tb = t->getTraitsBindings();

        int slot_id = AvmCore::bindingToSlotId(b);
        Traits* slotType = tb->getSlotTraits(slot_id);

        if (slotType == core->traits.int_ctraits)
        {
            coder->write(state, pc, OP_convert_i, INT_TYPE);
            state->setType(sp, INT_TYPE, true);
        }
        else
        if (slotType == core->traits.uint_ctraits)
        {
            coder->write(state, pc, OP_convert_u, UINT_TYPE);
            state->setType(sp, UINT_TYPE, true);
        }
        else
        if (slotType == core->traits.number_ctraits)
        {
            coder->write(state, pc, OP_convert_d, NUMBER_TYPE);
            state->setType(sp, NUMBER_TYPE, true);
        }
        else
        if (slotType == core->traits.boolean_ctraits)
        {
            coder->write(state, pc, OP_convert_b, BOOLEAN_TYPE);
            state->setType(sp, BOOLEAN_TYPE, true);
        }
        else
        if (slotType == core->traits.string_ctraits)
        {
            coder->write(state, pc, OP_convert_s, STRING_TYPE);
            state->setType(sp, STRING_TYPE, true);
        }
        else
        // NOTE the following has been refactored so that both lir and wc coerce. previously
        // wc would be skipped and fall back on the method call the the class converter
        if (slotType && slotType->base == CLASS_TYPE && slotType->getCreateClassClosureProc() == NULL)
        {
            // is this a user defined class?  A(1+ args) means coerce to A
            AvmAssert(slotType->itraits != NULL);
            Value &v = state->value(sp);
            coder->write(state, pc, OP_coerce, slotType->itraits);
            state->setType(sp, slotType->itraits, v.notNull);
        }
        else
        {
            return false;
        }

        if (opcode == OP_callpropvoid)
        {
            coder->write(state, pc, OP_pop);  // result
            coder->write(state, pc, OP_pop);  // function
            state->pop(2);
        }
        else
        {
            Value v = state->stackTop();
            // NOTE writeNip is necessary until lir optimizes the "nip"
            // case to avoid the extra copies that result from swap+pop
            coder->writeNip(state, pc);
            state->pop(2);
            state->push(v);
        }
        return true;
    }

    void Verifier::emitFindProperty(AbcOpcode opcode, Multiname& multiname, uint32_t imm30, const byte *pc)
    {
        bool skip_translation = false;
        const ScopeTypeChain* scope = info->declaringScope();
        if (multiname.isBinding())
        {
            int index = scopeBase + state->scopeDepth - 1;
            int base = scopeBase;
            if (scope->size == 0)
            {
                // if scope->size = 0, then global is a local
                // scope, and we dont want to early bind to global.
                base++;
            }
            for (; index >= base; index--)
            {
                Value& v = state->value(index);
                Binding b = toplevel->getBinding(v.traits, &multiname);
                if (b != BIND_NONE)
                {
                    coder->writeOp1(state, pc, OP_getscopeobject, index-scopeBase);
                    state->push(v);
                    return;
                }
                if (v.isWith)
                    break;  // with scope could have dynamic property
            }
            if (index < base)
            {
                // look at captured scope types
                for (index = scope->size-1; index > 0; index--)
                {
                    Traits* t = scope->getScopeTraitsAt(index);
                    Binding b = toplevel->getBinding(t, &multiname);
                    if (b != BIND_NONE)
                    {
                        coder->writeOp1(state, pc, OP_getouterscope, index);
                        state->push(t, true);
                        return;
                    }
                    if (scope->getScopeIsWithAt(index))
                        break;  // with scope could have dynamic property
                }
                if (index <= 0)
                {
                    // look at import table for a suitable script
                    MethodInfo* script = pool->getNamedScript(&multiname);
                    if (script != (MethodInfo*)BIND_NONE && script != (MethodInfo*)BIND_AMBIGUOUS)
                    {
                        if (script == info)
                        {
                            // ISSUE what if there is an ambiguity at runtime? is VT too early to bind?
                            // its defined here, use getscopeobject 0
                            if (scope->size > 0)
                            {
                                coder->writeOp1(state, pc, OP_getouterscope, 0);
                            }
                            else
                            {
                                coder->write(state, pc, OP_getglobalscope);
                            }
                        }
                        else // found a single matching traits
                        {
                            coder->writeOp1(state, pc, OP_finddef, imm30, script->declaringTraits());
                        }
                        state->push(script->declaringTraits(), true);
                        return;
                    }
                    else
                    {
                        switch (opcode) {
                            case OP_findproperty:
                                coder->writeOp1(state, pc, OP_findpropglobal, imm30);
                                break;
                            case OP_findpropstrict:
                                coder->writeOp1(state, pc, OP_findpropglobalstrict, imm30);
                                break;
                            default:
                                AvmAssert(false);
                                break;
                        }
                        skip_translation = true;
                    }
                }
            }
        }
        uint32_t n=1;
        checkPropertyMultiname(n, multiname);
        if (!skip_translation) coder->writeOp1(state, pc, opcode, imm30, OBJECT_TYPE);
        state->pop_push(n-1, OBJECT_TYPE, true);
    }

    void Verifier::emitGetProperty(Multiname &multiname, int n, uint32_t imm30, const byte *pc)
    {
        Value& obj = state->peek(n);

        Binding b = toplevel->getBinding(obj.traits, &multiname);
        Traits* propType = readBinding(obj.traits, b);

        emitCheckNull(state->sp()-(n-1));

        // early bind slot
        if (AvmCore::isSlotBinding(b))
        {
            coder->writeOp1(state, pc, OP_getslot, AvmCore::bindingToSlotId(b), propType);
            state->pop_push(n, propType);
            return;
        }

        // early bind accessor
        if (AvmCore::hasGetterBinding(b))
        {
            // Invoke the getter
            int disp_id = AvmCore::bindingToGetterId(b);
            const TraitsBindingsp objtd = obj.traits->getTraitsBindings();
            MethodInfo *f = objtd->getMethod(disp_id);
            AvmAssert(f != NULL);
            emitCoerceArgs(f, 0);
            coder->writeOp2(state, pc, OP_getproperty, imm30, n, propType);
            AvmAssert(propType == f->getMethodSignature()->returnTraits());
            state->pop_push(n, propType);
            return;
        }
        if( !propType )
        {
            if( obj.traits == VECTORINT_TYPE  || obj.traits == VECTORUINT_TYPE ||
                obj.traits == VECTORDOUBLE_TYPE )
            {
                bool attr = multiname.isAttr();
                Traits* indexType = state->value(state->sp()).traits;
                // NOTE a dynamic name should have the same version as the current pool
                bool maybeIntegerIndex = !attr && multiname.isRtname() && multiname.containsAnyPublicNamespace();
                if( maybeIntegerIndex && (indexType == UINT_TYPE || indexType == INT_TYPE) )
                {
                    if(obj.traits == VECTORINT_TYPE)
                        propType = INT_TYPE;
                    else if(obj.traits == VECTORUINT_TYPE)
                        propType = UINT_TYPE;
                    else if(obj.traits == VECTORDOUBLE_TYPE)
                        propType = NUMBER_TYPE;
                }
            }
        }

        // default - do getproperty at runtime

        coder->writeOp2(state, pc, OP_getproperty, imm30, n, propType);
        state->pop_push(n, propType);
    }

    Traits* Verifier::checkGetGlobalScope()
    {
        const ScopeTypeChain* scope = info->declaringScope();
        int captured_depth = scope->size;
        if (captured_depth > 0) {
            // enclosing scope
            Traits* t = scope->getScopeTraitsAt(0);
            state->push(t, true);
            return t;
        }
        else {
            // local scope
            if (state->scopeDepth == 0)
                verifyFailed(kGetScopeObjectBoundsError, core->toErrorString(0));
            Traits* t = state->scopeValue(0).traits;
            state->push(state->scopeValue(0));
            return t;
        }
    }

    FrameState *Verifier::getFrameState(int target_off)
    {
        return blockStates ? blockStates->get(code_pos + target_off) : NULL;
    }

    bool Verifier::hasFrameState(int target_off)
    {
        return blockStates && blockStates->containsKey(code_pos + target_off);
    }

    int Verifier::getBlockCount()
    {
        return blockStates ? blockStates->size() : 0;
    }

    void Verifier::emitCheckNull(int i)
    {
        Value& value = state->value(i);
        if (!value.notNull) {
            coder->writeCheckNull(state, i);
            value.notNull = true;
        }
    }

    void Verifier::checkCallMultiname(AbcOpcode /*opcode*/, Multiname* name) const
    {
        if (name->isAttr())
        {
            verifyFailed(kIllegalOpMultinameError, core->toErrorString(name), name->format(core));
        }
    }

    Traits* Verifier::emitCoerceSuper(int index)
    {
        Traits* base = info->declaringTraits()->base;
        if (base != NULL)
        {
            emitCoerce(base, index);
        }
        else
        {
            verifyFailed(kIllegalSuperCallError, core->toErrorString(info));
        }
        return base;
    }

    void Verifier::emitCoerce(Traits* target, int index)
    {
        Value &v = state->value(index);
        coder->writeCoerce(state, index, target);
        state->setType(index, target, v.notNull);
    }

    Traits* Verifier::peekType(Traits* requiredType, int n)
    {
        Traits* t = state->peek(n).traits;
        if (t != requiredType)
        {
            verifyFailed(kIllegalOperandTypeError, core->toErrorString(t), core->toErrorString(requiredType));
        }
        return t;
    }

    void Verifier::checkEarlySlotBinding(Traits* t)
    {
        if (!t->allowEarlyBinding())
            verifyFailed(kIllegalEarlyBindingError, core->toErrorString(t));
    }

    void Verifier::emitCoerceArgs(MethodInfo* m, int argc)
    {
        if (!m->isResolved())
            m->resolveSignature(toplevel);

        MethodSignaturep mms = m->getMethodSignature();
        if (!mms->argcOk(argc))
        {
            verifyFailed(kWrongArgumentCountError, core->toErrorString(m), core->toErrorString(mms->requiredParamCount()), core->toErrorString(argc));
        }

        // coerce parameter types
        int n=1;
        while (argc > 0)
        {
            Traits* target = (argc <= mms->param_count()) ? mms->paramTraits(argc) : NULL;
            emitCoerce(target, state->sp()-(n-1));
            argc--;
            n++;
        }

        // coerce receiver type
        emitCoerce(mms->paramTraits(0), state->sp()-(n-1));
    }

    bool Verifier::canAssign(Traits* lhs, Traits* rhs) const
    {
        if (!Traits::isMachineCompatible(lhs,rhs))
        {
            // no machine type is compatible with any other
            return false;
        }

        if (!lhs)
            return true;

        // type on right must be same class or subclass of type on left.
        Traits* t = rhs;
        while (t != lhs && t != NULL)
            t = t->base;
        return t != NULL;
    }

    void Verifier::checkStack(uint32_t pop, uint32_t push)
    {
        if (uint32_t(state->stackDepth) < pop)
            verifyFailed(kStackUnderflowError);
        if (state->stackDepth-pop+push > uint32_t(max_stack))
            verifyFailed(kStackOverflowError);
    }

    void Verifier::checkStackMulti(uint32_t pop, uint32_t push, Multiname* m)
    {
        if (m->isRtname()) pop++;
        if (m->isRtns()) pop++;
        checkStack(pop,push);
    }

    Value& Verifier::checkLocal(int local)
    {
        if (local < 0 || local >= local_count)
            verifyFailed(kInvalidRegisterError, core->toErrorString(local));
        return state->value(local);
    }

    Traits* Verifier::checkSlot(Traits *traits, int imm30)
    {
        uint32_t slot = imm30;
        if (traits)
            traits->resolveSignatures(toplevel);
        TraitsBindingsp td = traits ? traits->getTraitsBindings() : NULL;
        const uint32_t count = td ? td->slotCount : 0;
        if (!traits || slot >= count)
        {
            verifyFailed(kSlotExceedsCountError, core->toErrorString(slot+1), core->toErrorString(count), core->toErrorString(traits));
        }
        return td->getSlotTraits(slot);
    }

    Traits* Verifier::readBinding(Traits* traits, Binding b)
    {
        if (traits)
        {
            traits->resolveSignatures(toplevel);
        }
        else
        {
            AvmAssert(AvmCore::bindingKind(b) == BKIND_NONE);
        }

        switch (AvmCore::bindingKind(b))
        {
        default:
            AvmAssert(false); // internal error - illegal binding type
        case BKIND_GET:
        case BKIND_GETSET:
        {
            int m = AvmCore::bindingToGetterId(b);
            MethodInfo *f = traits->getTraitsBindings()->getMethod(m);
            MethodSignaturep fms = f->getMethodSignature();
            return fms->returnTraits();
        }
        case BKIND_SET:
            // TODO lookup type here. get/set must have same type.
        case BKIND_NONE:
            // dont know what this is
            // fall through
        case BKIND_METHOD:
            // extracted method or dynamic data, don't know which
            return NULL;
        case BKIND_VAR:
        case BKIND_CONST:
            return traits->getTraitsBindings()->getSlotTraits(AvmCore::bindingToSlotId(b));
        }
    }

    MethodInfo* Verifier::checkMethodInfo(uint32_t id)
    {
        const uint32_t c = pool->methodCount();
        if (id >= c)
        {
            verifyFailed(kMethodInfoExceedsCountError, core->toErrorString(id), core->toErrorString(c));
        }

        return pool->getMethodInfo(id);
    }

    Traits* Verifier::checkClassInfo(uint32_t id)
    {
        const uint32_t c = pool->classCount();
        if (id >= c)
        {
            verifyFailed(kClassInfoExceedsCountError, core->toErrorString(id), core->toErrorString(c));
        }

        return pool->getClassTraits(id);
    }

    Traits* Verifier::checkTypeName(uint32_t index)
    {
        Multiname name;
        checkConstantMultiname(index, name); // CONSTANT_Multiname
        Traits *t = pool->getTraits(name, toplevel);
        if (t == NULL)
            verifyFailed(kClassNotFoundError, core->toErrorString(&name));
        else
            if( name.isParameterizedType() )
            {
                core->stackCheck(toplevel);
                Traits* param_traits = name.getTypeParameter() ? checkTypeName(name.getTypeParameter()) : NULL ;
                t = pool->resolveParameterizedType(toplevel, t, param_traits);
            }
        return t;
    }

    MethodInfo* Verifier::checkDispId(Traits* traits, uint32_t disp_id)
    {
        TraitsBindingsp td = traits->getTraitsBindings();
        if (disp_id > td->methodCount)
        {
            verifyFailed(kDispIdExceedsCountError, core->toErrorString(disp_id), core->toErrorString(td->methodCount), core->toErrorString(traits));
        }
        MethodInfo* m = td->getMethod(disp_id);
        if (!m)
        {
            verifyFailed(kDispIdUndefinedError, core->toErrorString(disp_id), core->toErrorString(traits));
        }
        return m;
    }

    void Verifier::verifyFailed(int errorID, Stringp arg1, Stringp arg2, Stringp arg3) const
    {
        #ifdef AVMPLUS_VERBOSE
        if (!secondTry && !verbose) {
            // capture the verify trace even if verbose is false.
            Verifier v2(info, toplevel, abc_env, true);
            v2.verbose = true;
            v2.tryFrom = tryFrom;
            v2.tryTo = tryTo;
            CodeWriter stubWriter;

            // The second verification pass will presumably always throw an
            // error, which we ignore.  But we /must/ catch it so that we can
            // clean up the verifier resources.  Cleanup happens automatically
            // when execution reaches the end of the block.

            TRY(core, kCatchAction_Ignore) {
                v2.verify(&stubWriter);
            }
            CATCH(Exception *ignored) {
                (void)ignored;
            }
            END_CATCH
            END_TRY
        }
        #endif
        toplevel->throwVerifyError(errorID, arg1, arg2, arg3);

        // This function throws, and should never return.
        AvmAssert(false);
    }

    // Merge the current FrameState (this->state) with the target
    // FrameState (getFrameState(target)), and report verify errors.
    // Fixme: Bug 558876 - |current| must not be dereferenced, it could point
    // outside the valid range of bytecodes.  Its only for back-edge detection.
    void Verifier::checkTarget(const byte* current, const byte* target)
    {
        int target_off = int(target - code_pos);
        if (emitPass) {
            AvmAssert(hasFrameState(target_off));
            return;
        }

        // branches must stay inside code, and back edges must land on an OP_label,
        // or a location already known as a forward-branch target
        if (target_off < 0 || target_off >= code_length ||
            (target <= current && !hasFrameState(target_off) && code_pos[target_off] != OP_label)) {
            verifyFailed(kInvalidBranchTargetError);
        }

        FrameState *targetState = getFrameState(target_off);
        bool targetChanged;
        if (!targetState) {
            if (!blockStates)
                blockStates = new (core->GetGC()) GCSortedMap<const byte*, FrameState*, LIST_NonGCObjects>(core->GetGC());
            targetState = mmfx_new( FrameState(this) );
            targetState->pc = target_off;
            blockStates->put(target, targetState);

            // first time visiting target block
            targetChanged = true;
            targetState->init(state);

            #ifdef AVMPLUS_VERBOSE
            if (verbose) {
                core->console << "------------------------------------\n";
                StringBuffer buf(core);
                buf << "MERGE FIRST B" << targetState->pc << ":";
                printState(buf, targetState);
                core->console << "------------------------------------\n";
            }
            #endif
        } else {
            targetChanged = mergeState(targetState);
        }
        bool targetOfBackwardsBranch = targetState->targetOfBackwardsBranch || target <= current;
        if (targetOfBackwardsBranch != targetState->targetOfBackwardsBranch)
            targetChanged |= true;
        targetState->targetOfBackwardsBranch = targetOfBackwardsBranch;
        if (targetChanged && !targetState->wl_pending) {
            targetState->wl_pending = true;
            targetState->wl_next = worklist;
            worklist = targetState;
        }
    }

    bool Verifier::mergeState(FrameState* targetState)
    {
#ifdef AVMPLUS_VERBOSE
        if (verbose) {
            core->console << "------------------------------------\n";
            StringBuffer buf(core);
            buf << "MERGE CURRENT " << (int)state->pc << ":";
            printState(buf, state);
            buf.reset();
            buf << "MERGE TARGET B" << (int)targetState->pc << ":";
            printState(buf, targetState);
        }
#endif

        // check matching stack depth
        if (state->stackDepth != targetState->stackDepth)
            verifyFailed(kStackDepthUnbalancedError, core->toErrorString((int)state->stackDepth), core->toErrorString((int)targetState->stackDepth));

        // check matching scope chain depth
        if (state->scopeDepth != targetState->scopeDepth)
            verifyFailed(kScopeDepthUnbalancedError, core->toErrorString(state->scopeDepth), core->toErrorString(targetState->scopeDepth));

        // Merge types of locals, scopes, and operands.
        // Merge could preserve common interfaces even when
        // common supertype does not:
        //    class A implements I {}
        //    class B implements I {}
        //    var i:I = b ? new A : new B
        // Doing so would require different specification for verify-time analysis,
        // essentially a differnet ABC spec, yet each abc version needs predictable
        // verifier semantics.
        // On the other hand, later optimization passes are free to be as accurate as
        // they like, if it produces better code.

        bool targetChanged = false;
        const int scopeTop  = scopeBase + targetState->scopeDepth;
        const int stackTop  = stackBase + targetState->stackDepth;
        for (int i=0, n=stackTop; i < n; i++)
        {
            // ignore empty locations between scopeTop and stackBase
            if (i >= scopeTop && i < stackBase)
                continue;

            const Value& curValue = state->value(i);
            Value& targetValue = targetState->value(i);

            if (curValue.isWith != targetValue.isWith) {
                // failure: pushwith on one edge, pushscope on other edge, cannot merge.
                verifyFailed(kCannotMergeTypesError, core->toErrorString(targetValue.traits), core->toErrorString(curValue.traits));
            }

            Traits* merged_traits = findCommonBase(targetValue.traits, curValue.traits);
            bool merged_notNull = targetValue.notNull && curValue.notNull;

            if (targetValue.traits != merged_traits || targetValue.notNull != merged_notNull)
                targetChanged = true;

            targetValue.traits = merged_traits;
            targetValue.notNull = merged_notNull;
#ifdef VMCFG_NANOJIT
            uint8_t merged_sst = targetValue.sst_mask | curValue.sst_mask;
            if (targetValue.sst_mask != merged_sst)
                targetChanged = true;
            targetValue.sst_mask = merged_sst;
#endif
        }

#ifdef AVMPLUS_VERBOSE
        if (verbose) {
            StringBuffer buf(core);
            buf << "AFTER MERGE B" << targetState->pc << ":";
            printState(buf, targetState);
            core->console << "------------------------------------\n";
        }
#endif
        return targetChanged;
    }

    /**
     * find common base class of these two types
     */
    Traits* Verifier::findCommonBase(Traits* t1, Traits* t2)
    {
        if (t1 == t2)
            return t1;

        if (t1 == NULL) {
            // assume t1 is always non-null
            Traits *temp = t1;
            t1 = t2;
            t2 = temp;
        }

        if (t1 == NULL_TYPE && t2 && !t2->isMachineType())
        {
            // okay to merge null with pointer type
            return t2;
        }
        if (t2 == NULL_TYPE && t1 && !t1->isMachineType())
        {
            // okay to merge null with pointer type
            return t1;
        }

        // all commonBase flags start out false.  set the cb bits on
        // t1 and its ancestors.
        Traits* t = t1;
        do t->commonBase = true;
        while ((t = t->base) != NULL);

        // now search t2 and its ancestors looking for the first cb=true
        t = t2;
        while (t != NULL && !t->commonBase)
            t = t->base;

        Traits* common = t;

        // finally reset the cb bits to false for next time
        t = t1;
        do t->commonBase = false;
        while ((t = t->base) != NULL);

        return common;
    }

    void Verifier::checkCpoolOperand(uint32_t index, int requiredAtomType)
    {
        switch( requiredAtomType )
        {
        case kStringType:
            if( !index || index >= pool->constantStringCount )
            {
                verifyFailed(kCpoolIndexRangeError, core->toErrorString(index), core->toErrorString(pool->constantStringCount));
            }
            break;

        case kObjectType:
            if( !index || index >= pool->cpool_mn_offsets.size() )
            {
                verifyFailed(kCpoolIndexRangeError, core->toErrorString(index), core->toErrorString(pool->cpool_mn_offsets.size()));
            }
            break;

        default:
            verifyFailed(kCpoolEntryWrongTypeError, core->toErrorString(index));
            break;
        }
    }

    void Verifier::checkConstantMultiname(uint32_t index, Multiname& m)
    {
        checkCpoolOperand(index, kObjectType);
        pool->parseMultiname(m, index);
    }

    Binding Verifier::findMathFunction(TraitsBindingsp math, const Multiname& multiname, Binding b, int argc)
    {
        Stringp newname = core->internString(core->concatStrings(core->internConstantStringLatin1("_"), multiname.getName()));
        Binding newb = math->findBinding(newname);
        if (AvmCore::isMethodBinding(newb))
        {
            int disp_id = AvmCore::bindingToMethodId(newb);
            MethodInfo* newf = math->getMethod(disp_id);
            MethodSignaturep newfms = newf->getMethodSignature();
            const int param_count = newfms->param_count();
            if (argc == param_count)
            {
                for (int i=state->stackDepth-argc, n=state->stackDepth; i < n; i++)
                {
                    Traits* t = state->stackValue(i).traits;
                    if (!t || !t->isNumeric())
                        return b;
                }
                b = newb;
            }
        }
        return b;
    }

    Binding Verifier::findStringFunction(TraitsBindingsp str, const Multiname& multiname, Binding b, int argc)
    {
        Stringp newname = core->internString(core->concatStrings(core->internConstantStringLatin1("_"), multiname.getName()));
        Binding newb = str->findBinding(newname);
        if (AvmCore::isMethodBinding(newb))
        {
            int disp_id = AvmCore::bindingToMethodId(newb);
            MethodInfo* newf = str->getMethod(disp_id);
            // We have all required parameters but not more than required.
            MethodSignaturep newfms = newf->getMethodSignature();
            const int param_count = newfms->param_count();
            const int optional_count = newfms->optional_count();
            if ((argc >= (param_count - optional_count)) && (argc <= param_count))
            {
                for (int i=state->stackDepth-argc, k = 1, n=state->stackDepth; i < n; i++, k++)
                {
                    Traits* t = state->stackValue(i).traits;
                    if (t != newfms->paramTraits(k))
                        return b;
                }
                b = newb;
            }
        }
        return b;
    }

#ifndef SIZE_T_MAX
#  ifdef SIZE_MAX
#    define SIZE_T_MAX SIZE_MAX
#  else
#    define SIZE_T_MAX UINT_MAX
#  endif
#endif

    void Verifier::parseExceptionHandlers()
    {
        if (info->abc_exceptions()) {
            AvmAssert(tryFrom && tryTo);
            return;
        }

        const byte* pos = code_pos + code_length;
        int exception_count = toplevel->readU30(pos);   // will be nonnegative and less than 0xC0000000

        if (exception_count != 0)
        {
            if (exception_count == 0 || (size_t)(exception_count-1) > SIZE_T_MAX / sizeof(ExceptionHandler))
                verifyFailed(kIllegalExceptionHandlerError);

            size_t extra = sizeof(ExceptionHandler)*(exception_count-1);
            ExceptionHandlerTable* table = new (core->GetGC(), extra) ExceptionHandlerTable(exception_count);
            ExceptionHandler *handler = table->exceptions;
            for (int i=0; i < exception_count; i++, handler++)
            {
                handler->from = toplevel->readU30(pos);
                handler->to = toplevel->readU30(pos);
                handler->target = toplevel->readU30(pos);

                const uint8_t* const scopePosInPool = pos;

                int type_index = toplevel->readU30(pos);
                Traits* t = type_index ? checkTypeName(type_index) : NULL;

                Multiname qn;
                int name_index = (pool->version != (46<<16|15)) ? toplevel->readU30(pos) : 0;
                if (name_index != 0)
                {
                    pool->parseMultiname(qn, name_index);
                    if (!qn.isBinding()) {
                        // abc docs specify that the name is a string but asc generates QNames.
                        // Multinames with no namespaces could be supported, but Tamarin currently can't
                        // handle them in this context.
                        verifyFailed(kCorruptABCError); // the error code could be more precise
                    }
                }

                #ifdef AVMPLUS_VERBOSE
                if (verbose)
                {
                    core->console << "            exception["<<i<<"] from="<< handler->from
                        << " to=" << handler->to
                        << " target=" << handler->target
                        << " type=" << t
                        << " name=";
                    if (name_index != 0)
                        core->console << qn;
                    else
                        core->console << "(none)";
                    core->console << "\n";
                }
                #endif

                if (handler->from < 0 ||
                    handler->to < handler->from ||
                    handler->target < handler->to ||
                    handler->target >= code_length)
                {
                    // illegal range in handler record
                    verifyFailed(kIllegalExceptionHandlerError);
                }

                // save maximum try range
                if (!tryFrom || (code_pos + handler->from) < tryFrom)
                    tryFrom = code_pos + handler->from;
                if (code_pos + handler->to > tryTo)
                    tryTo = code_pos + handler->to;

                // note: since we require (code_len > target >= to >= from >= 0),
                // all implicit exception edges are forward edges.

                // handler->traits = t
                WB(core->GetGC(), table, &handler->traits, t);

                Traits* scopeTraits = name_index == 0 ? OBJECT_TYPE :
                    Traits::newCatchTraits(toplevel, pool, scopePosInPool, qn.getName(), qn.getNamespace());

                // handler->scopeTraits = scopeTraits
                WB(core->GetGC(), table, &handler->scopeTraits, scopeTraits);
            }

            info->set_abc_exceptions(core->GetGC(), table);
        }
        else
        {
            info->set_abc_exceptions(core->GetGC(), NULL);
        }
    }

    #ifdef AVMPLUS_VERBOSE
    void Verifier::printOpcode(const byte* pc)
    {
        int offset = int(pc - code_pos);
        core->console << "  " << offset << ':';
        core->formatOpcode(core->console, pc, (AbcOpcode)*pc, offset, pool);
        core->console << '\n';
    }

    /**
     * display contents of current stack frame only.
     */
    void Verifier::printState(StringBuffer& prefix, FrameState *state)
    {
        PrintWriter& out = core->console;
        if (prefix.length() > 0) {
            char buf[80]; // plenty for currently known prefixes
            VMPI_sprintf(buf, "%-23s[", prefix.c_str());
            out << buf;
        } else {
            out << "                       [";
        }

        // locals
        for (int i=0, n = scopeBase; i < n; i++) {
            printValue(state->value(i));
            if (i+1 < n)
                out << ' ';
        }
        out << "] {";

        // scope chain
        for (int i = scopeBase, n = scopeBase + state->scopeDepth; i < n; i++) {
            printValue(state->value(i));
            if (i+1 < n)
                out << ' ';
        }
        out << "} (";

        // stack
        int stackStart;
        const int stackLimit = 20; // don't display more than this, to reduce verbosity
        if (state->stackDepth > stackLimit) {
            stackStart = stackBase + state->stackDepth - stackLimit;
            out << "..." << stackStart << ": ";
        } else {
            stackStart = stackBase;
        }
        for (int i = stackStart, n = stackBase + state->stackDepth; i < n; i++) {
            printValue(state->value(i));
            if (i+1 < n)
                out << ' ';
        }
        out << ")\n";
    }

    /** display contents of a captured scope chain */
    void Verifier::printScope(const char* title, const ScopeTypeChain* scope)
    {
        PrintWriter& out = core->console;
        out << "  " << title << " = ";
        if (scope && scope->size > 0) {
            out << '[';
            for (int i=0, n=scope->size; i < n; i++) {
                Traits* t = scope->getScopeTraitsAt(i);
                if (!t)
                    out << "*!";
                else
                    out << t->format(core);
                if (i+1 < n)
                    out << ' ';
            }
            out << "]\n";
        } else {
            out << "null\n";
        }
    }

    void Verifier::printValue(Value& v)
    {
        Traits* t = v.traits;
        PrintWriter& out = core->console;
        if (!t) {
            out << (v.notNull ? "*!" : "*");
        } else {
            out << t->format(core);
            if (!t->isNumeric() && t != BOOLEAN_TYPE && t != NULL_TYPE && t != VOID_TYPE)
                out << (v.notNull ? "" : "?");
        }
#ifdef VMCFG_NANOJIT
        if (v.sst_mask) {
            out << '[';
            if (v.sst_mask & (1 << SST_atom))            out << 'A';
            if (v.sst_mask & (1 << SST_string))          out << 'S';
            if (v.sst_mask & (1 << SST_namespace))       out << 'N';
            if (v.sst_mask & (1 << SST_scriptobject))    out << 'O';
            if (v.sst_mask & (1 << SST_int32))           out << 'I';
            if (v.sst_mask & (1 << SST_uint32))          out << 'U';
            if (v.sst_mask & (1 << SST_bool32))          out << 'B';
            if (v.sst_mask & (1 << SST_double))          out << 'D';
            out << ']';
        }
#endif
    }
    #endif /* AVMPLUS_VERBOSE */

    FrameState::FrameState(Verifier* verifier)
        : verifier(verifier), wl_next(NULL),
          pc(0), scopeDepth(0), stackDepth(0), withBase(-1),
          targetOfBackwardsBranch(false),
          wl_pending(false)
    {
        locals = (Value*)mmfx_alloc_opt(sizeof(Value) * verifier->frameSize, MMgc::kZero);
    }

    FrameState::~FrameState() {
        mmfx_free( locals );
    }

#if defined FEATURE_CFGWRITER
    Block::Block(uint32_t label, int32_t begin)
        : label(label), begin(begin), end(0), succ(0)
        , pred_count(0)
    {}

    Block::~Block()
    {}

    Edge::Edge(uint32_t src, uint32_t snk)
        : src(src), snk(snk)
    {}

    CFGWriter::CFGWriter (MethodInfo* info, CodeWriter* coder)
        : NullWriter(coder), info(info), label(0), edge(0) {
            blocks.put(0, mmfx_new( Block(label++, 0)));
        current = blocks.at(0);
        current->pred_count = -1;
    }

    CFGWriter::~CFGWriter() {
        for (int i=0, n=blocks.size(); i < n; i++)
            mmfx_delete( blocks.at(i) );
        for (int i=0, n=edges.size(); i < n; i++)
            mmfx_delete( edges.at(i) );
    }

    void CfgWriter::cleanup()
    {
        // this is only called on abnormal paths where the dtor wouldn't otherwise run at all.
        coder->cleanup();
        this->~CFGWriter();
    }

    void CFGWriter::writeEpilogue(const FrameState* state)
    {
        Block* b;
        AvmCore *core = info->pool()->core;
        core->console << "CFG " << info << "\n";
        for (int i = 0; i < blocks.size(); i++) {
          b = blocks.at(i);
          core->console << "B" << b->label; // << " @" << (int)b->begin << ", @" << (int)b->end;
          core->console << " preds=[";
          for (uint32_t j = 0; j < b->preds.size(); j++) {
            if(j!=0) core->console << ",";
            core->console << "B" << b->preds.get(j);
          }
          core->console << "] succs=[";
          for (uint32_t j = 0; j < b->succs.size(); j++) {
            if(j!=0) core->console << ",";
            core->console << "B" << b->succs.get(j);
          }
          core->console << "]\n";
        }
        Edge* e;
        for (int i = 0; i < edges.size(); i++) {
          e = edges.at(i);
          core->console << "E" << i << ": " << "B" << e->src << " --> " << "B" << e->snk << "\n";
        }
        core->console << "\n";

        coder->writeEpilogue(state);
    }

    void CFGWriter::write(const FrameState* state, const byte* pc, AbcOpcode opcode, Traits*type)
    {
      //AvmLog ("%i: %s\n", state->pc, opcodeInfo[opcode].name);
      Block* b = blocks.get(state->pc);
      if (b) {
        current = b;
      }

      switch (opcode) {
      case OP_label:
      {
        //core->console << "  " << (uint32_t)state->pc << ":" << opcodeInfo[opcode].name << "\n";
        //core->console << "label @ " << (uint32_t)state->pc << "\n";
        Block *b = blocks.get(state->pc);
        // if there isn't a block for the current pc, then create one
        if (!b) {
          b = mmfx_new(Block(label++, state->pc));
          //b->pred_count++;
          blocks.put(state->pc, b);
          current = b;
        }
        break;
      }
      case OP_returnvoid:
        current->end = state->pc+1;
        break;
      default:
        break;
      }

        coder->write(state, pc, opcode, type);
    }

    void CFGWriter::writeOp1(const FrameState* state, const byte *pc, AbcOpcode opcode, uint32_t opd1, Traits *type)
    {
      //AvmLog ("%i: %s\n", state->pc, opcodeInfo[opcode].name);
      Block* b=blocks.get(state->pc);
      if (b) {
        current = b;
      }

      //AvmLog ("%i: %s %i\n", state->pc, opcodeInfo[opcode].name, opd1);
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
        case OP_iftrue:
        case OP_iffalse:
        case OP_jump:
        {
          //core->console << "  " << (uint32_t)state->pc << ":" << opcodeInfo[opcode].name;
          //core->console << " " << (uint32_t)state->pc+opd1+4 << "\n";
          Block *b = blocks.get(state->pc+4);

          // if there isn't a block for the next pc, then create one
          if (!b) {
            b = mmfx_new( Block(label++, state->pc+4) );
            blocks.put(state->pc+4, b);
          }
          if (opcode != OP_jump) {
              b->pred_count++;
              b->preds.add(current->label);
              current->succs.add(b->label);
              edges.put(edge++, mmfx_new(Edge(current->label, b->label)));
          }


          // if there isn't a block for target then create one
          b = blocks.get(state->pc+4+opd1);
          if (!b) {
            b = mmfx_new( Block(label++, state->pc+4+opd1));
            blocks.put(state->pc+4+opd1, b);
          }

          if ((int)opd1>0)
          {
              b->pred_count++;
          }
          else
          {
              b->pred_count = -1;
          }
          b->preds.add(current->label);
          current->succs.add(b->label);
          current->end = state->pc+4;
          edges.put(edge++, mmfx_new(Edge(current->label, b->label)));

          //core->console << "label " << (uint32_t)state->pc+opd1+4 << "\n";
          //core->console << "    edge " << (uint32_t)state->pc << " -> " << (uint32_t)state->pc+opd1+4 << "\n";
            break;
        }
        default:
          //core->console << " " << (int)opd1 << "\n";
            break;
        }

        coder->writeOp1(state, pc, opcode, opd1, type);
    }

    void CFGWriter::writeOp2(const FrameState* state, const byte *pc, AbcOpcode opcode, uint32_t opd1, uint32_t opd2, Traits* type)
    {
      //AvmLog ("%i: %s\n", state->pc, opcodeInfo[opcode].name);
      Block* b=blocks.get(state->pc);
      if (b) {
        current = b;
      }

      //AvmLog ("%i: %s %i %i\n", state->pc, opcodeInfo[opcode].name, opd1, opd2);
      //core->console << "  " << (uint32_t)state->pc << ":" << opcodeInfo[opcode].name << " " << opd1 << " " << opd2 << "\n";
        coder->writeOp2 (state, pc, opcode, opd1, opd2, type);
    }
    #endif // FEATURE_CFGWRITER
}
