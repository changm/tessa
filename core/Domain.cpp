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


#include "avmplus.h"

namespace avmplus
{
    Domain::Domain(AvmCore* core, Domain* base) :
        m_base(base),
        m_core(core),
        m_namedTraits(new (core->GetGC()) MultinameHashtable()),
        m_namedScripts(new (core->GetGC()) MultinameHashtable()),
        m_parameterizedTypes(new (core->GetGC(), 0) HeapHashtable(core->GetGC()))
    {
    }

    Traits* Domain::getNamedTraits(Stringp name, Namespacep ns)
    {
        Traits* f = NULL;
        Domain* dom = this;
        do {
            f = (Traits*) dom->m_namedTraits->get(name, ns);
            dom = dom->m_base;
        } while (!f && dom);
        return f;
    }

    Traits* Domain::addUniqueTrait(Stringp name, Namespace* ns, Traits* v)
    {
        Traits* t = getNamedTraits(name, ns);
        if (t == NULL) {
            m_namedTraits->add(name, ns, (Binding)v);
            t = v; // return trait that we'd get from a getNamedTrait() call.
        }
        return t;
    }

    MethodInfo* Domain::getNamedScript(Stringp name, Namespacep ns) const
    {
        MethodInfo* f = NULL;
        const Domain* dom = this;
        do {
            f = (MethodInfo*) dom->m_namedScripts->get(name, ns);
            dom = dom->m_base;
        } while (!f && dom);
        return f;
    }

    MethodInfo* Domain::getNamedScript(const Multiname* mn) const
    {
        MethodInfo* f = NULL;
        const Domain* dom = this;
        do {
            f = (MethodInfo*) dom->m_namedScripts->getMulti(mn);
            dom = dom->m_base;
        } while (!f && dom);
        return f;
    }

    MethodInfo* Domain::addUniqueScript(Stringp name, Namespace* ns, MethodInfo* v)
    {
        MethodInfo* t = getNamedScript(name, ns);
        if (t == NULL) {
            m_namedScripts->add(name, ns, (Binding)v);
            t = v; // return script that we'd get from a getNamedScript() call.
        }
        return t;
    }

    ClassClosure* Domain::getParameterizedType(ClassClosure* type)
    {
        AvmAssert(type != NULL);
        Atom a = type ? m_parameterizedTypes->get(type->atom()) : nullObjectAtom;
        return AvmCore::isObject(a) ? (ClassClosure*)AvmCore::atomToScriptObject(a) : NULL;
    }

    void Domain::addParameterizedType(ClassClosure* type, ClassClosure* parameterizedType)
    {
        AvmAssert(type && parameterizedType);
        if (type && parameterizedType)
        {
            AvmAssert(!m_parameterizedTypes->contains(type->atom()));
            m_parameterizedTypes->add(type->atom(), parameterizedType->atom());
        }
    }
}


