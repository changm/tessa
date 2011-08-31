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


#include "MMgc.h"

namespace MMgc
{
    // kSizeClassIndex[] is an array that lets us quickly determine the allocator
    // to use for a given size, without division.  A given allocation is rounded
    // up to the nearest multiple of 8, then downshifted 3 bits, and the index
    // tells us which allocator to use.  (A special case is made for <= 4 bytes on
    // 32-bit systems in FindAllocatorForSize to keep the table small.)

    // Code to generate the table:
    //
    //      const int kMaxSizeClassIndex = (kLargestAlloc>>3)+1;
    //      uint8_t kSizeClassIndex[kMaxSizeClassIndex];
    //      printf("static const unsigned kMaxSizeClassIndex = %d;\n",kMaxSizeClassIndex);
    //      printf("static const uint8_t kSizeClassIndex[kMaxSizeClassIndex] = {\n");
    //      for (int size = 0; size <= kLargestAlloc; size += 8)
    //      {
    //          int i = 0;
    //          while (kSizeClasses[i] < size)
    //          {
    //              ++i;
    //              AvmAssert(i < kNumSizeClasses);
    //          }
    //          AvmAssert((size>>3) < kMaxSizeClassIndex);
    //          kSizeClassIndex[(size>>3)] = i;
    //          if (size > 0) printf(",");
    //          if (size % (16*8) == 0) printf("\n");
    //          printf(" %d",i);
    //      }
    //      printf("};\n");

#ifdef MMGC_64BIT
    const int16_t FixedMalloc::kSizeClasses[kNumSizeClasses] = {
        8, 16, 24, 32, 40, 48, 56, 64, 72, 80, //0-9
        88, 96, 104, 112, 120, 128, 136, 144, 152, 160, //10-19
        168, 176, 192, 208, 224, 232, 248, 264, 288, 304, //20-29
        336, 360, 400, 448, 504, 576, 672, 800, 1008, 1344, //30-39
        2016, //40
    };

    /*static*/ const uint8_t FixedMalloc::kSizeClassIndex[kMaxSizeClassIndex] = {
         0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26,
         27, 27, 28, 28, 28, 29, 29, 30, 30, 30, 30, 31, 31, 31, 32, 32,
         32, 32, 32, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34,
         35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36,
         36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
         37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
         38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39,
         39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
         39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
         39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
         40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40
    };

#else
    const int16_t FixedMalloc::kSizeClasses[kNumSizeClasses] = {
        4, 8, 16, 24, 32, 40, 48, 56, 64, 72, //0-9
        80, 88, 96, 104, 112, 120, 128, 144, 160, 176, //10-19
        184, 192, 200, 208, 224, 232, 248, 264, 288, 312, //20-29
        336, 368, 400, 448, 504, 576, 672, 808, 1016, 1352, //30-39
        2032, //40
    };

    /*static*/ const uint8_t FixedMalloc::kSizeClassIndex[kMaxSizeClassIndex] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 17, 18, 18, 19, 19, 20, 21, 22, 23, 24, 24, 25, 26, 26,
        27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 31, 32,
        32, 32, 32, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34,
        35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36,
        36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
        37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
        38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
        39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
        39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
        39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40
    };

#endif

#if defined DEBUG
    // For debugging we track live large objects in a list.  If there are a lot
    // of large objects then a list may slow down debug builds too much; in that
    // case we can move to a tree or similar structure.  (It's useful to avoid using
    // large objects in this data structure.)
    struct FixedMalloc::LargeObject
    {
        const void *item;       // Start of a block
        LargeObject* next;      // Next object
    };
