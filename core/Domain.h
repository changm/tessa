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

#ifndef __avmplus_Domain__
#define __avmplus_Domain__

namespace avmplus
{
    class Domain : public MMgc::GCObject
    {
    public:
        Domain(AvmCore* core, Domain* base);

        Traits* getNamedTraits(Stringp name, Namespacep ns);
        MethodInfo* getNamedScript(Stringp name, Namespacep ns) const;
        MethodInfo* getNamedScript(const Multiname* mn) const;

        Traits* addUniqueTrait(Stringp name, Namespace* ns, Traits* v) ;
        MethodInfo* addUniqueScript(Stringp name, Namespace* ns, MethodInfo* v);

        // returns NULL if the type doesn't exist yet.
        ClassClosure* getParameterizedType(ClassClosure* type);
        void addParameterizedType(ClassClosure* type, ClassClosure* parameterizedType);

        REALLY_INLINE Domain* base() const { return m_base; }
        REALLY_INLINE AvmCore* core() const { return m_core; }

    private:
        Domain* const                   m_base;
        AvmCore* const                  m_core;
        /** The domain-wide traits table (type name => instance Traits) */
        DWB(MultinameHashtable*)        m_namedTraits;
        /** domain-wide type table of scripts, indexed by definition name */
        DWB(MultinameHashtable*)        m_namedScripts;
        DWB(HeapHashtable*)             m_parameterizedTypes;
    };

}

#endif /* __avmplus_Domain__ */
