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

#ifndef __GCHashtable__
#define __GCHashtable__

namespace MMgc
{
    /**
    * simplified version of avmplus hashtable
    */
    template <class KEYHANDLER, class ALLOCHANDLER>
    class GCHashtableBase
    {
    public:
        static uint32_t const kDefaultSize = 16;

        GCHashtableBase(uint32_t capacity = kDefaultSize);
        ~GCHashtableBase();

        void clear();

        REALLY_INLINE const void* get(const void* key) { return table[find(key, table, tableSize)+1]; }
        REALLY_INLINE const void* get(intptr_t key) { return get((const void*)key); }
        const void* remove(const void* key);
        // updates value if present, adds and grows if necessary if not
        void put(const void* key, const void* value);
        REALLY_INLINE void add(const void* key, const void* value) { put(key, value); }
        REALLY_INLINE void add(intptr_t key, const void* value) { put((const void*)key, value); }
        REALLY_INLINE uint32_t count() const { return numValues - numDeleted; }

        int32_t nextIndex(int32_t index);
        const void* keyAt(int32_t index) const { return table[index<<1]; }
        const void* valueAt(int32_t index) const { return table[((index)<<1)+1]; }

        class Iterator
        {
        public:
            Iterator(GCHashtableBase* _ht) : ht(_ht), index(-2) {}

            const void* nextKey()
            {
                do {
                    index += 2;
                } while(index < (int32_t)ht->tableSize && ht->table[index] <= GCHashtableBase::DELETED);

                return (index < (int32_t)ht->tableSize) ? ht->table[index] : NULL;
            }

            const void* value()
            {
                GCAssert(ht->table[index] != NULL);
                return ht->table[index+1];
            }

        private:
            GCHashtableBase* volatile ht;
            int32_t index;
        };

    private:
        uint32_t find(const void* key, const void** table, uint32_t tableSize);

        void grow(bool isRemoval);

        static const void* const DELETED;// = (const void*)1;
        static const void* EMPTY[4];// = { NULL, NULL, NULL, NULL };

    protected:
        const void** table;     // table elements
        uint32_t tableSize;     // capacity
        uint32_t numValues;     // size of table array
        uint32_t numDeleted;    // number of delete items
    };

    // --------------------------------

    template <class KEYHANDLER, class ALLOCHANDLER>
    /*static*/ const void* const GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::DELETED = (const void*)1;

    template <class KEYHANDLER, class ALLOCHANDLER>
    /*static*/ const void* GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::EMPTY[4] = { NULL, NULL, NULL, NULL };

    // --------------------------------

    /*
        Why do we need two allocator options?

        On some platforms FixedMalloc is known to be faster than the system (VMPI) allocator,
        but really its about OOM: when we run out of memory we can shutdown in a
        leak proof manner by zapping the GCHeap; if we use system memory we can't do
        that (short of using a malloc zone). Also, it's about getting the right
        profiler data: we want the WeakRef hashtable to show up in the memory profile
        results.
    */
    class GCHashtableAllocHandler_VMPI
    {
    public:
        static void* alloc(size_t size, bool canFail);
        static void free(void* ptr);
    };

    class GCHashtableAllocHandler_new
    {
    public:
        static void* alloc(size_t size, bool canFail);
        static void free(void* ptr);
    };

    // --------------------------------

    class GCHashtableKeyHandler
    {
    public:
        REALLY_INLINE static uint32_t hash(const void* k)
        {
            return uint32_t((uintptr_t(k)>>3) ^ (uintptr_t(k)<<29)); // move the low 3 bits higher up since they're often 0
        }

        REALLY_INLINE static bool equal(const void* k1, const void* k2)
        {
            return k1 == k2;
        }
    };

    typedef GCHashtableBase<GCHashtableKeyHandler, GCHashtableAllocHandler_new> GCHashtable;
    typedef GCHashtableBase<GCHashtableKeyHandler, GCHashtableAllocHandler_VMPI> GCHashtable_VMPI;

    // --------------------------------

    class GCStringHashtableKeyHandler
    {
    public:
        REALLY_INLINE static uint32_t hash(const void* k)
        {
            uint32_t hash = 0;
            const char* s = (const char*)k;
            while (*s++)
            {
                hash = (hash >> 28) ^ (hash << 4) ^ ((uintptr_t)*s << ((uintptr_t)s & 0x3));
            }
            return hash;
        }

        REALLY_INLINE static bool equal(const void* k1, const void* k2)
        {
            if (k1 == k2)
                return true;
            if (k1 && k2)
                return VMPI_strcmp((const char*)k1, (const char*)k2) == 0;
            return false;
        }
    };

    typedef GCHashtableBase<GCStringHashtableKeyHandler, GCHashtableAllocHandler_new> GCStringHashtable;
    typedef GCHashtableBase<GCStringHashtableKeyHandler, GCHashtableAllocHandler_VMPI> GCStringHashtable_VMPI;
}

#endif

