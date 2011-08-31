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

#ifndef __avmplus_AtomArray_inlines__
#define __avmplus_AtomArray_inlines__

namespace avmplus
{
    using namespace MMgc;

    REALLY_INLINE void AtomArray::setAtInternal(uint32 index, Atom a)
    {
        // m_arr[index] = a;
        WBATOM( MMgc::GC::GetGC(m_atoms), m_atoms, m_atoms + index, a);
    }

    REALLY_INLINE uint32 AtomArray::getLength() const 
    {
        return m_length; 
    }

    REALLY_INLINE void AtomArray::setLength(uint32 len) 
    {
        m_length = len; 
    }

    REALLY_INLINE uint32 AtomArray::capacity()  const
    {
        return (uint32)(MMgc::GC::Size(m_atoms)/sizeof(Atom));
    }

    REALLY_INLINE Atom AtomArray::operator[](uint32 index) const 
    {
        return getAt(index); 
    }

    REALLY_INLINE Atom AtomArray::getAtFast(uint32 index) const
    {
        AvmAssert(index < m_length);
        return m_atoms[index];
    }

    REALLY_INLINE void AtomArray::setAtoms(MMgc::GC *gc, Atom *atoms)
    {
        WB(gc, gc->FindBeginningFast(this), &m_atoms, atoms);
    }

    REALLY_INLINE void AtomArray::moveAtoms(Atom *atoms, uint32_t dstOffset, uint32_t srcOffset, size_t numAtoms)
    {
        MMgc::GC::GetGC(this)->movePointers((void**)atoms, dstOffset, (const void**)atoms, srcOffset, numAtoms);
    }

    REALLY_INLINE void AtomArray::setAt (uint32 index, Atom a)
    {
        if (index > m_length)
        {
            AvmAssert(0);
            return;
        }

        setAtInternal(index, a);
    }

    REALLY_INLINE Atom AtomArray::getAt (uint32 index) const
    {
        if (index > m_length)
        {
            AvmAssert(0);
            return nullObjectAtom;
        }

        return m_atoms[index];
    }

    REALLY_INLINE void AtomArray::push (const AtomArray *a)
    {
        if (!a)
            return;

        push (a->m_atoms, a->getLength());
    }

} // namespace avmplus

#endif /* __avmplus_AtomArray_inlines__ */
