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

#ifndef __avmplus_VTable__
#define __avmplus_VTable__


namespace avmplus
{
#if defined FEATURE_NANOJIT
    class ImtThunkEnv;

    typedef uintptr_t GprImtThunkProcRetType;

    typedef GprImtThunkProcRetType (*GprImtThunkProc)(ImtThunkEnv*, int, uint32*, uintptr_t);

#endif

    class VTable : public MMgc::GCObject
    {
#if defined FEATURE_NANOJIT
        friend class CodegenLIR;
        friend class ImtThunkEnv;
#endif
    private:
        MethodEnv* makeMethodEnv(MethodInfo* method, ScopeChain* scope);

#if defined FEATURE_NANOJIT
        void resolveImtSlot(uint32_t slot);

        // Helpers for resolveImtSlot
        void resolveImtSlotFromBase(uint32_t slot);
        bool resolveImtSlotSelf(uint32_t slot);

        // return uint64_t, not uintptr_t: see note for GprImtThunkProc
        static GprImtThunkProcRetType resolveImt(ImtThunkEnv* ite, int argc, uint32* ap, uintptr_t iid);
        static GprImtThunkProcRetType dispatchImt(ImtThunkEnv* ite, int argc, uint32* ap, uintptr_t iid);

    public:
#if defined FEATURE_NANOJIT
        // choose a number that is relatively prime to sizeof(MethodInfo)/8
        // since we use the MethodInfo pointer as the interface method id
        // smaller = dense table, few large conflict stubs
        // larger  = sparse table, many small conflict stubs

    #ifdef _DEBUG
        enum { IMT_SIZE = 3 };  // good for testing all code paths
    #else
        enum { IMT_SIZE = 7 };  // good for performance
    #endif
#endif // FEATURE_NANOJIT

#endif

    public:
        VTable(Traits* traits, VTable* base, Toplevel* toplevel);
        void resolveSignatures(ScopeChain* scope);

        VTable* newParameterizedVTable(Traits* param_traits, Stringp fullname);

        size_t getExtraSize() const;
        MMgc::GC* gc() const;
        AvmCore* core() const;
        Toplevel* toplevel() const;

#ifdef AVMPLUS_VERBOSE
        Stringp format(AvmCore* core) const;
#endif

#ifdef DEBUGGER
        /**
         * Basically the same as AvmPlusScriptableObject::bytesUsed().
         */
        uint64_t bytesUsed() const;
#endif

    // ------------------------ DATA SECTION BEGIN
	public:
        Toplevel* const _toplevel;
    public:
        DWB(MethodEnv*) init;
        DWB(VTable*) base;
        DWB(VTable*) ivtable;
        Traits* const traits;
        ScriptObject* (*createInstance)(ClassClosure* cls, VTable* ivtable);
        bool basecase;
        bool linked;    // @todo -- surely there's a spare bit we can use for this.
        bool pad[2];

#if defined FEATURE_NANOJIT
        ImtThunkEnv* imt[VTable::IMT_SIZE];
#endif
        MethodEnv* methods[1]; // virtual method table
    // ------------------------ DATA SECTION END
    };
}

#endif // __avmplus_VTable__

