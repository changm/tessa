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

#ifndef __FixedAlloc__
#define __FixedAlloc__

namespace MMgc
{
    /**
     * FixedAlloc is a fast, non-thread-safe, fixed-size memory allocator for
     * manually freed objects.
     *
     * Memory is allocated from the system in individual blocks via the GCHeap
     * class and then carved into smaller objects.  For larger objects we have
     * the FixedAllocLarge allocator.
     */
    class FixedAlloc
    {
        friend class FixedMalloc;
        friend class FixedAllocSafe;
        friend class FastAllocator;
        friend class GC;

    public:
        /**
         * Obtain the allocator used to allocate a particular FixedAlloc item.
         *
         * @param item  An object allocated with FixedAlloc.
         *
         * @return   The allocator used to allocate the item
         */
        static FixedAlloc *GetFixedAlloc(void *item);

        /**
         * Free an object allocated with a FixedAlloc instance.  The object is returned
         * to the FixedAlloc that provided it.
         *
         * @param item  The object to free.
         */
        static void Free(void *item);
        
    public:
        /**
         * Initialize the allocator.
         *
         * @param itemSize          The size of items managed by this allocator.
         * @param heap              The block manager from which we obtain memory.
         * @param isFixedAllocSafe  If true, this allocator's concrete instance
         *                          is FixedAllocSafe, and certain unlocking operations
         *                          will be performed in out-of-memory situations.
         */
        FixedAlloc(uint32_t itemSize, GCHeap* heap, bool isFixedAllocSafe=false);

        /**
         * Destroy the allocator, returning all blocks to the block manager.
         * Check for and report storage leaks in MMGC_MEMORY_INFO mode.
         */
        ~FixedAlloc();

        /**
         * Allocate one object from this allocator.
         *
         * @param size   The size of the object; this is used for debugging only, as the
         *               size allocated is fixed by the allocator.
         * @param flags  A bit vector of allocation options.
         *
         * @return  A pointer to the object.  The pointer may be NULL only if kCanFail is
         *          part of flags.  The memory is zeroed only if kZero is part of flags.
         */
        void* Alloc(size_t size, FixedMallocOpts flags=kNone);

        /**
         * @return the number bytes currently allocated by this FixedAlloc
         */
        size_t GetBytesInUse() const;

        /**
         * Obtain current (not running total) allocation information for this FixedAlloc.
         *
         * @param totalAsk        (out) The total number of bytes requested
         * @param totalAllocated  (out) The number of bytes actually allocated
         */
        void GetUsageInfo(size_t& totalAsk, size_t& totalAllocated) const;

        /**
         * @return the size of objects managed by this allocator
         */
        size_t GetItemSize() const;

        /**
         * @return the number of objects (free and allocated) held by this allocator.
         */
        size_t GetMaxAlloc() const;

        /**
         * @return the number of blocks held by this allocator.
         */
        size_t GetNumBlocks() const;

    public:
        // This structure is really private to FixedAlloc, but the current Symbian compiler
        // requires this structure to be public (now == May, 2010).
        struct FixedBlock
        {
            void*  firstFree;       // First object on the block's free list
            void*  nextItem;        // First object free at the end of the block
            FixedBlock* next;       // Next block on the list of blocks (m_firstBlock list in the allocator)
            FixedBlock* prev;       // Previous block on the list of blocks
            uint16_t numAlloc;      // Number of items allocated from the block
            uint16_t size;          // Size of objects in the block
            FixedBlock *nextFree;   // Next block on the list of blocks with free items (m_firstFree list in the allocator)
            FixedBlock *prevFree;   // Previous block on the list of blocks with free items
            FixedAlloc *alloc;      // The allocator that owns this block
            char   items[1];        // Memory for objects starts here
        };

    private:
        GCHeap *m_heap;             // The heap from which we obtain memory
        uint32_t m_itemsPerBlock;   // Number of items that fit in a block
        uint32_t m_itemSize;        // Size of each individual item
        
        FixedBlock* m_firstBlock;   // First block on list of free blocks
        FixedBlock* m_lastBlock;    // Last block on list of free blocks
        FixedBlock* m_firstFree;    // The lowest priority block that has free items

        size_t    m_numBlocks;      // Number of blocks owned by this allocator
#ifdef MMGC_MEMORY_PROFILER
        size_t m_totalAskSize;      // Current total amount of memory requested from this allocator
#endif
        bool const m_isFixedAllocSafe;  // true if this allocator's true type is FixedAllocSafe

    private:
        // Return the FixedBlock for any item allocated by FixedAlloc.
        static FixedBlock *GetFixedBlock(const void *item);
        