#endif
    
    /*static*/
    FixedMalloc *FixedMalloc::instance;

    void FixedMalloc::InitInstance(GCHeap* heap)
    {
        m_heap = heap;
        numLargeBlocks = 0;
        VMPI_lockInit(&m_largeAllocInfoLock);
    #ifdef MMGC_MEMORY_PROFILER
        totalAskSizeLargeAllocs = 0;
    #endif
    #if defined DEBUG && !defined AVMPLUS_SAMPLER
        largeObjects = NULL;
        VMPI_lockInit(&m_largeObjectLock);
    #endif

        for (int i=0; i<kNumSizeClasses; i++)
            m_allocs[i].Init((uint32_t)kSizeClasses[i], heap);

        FixedMalloc::instance = this;
    }

    void FixedMalloc::DestroyInstance()
    {
        for (int i=0; i<kNumSizeClasses; i++)
            m_allocs[i].Destroy();

        VMPI_lockDestroy(&m_largeAllocInfoLock);
    #if defined DEBUG && !defined AVMPLUS_SAMPLER
        VMPI_lockDestroy(&m_largeObjectLock);
    #endif

        FixedMalloc::instance = NULL;
    }

    void* FASTCALL FixedMalloc::OutOfLineAlloc(size_t size, FixedMallocOpts flags)
    {
        return Alloc(size, flags);
    }

    void FASTCALL FixedMalloc::OutOfLineFree(void* p)
    {
        Free(p);
    }

    void FixedMalloc::GetUsageInfo(size_t& totalAskSize, size_t& totalAllocated)
    {
        totalAskSize = 0;
        totalAllocated = 0;
        for (int i=0; i<kNumSizeClasses; i++) {
            size_t allocated = 0;
            size_t ask = 0;
            m_allocs[i].GetUsageInfo(ask, allocated);
            totalAskSize += ask;
            totalAllocated += allocated;
        }

#ifdef MMGC_MEMORY_PROFILER
        {
            MMGC_LOCK(m_largeAllocInfoLock);
            totalAskSize += totalAskSizeLargeAllocs;
        }
#endif

        // Not entirely accurate, assumes large allocations using all of
        // the last block (large ask size not stored).
        totalAllocated += (GetNumLargeBlocks() * GCHeap::kBlockSize);
    }

    void *FixedMalloc::LargeAlloc(size_t size, FixedMallocOpts flags)
    {
        GCHeap::CheckForAllocSizeOverflow(size, GCHeap::kBlockSize+DebugSize());

        size += DebugSize();

        int blocksNeeded = (int)GCHeap::SizeToBlocks(size);
        uint32_t gcheap_flags = GCHeap::kExpand;

        if((flags & kCanFail) != 0)
            gcheap_flags |= GCHeap::kCanFail;
        if((flags & kZero) != 0)
            gcheap_flags |= GCHeap::kZero;

        void *item = m_heap->Alloc(blocksNeeded, gcheap_flags);
        if(item)
        {

            item = GetUserPointer(item);
#ifdef MMGC_HOOKS
            if(m_heap->HooksEnabled())
                m_heap->AllocHook(item, size - DebugSize(), Size(item));
#endif // MMGC_HOOKS

            UpdateLargeAllocStats(item, blocksNeeded);

#ifdef DEBUG
            // Fresh memory poisoning
            if((flags & kZero) == 0)
                memset(item, uint8_t(GCHeap::FXFreshPoison), size - DebugSize());

#ifndef AVMPLUS_SAMPLER
            // Enregister the large object
            AddToLargeObjectTracker(item);
#endif
#endif // DEBUG
        }
        return item;
    }

    void FixedMalloc::LargeFree(void *item)
    {
#if defined DEBUG && !defined AVMPLUS_SAMPLER
        RemoveFromLargeObjectTracker(item);
#endif
        UpdateLargeFreeStats(item, GCHeap::SizeToBlocks(LargeSize(item)));

#ifdef MMGC_HOOKS
        if(m_heap->HooksEnabled())
        {
            m_heap->FinalizeHook(item, Size(item));
            m_heap->FreeHook(item, Size(item), uint8_t(GCHeap::FXFreedPoison));
        }
#endif
        m_heap->FreeNoProfile(GetRealPointer(item));
    }

    size_t FixedMalloc::LargeSize(const void *item)
    {
        return m_heap->Size(GetRealPointer(item)) * GCHeap::kBlockSize;
    }

    void *FixedMalloc::Calloc(size_t count, size_t elsize, FixedMallocOpts opts)
    {
        return Alloc(GCHeap::CheckForCallocSizeOverflow(count, elsize), opts);
    }

    size_t FixedMalloc::GetTotalSize()
    {
        size_t total = GetNumLargeBlocks();
        for (int i=0; i<kNumSizeClasses; i++)
            total += m_allocs[i].GetNumBlocks();
        return total;
    }

