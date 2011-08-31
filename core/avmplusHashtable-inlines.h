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

#ifndef __avmplus_Hashtable_inlines__
#define __avmplus_Hashtable_inlines__


namespace avmplus
{
    REALLY_INLINE InlineHashtable::InlineHashtable() :
        m_atomsAndFlags(0), m_size(0), m_logCapacity(0)
    {
        // nothing, here only for HeapHashtable, which explicitly calls initialize()
    }

    // do NOT make this virtual; we want InlineHashtable to NOT have ANY virtual methods, not even a dtor
    REALLY_INLINE InlineHashtable::~InlineHashtable()
    {
        // nothing, here only for HeapHashtable, which explicitly calls destroy()
    }

    REALLY_INLINE void InlineHashtable::initializeWithDontEnumSupport(MMgc::GC* gc, int capacity)
    {
        initialize(gc, capacity);
        m_atomsAndFlags |= kDontEnumBit;
    }

    REALLY_INLINE uint32_t InlineHashtable::getSize() const
    {
        return m_size;
    }

    REALLY_INLINE bool InlineHashtable::needsInitialize() const
    {
        return m_logCapacity == 0;
    }

    REALLY_INLINE uint32_t InlineHashtable::getCapacity() const
    {
        return m_logCapacity ? 1UL<<(m_logCapacity-1) : 0;
    }

    REALLY_INLINE uintptr_t InlineHashtable::hasDontEnumSupport() const
    {
        return (m_atomsAndFlags & kDontEnumBit);
    }

    REALLY_INLINE Atom InlineHashtable::removeDontEnumMask(Atom a) const
    {
        return Atom(uintptr_t(a) & ~(m_atomsAndFlags & kDontEnumBit));
    }

    REALLY_INLINE bool InlineHashtable::enumerable(Atom a) const
    {
        return a != EMPTY && a != DELETED && !(uintptr_t(a) & (m_atomsAndFlags & kDontEnumBit));
    }

#ifdef DEBUGGER
    REALLY_INLINE uint64_t InlineHashtable::bytesUsed() const
    {
        return getCapacity() * sizeof(Atom);
    }
#endif

    REALLY_INLINE uintptr_t InlineHashtable::hasDeletedItems() const
    {
        return (m_atomsAndFlags & kHasDeletedItems);
    }

    REALLY_INLINE void InlineHashtable::reset()
    {
        MMgc::GC* gc = MMgc::GC::GetGC(getAtoms());
        destroy();
        initialize(gc);
    }

    REALLY_INLINE Atom InlineHashtable::getNonEmpty(Atom name) const
    {
        const Atom* const atoms = getAtoms();
        int const i = find(name, atoms, getCapacity());
        return isEmpty(atoms[i]) ? 0 : atoms[i+1];
    }

    REALLY_INLINE /*static*/ bool InlineHashtable::isEmpty(Atom a)
    {
        return a == 0; // can't use EMPTY unless you like link errors.
    }

    REALLY_INLINE bool InlineHashtable::contains(Atom name) const
    {
        const Atom* atoms = getAtoms();
        return removeDontEnumMask(atoms[find(name, atoms, getCapacity())]) == name;
    }

    REALLY_INLINE int InlineHashtable::find(Atom x) const
    {
        return find(x, getAtoms(), getCapacity());
    }

    REALLY_INLINE void InlineHashtable::setHasDeletedItems()
    {
        m_atomsAndFlags |= kHasDeletedItems;
    }

    REALLY_INLINE void InlineHashtable::clrHasDeletedItems()
    {
        m_atomsAndFlags &= ~kHasDeletedItems;
    }

    REALLY_INLINE Atom* InlineHashtable::getAtoms()
    {
        return (Atom*)(m_atomsAndFlags & ~kAtomFlags);
    }

    REALLY_INLINE const Atom* InlineHashtable::getAtoms() const
    {
        return (const Atom*)(m_atomsAndFlags & ~kAtomFlags);
    }

    REALLY_INLINE Atom InlineHashtable::keyAt(int i) const
    {
        int const index = (i-1)<<1;
        int const cap = getCapacity();
        return index < cap ? removeDontEnumMask(getAtoms()[index]) : nullStringAtom;
    }

    REALLY_INLINE Atom InlineHashtable::valueAt(int i) const
    {
        int const index = ((i-1)<<1)+1;
        int const cap = getCapacity();
        return index < cap ? getAtoms()[index] : undefinedAtom;
    }

