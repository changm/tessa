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

#ifndef __GCHashtable_inlines_
#define __GCHashtable_inlines_

namespace MMgc
{
    template <class KEYHANDLER, class ALLOCHANDLER>
    GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::GCHashtableBase(uint32_t capacity) :
    table(NULL),
    tableSize(capacity*2),
    numValues(0),
    numDeleted(0)
    {
        if (tableSize > 0)
        {
            grow(false);
        }
        else
        {
            // appear as full table so grow, numValues will go to zero on rehash
            tableSize = 4;
            numValues = 4;
            table = EMPTY;
        }
    }

    template <class KEYHANDLER, class ALLOCHANDLER>
    GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::~GCHashtableBase()
    {
        if (table && table != EMPTY)
            ALLOCHANDLER::free(table);
        table = NULL;
        tableSize = 0;
        numValues = 0;
        numDeleted = 0;
    }

    template <class KEYHANDLER, class ALLOCHANDLER>
    void GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::clear()
    {
        if (table && table != EMPTY)
            ALLOCHANDLER::free(table);
        table = EMPTY;
        tableSize = 4;
        numValues = 4;
        numDeleted = 0;
    }

    template <class KEYHANDLER, class ALLOCHANDLER>
    uint32_t GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::find(const void* key, const void** table, uint32_t tableSize)
    {
        GCAssert(key != DELETED);

        // this is a quadratic probe but we only hit even numbered slots since those hold keys.
        uint32_t n = 7 << 1;
        // bitmask is defined in Symbian OS headers. changing to bitMask
        uint32_t const bitMask = (tableSize - 1) & ~0x1;
        uint32_t i = KEYHANDLER::hash(key) & bitMask;
        const void* k;
        while ((k = table[i]) != NULL && !KEYHANDLER::equal(k, key))
        {
            i = (i + (n += 2)) & bitMask;       // quadratic probe
        }
        GCAssert(i <= ((tableSize-1)&~0x1));
        return i;
    }

    template <class KEYHANDLER, class ALLOCHANDLER>
    void GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::put(const void* key, const void* value)
    {
        GCAssert(table != NULL);

        // this is basically an inlined version of find() with one minor difference:
        // it notices if there are DELETED slots along the probe, and if there is one,
        // we recycle it rather than adding to the end of the probe chain. this allows us
        // to recycle deleted slots MUCH more quickly (without having to wait for a full rehash)
        // and can dramatically reduce the average probe depth if there are a lot of
        // insertions and removals.
        const uint32_t NO_DELINDEX = 0xffffffff;
        uint32_t delindex = NO_DELINDEX;
        uint32_t n = 7 << 1;
        uint32_t const bitMask = (tableSize - 1) & ~0x1;
        uint32_t i = KEYHANDLER::hash(key) & bitMask;
        const void* k;
        while ((k = table[i]) != NULL && !KEYHANDLER::equal(k, key))
        {
            // note that we can't just stop at the first DELETED value we find --
            // we might have a matching value later in the chain. We choose
            // the first such entry so that subsequent searches are as short as possible
            // (choosing any other entry would be fine, just suboptimal)
            if (k == DELETED && delindex == NO_DELINDEX) delindex = i;
            i = (i + (n += 2)) & bitMask;       // quadratic probe
        }
        GCAssert(k == NULL || KEYHANDLER::equal(k, key));
        if (k == NULL)
        {
            if (delindex != NO_DELINDEX)
            {
                // there's a deleted entry we can replace
                i = delindex;
                numDeleted--;
                // note that we don't increment numValues here!
            }
            else
            {
                // .75 load factor, note we don't take numDeleted into account
                // numValues includes numDeleted
                if (numValues * 8 >= tableSize * 3)
                {
                    grow(false);
                    // grow rehashes, so no DELETED items, thus normal find() is OK
                    GCAssert(numDeleted == 0);
                    i = find(key, table, tableSize);
                    GCAssert(!table[i]);
                }
                numValues++;
            }
            table[i] = key;
        }
        table[i+1] = value;
    }

    template <class KEYHANDLER, class ALLOCHANDLER>
    const void* GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::remove(const void* key)
    {
        const void* ret = NULL;
        uint32_t i = find(key, table, tableSize);
        if (table[i] == key)
        {
            table[i] = DELETED;
            ret = table[i+1];
            table[i+1] = NULL;
            numDeleted++;
            // this helps a bit on pathologic memory profiler use case, needs more investigation
            // 20% deleted == rehash
            if ((numValues - numDeleted) * 10 < tableSize)
            {
                grow(true);
            }
        }
        return ret;
    }

    template <class KEYHANDLER, class ALLOCHANDLER>
    void GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::grow(bool isRemoval)
    {
        if (isRemoval)
        {
            // Bugzilla 553679: Skip table resizing in a removal situation if the heap
            // is in an abort state, we don't want to allocate during abort if we can
            // avoid it, and 'grow' is called when the collector clears its weak references.
            if (GCHeap::GetGCHeap()->GetStatus() == kMemAbort)
                return;
        }

        uint32_t newTableSize = tableSize;

        uint32_t occupiedSlots = numValues - numDeleted;
        GCAssert(numValues >= numDeleted);

        // grow or shrink as appropriate:
        // if we're greater than 50% full grow
        // if we're less than 10% shrink
        // else stay the same
        if (4*occupiedSlots > tableSize)
            newTableSize <<= 1;
        else if (10*occupiedSlots < tableSize && tableSize > kDefaultSize && table)
            newTableSize >>= 1;

        const void** newTable;
        newTable = (const void**)ALLOCHANDLER::alloc(newTableSize*sizeof(const void*), isRemoval);
        if (!newTable)
            return;

        VMPI_memset(newTable, 0, newTableSize*sizeof(void*));

        numValues = 0;
        numDeleted = 0;

        if (table)
        {
            for (uint32_t i=0, n=tableSize; i < n; i += 2)
            {
                const void* oldKey;
                if ((oldKey=table[i]) != NULL)
                {
                    // inlined & simplified version of put()
                    if (oldKey != DELETED) {
                        uint32_t j = find(oldKey, newTable, newTableSize);
                        newTable[j] = oldKey;
                        newTable[j+1] = table[i+1];
                        numValues++;
                    }
                }
            }
        }

        if (table && table != EMPTY)
        {
            ALLOCHANDLER::free(table);
        }
        table = newTable;
        tableSize = newTableSize;
        GCAssert(table != NULL);
    }

    template <class KEYHANDLER, class ALLOCHANDLER>
    int32_t GCHashtableBase<KEYHANDLER,ALLOCHANDLER>::nextIndex(int32_t index)
    {
        uint32_t i = index<<1;
        while(i < tableSize)
        {
            if (table[i] > DELETED)
                return (i>>1)+1;
            i += 2;
        }
        return 0;
    }

}

#endif

