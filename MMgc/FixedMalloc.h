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

#ifndef __Malloc__
#define __Malloc__

namespace MMgc
{
    /**
     * A general purpose memory allocator.
     *
     * FixedMalloc is a singleton, obtainable by calling FixedMalloc::GetFixedMalloc.
     * The "owner" of the FixedMalloc is GCHeap, the block manager.
     *
     * FixedMalloc uses size classes; each size class is handled by a FixedAllocSafe
     * instance.  Large objects are handled specially.  All objects are headerless.
     */
    class FixedMalloc
    {
        friend class GCHeap;

    public:
        /**
         * Obtain a FixedMalloc instance.
         *
         * @return the FixedMalloc singleton.
         */
        static FixedMalloc *GetFixedMalloc();

        /**
         * Obtain a FixedMalloc instance.
         *
         * @return the FixedMalloc singleton.
         *
         * @note Backward compatible name for GetFixedMalloc, not used by Tamarin.
         */
        static FixedMalloc *GetInstance();

        /**
         * Allocate one object from this allocator.
         *
         * @param size   The size of the object.
         * @param flags  A bit vector of allocation options.
         *
         * @return  A pointer to the object.  The pointer may be NULL only if kCanFail is
         *          part of flags.  The memory is zeroed only if kZero is part of flags.
         */
        void* Alloc(size_t size, FixedMallocOpts flags=kNone);

        /**
         * Allocate one object from this allocator, may return NULL.
         *
         * @param size   The size of the object.
         *
         * @return  A pointer to the object.  The pointer may be NULL.
         *          The memory is /not/ necessarily zeroed; for zeroed memory, use Alloc().
         *
         * @note Exactly like Alloc with flags=kCanFail.
         */
        void *PleaseAlloc(size_t size);

        /**
         * Allocate one object from this allocator.
         *
         * @param size   The size of the object.
         * @param flags  A bit vector of allocation options.
         *
         * @return  A pointer to the object.  The pointer may be NULL only if kCanFail is
         *          part of flags.  The memory is zeroed only if kZero is part of flags.
         *
         * @note Exactly like Alloc, but guaranteed not to be inlined - used by ::new etc.
         */
        void* FASTCALL OutOfLineAlloc(size_t size, FixedMallocOpts flags=kNone);

        /**
         * Allocate space for an array of objects from this allocator.
         *
         * @param count  The number of objects
         * @param size   The size of each part object
         * @param flags  A bit vector of allocation options
         *
         * @return  A pointer to the aggregate object.  The pointer may be NULL only if
         *          kCanFail is part of flags.  The memory is zeroed only if kZero is part
         *          of flags.
         *
         * @note  Unlike 'calloc' in the C library, this does /not/ zero the memory
         *        unless kZero is passed in flags.  The name 'Calloc' comes from the
         *        shape of the API.
         */
        void *Calloc(size_t count, size_t size, FixedMallocOpts flags=kNone);

        /**
         * Free an object allocated through FixedMalloc.
         *
         * @param item  The object to free.
         */
        void Free(void *item);

        /**
         * Free an object allocated through FixedMalloc.
         *
         * @param item  The object to free.
         *
         * @note  Exactly like Free, but guaranteed not to be inlined - used by ::delete etc.
         */
        void FASTCALL OutOfLineFree(void* p);

        /**
         * Obtain the size of an object allocated through FixedMalloc.
         *
         * @param  item  An object reference.
         *
         * @return the allocated size of 'item'
         */
        size_t Size(const void *item);

        /**
         * Obtain FixedMalloc's heap usage.
         *
         * @return The total number of /blocks/ managed by FixedMalloc, where the
         *         block size is given by GCHeap::kBlockSize.
         */
        size_t GetTotalSize();

        /**
         * Obtain current (not running total) allocation information for FixedMalloc.
         *
         * @param totalAskSize    (out) The total number of bytes requested
         * @param totalAllocated  (out) The number of bytes actually allocated
         */
        void GetUsageInfo(size_t& totalAskSize, size_t& totalAllocated);