#ifdef MMGC_MEMORY_PROFILER
    void FixedMalloc::DumpMemoryInfo()
    {
        size_t inUse, ask;
        GetUsageInfo(ask, inUse);
        GCLog("[mem] FixedMalloc total %d pages inuse %d bytes ask %d bytes\n", GetTotalSize(), inUse, ask);
        for (int i=0; i<kNumSizeClasses; i++) {
            m_allocs[i].GetUsageInfo(ask, inUse);
            if( m_allocs[i].GetNumBlocks() > 0)
                GCLog("[mem] FixedMalloc[%d] total %d pages inuse %d bytes ask %d bytes\n", kSizeClasses[i], m_allocs[i].GetNumBlocks(), inUse, ask);
        }
        GCLog("[mem] FixedMalloc[large] total %d pages\n", GetNumLargeBlocks());
    }
#endif

#ifdef DEBUG
    // If EnsureFixedMallocMemory returns and fixed-memory checking has not
    // been disabled then item was definitely allocated by an allocator owned
    // by this FixedMalloc.  Large objects must be handled one of two ways
    // depending on whether the sampler is operating: if it is, we can't
    // allocate storage to track large objects (see bugzilla 533954),
    // so fall back on a less accurate method.

    void FixedMalloc::EnsureFixedMallocMemory(const void* item)
    {
        // For a discussion of this flag, see bugzilla 564878.
        if (!m_heap->config.checkFixedMemory())
            return;
        
        for (int i=0; i<kNumSizeClasses; i++)
            if (m_allocs[i].QueryOwnsObject(item))
                return;

#ifdef AVMPLUS_SAMPLER
        if (m_heap->SafeSize(GetRealPointer(item)) != (size_t)-1)
            return;
#else
        {
            MMGC_LOCK(m_largeObjectLock);
            for ( LargeObject* lo=largeObjects; lo != NULL ; lo=lo->next)
                if (lo->item == item)
                    return;
        }
#endif

        GCAssertMsg(false, "Trying to delete an object with FixedMalloc::Free that was not allocated with FixedMalloc::Alloc");
    }

#ifndef AVMPLUS_SAMPLER
    void FixedMalloc::AddToLargeObjectTracker(const void* item)
    {
        if (!m_heap->config.checkFixedMemory())
            return;
        
        LargeObject* lo = (LargeObject*)Alloc(sizeof(LargeObject));
        lo->item = item;
        MMGC_LOCK(m_largeObjectLock);
        lo->next = largeObjects;
        largeObjects = lo;
    }

    void FixedMalloc::RemoveFromLargeObjectTracker(const void* item)
    {
        if (!m_heap->config.checkFixedMemory())
            return;
        
        void *loToFree=NULL;
        {
            MMGC_LOCK(m_largeObjectLock);
            LargeObject *lo, *prev;
            for ( prev=NULL, lo=largeObjects ; lo != NULL ; prev=lo, lo=lo->next ) {
                if (lo->item == item) {
                    if (prev != NULL)
                        prev->next = lo->next;
                    else
                        largeObjects = lo->next;
                    loToFree = lo;
                    break;
                }
            }
        }
        if(loToFree)
            Free(loToFree);
    }
#endif // !AVMPLUS_SAMPLER
#endif // DEBUG
}

