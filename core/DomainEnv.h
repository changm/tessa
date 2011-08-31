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
 * Portions created by the Initial Developer are Copyright (C) 1993-2006
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


#ifndef __avmplus_DomainEnv__
#define __avmplus_DomainEnv__

namespace avmplus
{
    // needs to be a subclass of GCObject so we can convert to/from a WeakRef...
    // DomainEnv wants to be a finalized object anyway so let's use GCFinalizedObject
    class GlobalMemorySubscriber : public MMgc::GCFinalizedObject
    {
    public:
        virtual void notifyGlobalMemoryChanged(uint8_t* newBase, uint32_t newSize) = 0;
    };

    // an ABC
    class GlobalMemoryProvider
    {
    public:
        virtual ~GlobalMemoryProvider() {}
        virtual bool addSubscriber(GlobalMemorySubscriber* subscriber) = 0;
        virtual bool removeSubscriber(GlobalMemorySubscriber* subscriber) = 0;
    };

    class DomainEnv : public GlobalMemorySubscriber
    {
        friend class MopsRangeCheckFilter;
    public:
        DomainEnv(AvmCore *core, Domain *domain, DomainEnv* base);
        virtual ~DomainEnv();

        // these peek into base DomainEnv as appropriate
        MethodEnv* getScriptInit(Namespacep ns, Stringp name) const;
        MethodEnv* getScriptInit(const Multiname& multiname) const;

        inline ScriptEnv* getNamedScript(Stringp name) const { return (ScriptEnv*)m_namedScripts->getName(name); }
        inline ScriptEnv* getNamedScript(Stringp name, Namespacep ns) const { return (ScriptEnv*)m_namedScripts->get(name, ns); }
        inline void addNamedScript(Stringp name, Namespacep ns, ScriptEnv* scriptEnv) { m_namedScripts->add(name, ns, Binding(scriptEnv)); }

        inline Domain* domain() const { return m_domain; }
        inline DomainEnv* base() const { return m_base; }

        /**
         * Allow caller to enumerate the named entries in the table.
         */
        int scriptNext(int index) const;
        Stringp scriptNameAt(int index) const;
        Namespace* scriptNsAt(int index) const;

        Toplevel* toplevel() const;
        void setToplevel(Toplevel *t) { m_toplevel = t; }

        /**
         * global memory access glue
         */
        enum {
            // Must be at least 8 [ie, largest single load/store op we provide]
            // But using larger values allows us to collapse a lot of range checks in the JIT
            GLOBAL_MEMORY_MIN_SIZE = 1024
        };

        REALLY_INLINE uint8_t* globalMemoryBase() const { return m_globalMemoryBase; }
        REALLY_INLINE uint32_t globalMemorySize() const { return m_globalMemorySize; }

        // global memory object accessor (will always be a ByteArray but
        // ByteArray isn't part of AVMPlus proper so plumbing is a little
        // weird...)
        ScriptObject* get_globalMemory() const { return m_globalMemoryProviderObject; }
        bool set_globalMemory(ScriptObject* providerObject);

        // from GlobalMemorySubscriber
        /*virtual*/ void notifyGlobalMemoryChanged(uint8_t* newBase, uint32_t newSize);

    private:
        // subscribes to the memory object "mem" such that "mem" will call our
        // notifyGlobalMemoryChanged when it moves
        bool globalMemorySubscribe(ScriptObject* providerObject);
        // stops "mem" from notifying us if it moves
        bool globalMemoryUnsubscribe(ScriptObject* providerObject);

    private:

        // allocate "scratch" as a struct to make it easier to allocate pre-zeroed
        struct Scratch
        {
            uint8_t scratch[GLOBAL_MEMORY_MIN_SIZE];
        };

    // ------------------------ DATA SECTION BEGIN
    private:
        Domain* const                   m_domain;       // Domain associated with this DomainEnv
        DomainEnv* const                m_base;         // Parent DomainEnv
        DWB(MultinameHashtable*)        m_namedScripts; // table of named program init functions. (ns,name => MethodEnv)
        DWB(Toplevel*)                  m_toplevel;
        // scratch memory to use if the memory object is NULL...
        // allocated via mmfx_new, which is required by nanojit
        Scratch*                        m_globalMemoryScratch;
        // backing store / current size for global memory
        uint8_t*                        m_globalMemoryBase;
        uint32_t                        m_globalMemorySize;
        // the actual memory object (can be NULL)
        DRCWB(ScriptObject*)            m_globalMemoryProviderObject;
    // ------------------------ DATA SECTION END
    };
}

#endif /* __avmplus_DomainEnv__ */