        /**
         * Obtain current allocation information for FixedMalloc.
         *
         * @return the number bytes currently allocated by FixedMalloc.
         *
         * @note  The returned value is the totalAllocated value returned from GetUsageInfo.
         */
        size_t GetBytesInUse();

#ifdef MMGC_MEMORY_PROFILER
        /**
         * Print semi-structured human-readable data about FixedMalloc memory usage
         * on the VMPI_log channel.
         */
        void DumpMemoryInfo();
#endif

    private:
#ifdef DEBUG
        // Data type used for tracking live large objects, used by EnsureFixedMallocMemory.
        struct LargeObject;
#endif

        // Return true if item is a large-object item allocated through FixedMalloc.
        static bool IsLargeAlloc(const void *item);
        
        // Initialize FixedMalloc.  Must be called from GCHeap during GCHeap setup.
        void InitInstance(GCHeap *heap);

        // Destroy FixedMalloc and free all resources.
        void DestroyInstance();

        // Return the total number of blocks allocated by FixedMalloc for large-object allocations.
        size_t GetNumLargeBlocks();

        // Record that 'blocksAllocated' blocks are about to be or have been allocated 
        // in support of large-object allocation.
        void UpdateLargeAllocStats(void* item, size_t blocksAllocated);
        
        // Record that 'blocksFreed' blocks are about to be or have been freed 
        // in support of large-object freeing.
        void UpdateLargeFreeStats(void* item, size_t blocksFreed);

#ifdef DEBUG
        // Check that item was allocated by an allocator owned by this FixedMalloc,
        // otherwise trigger an assertion failure.
        void EnsureFixedMallocMemory(const void* item);

#ifndef AVMPLUS_SAMPLER
        // Track large object 'item', which is newly allocated.
        void AddToLargeObjectTracker(const void* item);

        // Untrack large object 'item', which is about to be freed.
        void RemoveFromLargeObjectTracker(const void* item);
#endif // !AVMPLUS_SAMPLER
#endif // DEBUG

        // Return a thread-safe allocator for objects of the given size.
        FixedAllocSafe* FindAllocatorForSize(size_t size);

        // Return an object of at least the requested size, allocated with the given
        // flags.  The object's real size will be an integral number of blocks.
        void *LargeAlloc(size_t size, FixedMallocOpts flags=kNone);

        // Free the item returned from LargeAlloc.
        void LargeFree(void *item);

        // Return the allocated size (in bytes) of 'item', which must have been returned
        // from LargeAlloc.
        size_t LargeSize(const void *item);
        
    private:
        static FixedMalloc *instance;   // The singleton FixedMalloc

#ifdef MMGC_64BIT
        const static int kLargestAlloc = 2016;  // The largest small-object allocation
#else
        const static int kLargestAlloc = 2032;  // The largest small-object allocation
#endif
        const static int kNumSizeClasses = 41;  // The number of small-object size classes
        
        // A table whose nth entry is the maximum size accomodated by the
        // allocator in the nth entry of the m_allocs table.
        const static int16_t kSizeClasses[kNumSizeClasses];
        
        // The number of entries in the table mapping a request not greater than
        // kLargestAlloc to the appropriate index in m_allocs.
        const static unsigned kMaxSizeClassIndex = (kLargestAlloc>>3)+1;
        
        // A table mapping a request not greater than kLargestAlloc to the appropriate
        // index in m_allocs.  The mapping is complicated and explained in comments
        // in FixedMalloc.cpp, also see the implementation of FindAllocatorForSize.
        const static uint8_t kSizeClassIndex[kMaxSizeClassIndex];

    private:
        GCHeap *m_heap;                             // The heap from which we allocate, set in InitInstance
        FixedAllocSafe m_allocs[kNumSizeClasses];   // The array of size-segregated allocators, set in InitInstance

        vmpi_spin_lock_t m_largeAllocInfoLock;  // Protects numLargeBlocks and totalAskSizeLargeAllocs

        size_t numLargeBlocks;              // Number of large-object blocks owned by this FixedMalloc
#ifdef MMGC_MEMORY_PROFILER
        size_t totalAskSizeLargeAllocs;     // The current number of bytes requested for large objects
#endif
#if defined DEBUG
        vmpi_spin_lock_t m_largeObjectLock; // Protects largeObjects
        LargeObject      *largeObjects;     // Data structure of live large objects, initially NULL
#endif
    };
}
#endif /* __Malloc__ */