    // @todo -- move this mess to VMPI, see https://bugzilla.mozilla.org/show_bug.cgi?id=546354
#if defined(AVMPLUS_IA32) || defined(AVMPLUS_AMD64)
    REALLY_INLINE /*static*/ uint32_t InlineHashtable::FindOneBit(uint32_t value)
    {

    #ifndef __GNUC__
        #if defined(_MSC_VER) && defined(AVMPLUS_64BIT)
        unsigned long index;
        _BitScanReverse(&index, value);
        return (uint32)index;
        #elif defined(__SUNPRO_C)||defined(__SUNPRO_CC)
        for (int i=0; i < 32; i++)
            if (value & (1<<i))
                return i;
        // asm versions of this function are undefined if no bits are set
        AvmAssert(false);
        return 0;
        #else
        _asm
        {
            bsr eax,[value];
        }
        #endif
    #else
        // DBC - This gets rid of a compiler warning and matchs PPC results where value = 0
        register int    result = ~0;

        if (value)
        {
            asm (
                "bsr %1, %0"
                : "=r" (result)
                : "m"(value)
                );
        }
        return result;
    #endif
    }

#elif defined(AVMPLUS_PPC)

    REALLY_INLINE /*static*/ uint32_t InlineHashtable::FindOneBit(uint32_t value)
    {
        register int index;
        #ifdef __GNUC__
        asm ("cntlzw %0,%1" : "=r" (index) : "r" (value));
        #else
        register uint32 in = value;
        asm { cntlzw index, in; }
        #endif
        return 31-index;
    }

#else // generic platform

    REALLY_INLINE /*static*/ uint32_t InlineHashtable::FindOneBit(uint32_t value)
    {
        for (int i=0; i < 32; i++)
            if (value & (1<<i))
                return i;
        // asm versions of this function are undefined if no bits are set
        AvmAssert(false);
        return 0;
    }

#endif

    REALLY_INLINE void InlineHashtable::setCapacity(uint32_t cap)
    {
        m_logCapacity = cap ? (FindOneBit(cap)+1) : 0;
        AvmAssert(getCapacity() == cap);
    }

    REALLY_INLINE HeapHashtableRC::HeapHashtableRC(MMgc::GC* gc, int32_t capacity /*= InlineHashtable::kDefaultCapacity*/)
    {
        ht.initialize(gc, capacity);
    }

    REALLY_INLINE void HeapHashtableRC::reset()
    {
        ht.reset();
    }

    REALLY_INLINE uint32_t HeapHashtableRC::getSize() const
    {
        return ht.getSize();
    }

    REALLY_INLINE int HeapHashtableRC::next(int index)
    {
        return ht.next(index);
    }

    REALLY_INLINE Atom HeapHashtableRC::keyAt(int index)
    {
        return ht.keyAt(index);
    }

    REALLY_INLINE MMgc::RCObject* HeapHashtableRC::valueAt(int index)
    {
        return untagAtom(ht.valueAt(index));
    }

    REALLY_INLINE void HeapHashtableRC::add(Atom name, MMgc::RCObject* value, Toplevel* toplevel /*=NULL*/)
    {
        ht.add(name, tagObject(value), toplevel);
    }

    REALLY_INLINE MMgc::RCObject* HeapHashtableRC::get(Atom name)
    {
        return untagAtom(ht.get(name));
    }

    REALLY_INLINE MMgc::RCObject* HeapHashtableRC::remove(Atom name)
    {
        return untagAtom(ht.remove(name));
    }

    REALLY_INLINE bool HeapHashtableRC::contains(Atom name) const
    {
        return ht.contains(name);
    }

    // used by Flash
    REALLY_INLINE size_t HeapHashtableRC::getAllocatedSize() const
    {
        return ht.getCapacity() * sizeof(Atom);
    }

    REALLY_INLINE Atom HeapHashtableRC::tagObject(MMgc::RCObject* obj)
    {
        return (Atom)obj | kObjectType;
    }

    REALLY_INLINE MMgc::RCObject* HeapHashtableRC::untagAtom(Atom a)
    {
        return (MMgc::RCObject*)atomPtr(a);
    }

    REALLY_INLINE HeapHashtable::HeapHashtable(MMgc::GC* gc, int32_t capacity /*= InlineHashtable::kDefaultCapacity*/)
    {
        ht.initialize(gc, capacity);
    }

    REALLY_INLINE InlineHashtable* HeapHashtable::get_ht()
    {
        return &ht;
    }

    REALLY_INLINE void HeapHashtable::reset()
    {
        ht.reset();
    }

    REALLY_INLINE uint32_t HeapHashtable::getSize() const
    {
        return ht.getSize();
    }

    REALLY_INLINE Atom HeapHashtable::keyAt(int index)
    {
        return ht.keyAt(index);
    }

    REALLY_INLINE Atom HeapHashtable::valueAt(int index)
    {
        return ht.valueAt(index);
    }

    REALLY_INLINE WeakKeyHashtable::WeakKeyHashtable(MMgc::GC* _gc) : HeapHashtable(_gc)
    {
    }

    REALLY_INLINE WeakValueHashtable::WeakValueHashtable(MMgc::GC* _gc) : HeapHashtable(_gc)
    {
    }
}

#endif /* __avmplus_Hashtable_inlines__ */