        // Return the size of any item allocated by FixedAlloc.
        static size_t Size(const void *item);
        
#ifdef MMGC_HEAP_GRAPH
        // Find the beginning of an object into which 'addr' points.
        //
        // Addr /must/ point into a block currently managed by a FixedAlloc
        // allocator, but it could point into freed storage, before the first
        // object, or after the last object in the block.
        //
        // Return an object pointer if the pointer is into some object in the
        // block (could be a free object), NULL otherwise.
        //
        // This API is used by the back pointer facility to find the beginning
        // of a GCRoot.
        static const void *FindBeginning(const void *addr);
#endif

#ifdef MMGC_MEMORY_INFO
        // Go through every item on the free list and make sure it wasn't written to
        // after it was poisoned as it was being freed.
        static void VerifyFreeBlockIntegrity(const void* item, uint32_t size);
#endif

        // Implement the 'Free' operation on 'item', excluding the calling of hooks
        static void InlineFreeSansHook(void *item MMGC_MEMORY_PROFILER_ARG(size_t askSize));
        
#ifdef MMGC_HOOKS
        // Call the freeing hooks on 'item'
        static void InlineFreeHook(void *item MMGC_MEMORY_PROFILER_ARG(size_t& askSize));
#endif
        
    private:
        // Initialize the allocator with default values.  Used indirectly through
        // FixedAllocSafe by FixedMalloc, which calls FixedAlloc::Init directly.
        FixedAlloc();

        // Initialize the allocator.  'itemSize' is the size of objects managed by
        // the allocator; 'heap' is the heap from which we obtain memory.
        void Init(uint32_t itemSize, GCHeap *heap);

        // Destroy the allocator and free all resources.
        void Destroy();
        
        // Create a new chunk for the allocator and make m_firstFree point to it.
        // If canFail is true then the chunk may not be allocated if the system is
        // low on memory; the caller can detect this by m_firstFree being the same
        // before and after the call.
        void CreateChunk(bool canFail);
        
        // Free the chunk 'b', returning it to the GCHeap.  Unlinks it from the
        // block lists if necessary. 
        //
        // Note, if the true type of the allocator is FixedAllocSafe then the
        // lock is released during the call into GCHeap.
        void FreeChunk(FixedBlock* b);
        
        // Allocate an object with the given flags.  Expands to a fair amount
        // of code, meant to be inlined into a few dedicated methods only.
        void* AllocInline(size_t size, FixedMallocOpts flags=kNone);
        
        // Implement the 'Alloc' operation, excluding the calling of hooks
        void* InlineAllocSansHook(size_t size, FixedMallocOpts flags=kNone);
        
#ifdef MMGC_HOOKS
        // Call the allocation hooks on 'item'
        void InlineAllocHook(size_t size, void *item);
#endif

#ifdef DEBUG
        // Query whether 'addr' points into memory controlled by this allocator.
        // The object identified could be free.  'addr' can be any address, it
        // does not need to point into a FixedBlock, for example.
        bool QueryOwnsObject(const void* addr);
#endif
        
        // Return true iff there are no free items in the block 'b'.
        bool IsFull(FixedBlock *b) const;

        // Return true if 'item' is on the free list of the block 'b'.
        bool IsOnFreelist(FixedBlock *b, void *item);
        
        // Return true if 'addr' points into the object region of b and is
        // not free (as determined by IsOnFreelist).
        bool IsInUse(FixedBlock *b, void *addr);
    };

    /**
     * FixedAlloc is a fast, thread-safe, fixed-size memory allocator for manually
     * freed objects.
     */
    class FixedAllocSafe : public FixedAlloc
    {
        friend class FixedAlloc;
        friend class FixedMalloc;

    private:
        // Obtain the allocator for 'item' as a FixedAllocSafe instance.
        static FixedAllocSafe *GetFixedAllocSafe(void *item);
        
    private:       
        vmpi_spin_lock_t m_spinlock;    // Protects the properties of FixedAlloc

    private: // I only exist for the sake of my friends

        // Initialize the allocator.  'itemSize' is the size of objects managed by
        // the allocator; 'heap' is the heap from which we obtain memory.
        FixedAllocSafe(int itemSize, GCHeap* heap);

        // Initialize the allocator with default values.  Used by FixedMalloc,
        // which calls FixedAlloc::Init directly.
        FixedAllocSafe();
        
        // Destroy the allocator and free all resources.
        ~FixedAllocSafe();

        // Destroy the allocator and free all resources.  Used by FixedMalloc.
        void Destroy();

        // Allocate an object with the given flags.  Holds the lock.
        void* Alloc(size_t size, FixedMallocOpts flags=kNone);

        // Free an object obtained from this allocator.  Holds the lock.
        void Free(void *item);

#ifdef DEBUG
        // Query whether 'item' was allocated from this allocator (see the
        // base class documentation for details).  Holds the lock.
        bool QueryOwnsObject(const void* item);
#endif
    };

    /**
     * FastAllocator is a fast, non-thread-safe, fixed-size memory allocator for
     * manually freed objects using 'new' and 'delete' operators for object management.
     */
    class FastAllocator
    {
    public:
        static void *operator new(size_t size, FixedAlloc *alloc);
        static void operator delete (void *item);
        static void *operator new[](size_t size);
        static void operator delete [](void *item);
    };
}

#endif /* __FixedAlloc__ */
