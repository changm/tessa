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

#ifndef __avmplus_Hashtable__
#define __avmplus_Hashtable__


namespace avmplus
{
    /**
     * common base class for hashtable-like objects.
     */
    // NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE
    //
    // InlineHashtable is designed to never have its ctors or dtors called directly; it can't be allocated
    // either on the stack or the heap. For a standalong heap allocation, use HeapHashtable instead.
    //
    // The reason for this oddity is that InlineHashtable can be grafted "inline" to a ScriptObject, and
    // will be manually initialized and destroyed (but not via its ctor/dtor). We also don't want
    // to descend from any class that might force us to have a vtable (eg by way of a virtual dtor) because
    // we neither need one nor want to account for the extra pointer-sized space in every instance of ScriptObject.
    // (It's not large, but it adds up...)
    //
    // NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE
    class InlineHashtable
    {
        friend class HeapHashtable;
        friend class HeapHashtableRC;
        friend class WeakKeyHashtable;
        friend class WeakValueHashtable;

    public:
        /** kDefaultCapacity must be a power of 2 */
        const static int kDefaultCapacity = 2;

        void initialize(MMgc::GC* gc, int capacity = kDefaultCapacity);
        void initializeWithDontEnumSupport(MMgc::GC* gc, int capacity = kDefaultCapacity);
        void destroy();
        void reset();

        uint32_t getSize() const;
        bool needsInitialize() const;

        bool getAtomPropertyIsEnumerable(Atom name) const;
        void setAtomPropertyIsEnumerable(Atom name, bool enumerable);

#ifdef DEBUGGER
        /**
         * Basically the same as AvmPlusScriptableObject::bytesUsed().
         */
        uint64_t bytesUsed() const;
#endif

        Atom get(Atom name) const;
        Atom remove(Atom name);

        // Similar to get(), but keys that are missing return EMPTY
        // rather than undefinedAtom. This is due to the regettable fact that undefinedAtom
        // is a legal value for a hashtable value (while EMPTY is not). (Note that we
        // can't actually use "EMPTY" here due to compiler issues, see isEmpty() for
        // explanation)
        Atom getNonEmpty(Atom name) const;

        // this is here for two reasons:
        // (1) it's conceptually nice to have "EMPTY" be protected.
        // (2) some recalcitrant compilers will fail to inline the constant and give
        // a linkage error if "EMPTY" is used outside this class. (We can heal this
        // by declaring a real definition of EMPTY in avmplusHashtable.cpp but then
        // some compilers generate actual loads for the constant, rather than
        // using immediate 0. Sigh...)
        static bool isEmpty(Atom a);

        bool contains(Atom name) const;

        /**
         * Adds a name/value pair to a hash table.  Automatically
         * grows the hash table if it is full.
         *
         * If the table can't be expanded because the number of elements
         * in the table is at the maximum allowable then one of two
         * things can happen:
         *
         *  - if toplevel is NULL then add() will call GCHeap::SignalObjectTooLarge,
         *    which will call GCHeap::Abort() and shut down the player
         *  - otherwise, add() will throw an Error object with tag kOutOfMemoryError
         *
         * Generally, toplevel should only be non-NULL if the operation originated
         * in AS3 code.
         */
        void add(Atom name, Atom value, Toplevel* toplevel = NULL);

        /**
         * The next()/keyAt()/valueAt() methods allow a caller to enumerate all entries in the table.
         * Note that the value passed in is not an index, per se; it's an integral value
         * that indicates that next entry, but is not guaranteed to increase
         * monotonically, or allow for random access: instead, it should be treated
         * as a "magic cookie". Proper usage is (conceptually) as follows:
         *
         *  for (int i = ht->next(0); i != 0; i = ht->next(i))
         *      key = ht->keyAt(i)
         *      value = ht->valueAt(i)
         *
         * i.e., get an initial value by calling next() with a value of 0;
         * continue calling next() until 0 is returned.
         *
         * All nonzero values of the next() int arg are reserved. Don't assume what they mean.
         */

        int next(int index);
        Atom keyAt(int index) const;
        Atom valueAt(int index) const;

    protected:

        /* See CPP for load factor variants. */
        bool isFull() const;

        /**
         * Called to grow the InlineHashtable, particularly by add.
         *
         * - Calculates the needed size for the new InlineHashtable
         *   (typically 2X the current size)
         * - Creates a new array of Atoms
         * - Rehashes the current table into the new one
         * - Deletes the old array of Atoms and sets the Atom
         *   pointer to our new array of Atoms
         *
         * If the table can't be expanded because the number of elements
         * in the table is at the maximum allowable then one of two
         * things can happen:
         *
         *  - if toplevel is NULL then add() will call GCHeap::SignalObjectTooLarge,
         *    which will call GCHeap::Abort() and shut down the player
         *  - otherwise, grow() will throw an Error object with tag kOutOfMemoryError
         *
         * Generally, toplevel should only be non-NULL if the operation originated
         * in AS3 code.
         */
        void grow(Toplevel* toplevel=NULL);

        void setHasDeletedItems();
        void clrHasDeletedItems();

    private:

        // -------------- private constants --------------
        /**
         * since identifiers are always interned strings, they can't be 0,
         * so we can use 0 as the empty value.
         */
        static const Atom EMPTY = 0;

        /** DELETED is stored as the key for deleted items */
        static const Atom DELETED = undefinedAtom;

        // kDontEnumBit is or'd into atoms to indicate that the property is {DontEnum},
        // and ALSO or'd into m_atomsAndFlags to indicate that the InlineHashtable as a whole supports DontEnum.
        static const uintptr_t kDontEnumBit     = 0x01;
        // kHasDeletedItems is or'd into m_atomsAndFlags (but not individual atoms)
        static const uintptr_t kHasDeletedItems = 0x02;
        static const uintptr_t kAtomFlags       = (kDontEnumBit | kHasDeletedItems);

        //
        // "capacity" is the total number of atoms we allocate.
        // we use that capacity as name-value pairs, thus the maximum size at any time is always half the capacity.
        // for 32-bit builds, we want to limit the maximum number of entries to (1<<27)-1,
        // thus we the max capacity we need is (1<<28)-2. but since capacity is always limited
        // to a power of two, we'll actually limit capacity to 1<<27... which in turn will limit
        // maximum number of entries to (1<<26)-1. this has the downside of halving our maximum size,
        // but the upside of avoiding the need to check m_size for overflow on every put (we only
        // need to check capacity for overflow on every grow).
        //
        // (note: for consistency between 32 and 64-bit builds, we'll artificially limit 64-bit systems
        // to the same size, even though the m_size field can hold more.)
        //
        // How does this compare with pre-existing behavior on 32-bit systems?
        // theoretically, capacity could have been an allocation of 1<<32 == 4GB max (it's always a power of two)...
        // but some memory is of course used for other purposes, thus effectively 2GB max.
        // divide by sizeof(Atom) == 4 to find we had an actual max-capacity of 1<<29 entries.
        // So it is mathematically possible that a hashtable that was previously possible
        // will now prematurely run out of memory... but extraordinarily unlikely.
        //
        // (In practice, Win32 limits each process to 2GB, so we can halve the above for those systems,
        // thus "portable" ABC/SWF code could only rely on a max capacity of 1<<28 entries.)
        //
        //
        static const uint32_t MAX_CAPACITY = (1UL<<27);

        // -------------- private methods --------------

        InlineHashtable();
        // do NOT make this virtual; we want InlineHashtable to NOT have ANY virtual methods, not even a dtor
        ~InlineHashtable();

        uintptr_t hasDeletedItems() const;
        void setCapacity(uint32_t cap);
        void put(Atom name, Atom value);
        int rehash(const Atom *oldAtoms, int oldlen, Atom *newAtoms, int newlen) const;
        void throwFailureToGrow(AvmCore* core);
        void setAtoms(Atom* atoms);
        Atom* getAtoms();
        const Atom* getAtoms() const;
        static uint32_t FindOneBit(uint32_t value);
        Atom removeDontEnumMask(Atom a) const;
        bool enumerable(Atom a) const;
        uint32_t getCapacity() const;
        uintptr_t hasDontEnumSupport() const;

        void deletePairAt(int i);

        /**
         * Finds the hash bucket corresponding to the key x
         * in the hash table starting at t, containing tLen
         * atoms.
         *
         * This is a quadratic probe, but we only hit even-numbered
         * slots since those hold keys.
         */
        int find(Atom x, const Atom *t, uint32_t tLen) const;
        int find(Stringp x, const Atom *t, uint32_t tLen) const;
        int find(Atom x) const;


    // ------------------------ DATA SECTION BEGIN
    private:
        uintptr_t m_atomsAndFlags;  /** property hashtable, this has no DWB on purpose, setAtoms contains the WB */
    #ifdef AVMPLUS_64BIT
        // on 64-bit systems, padding will force us to 16 bytes here anyway, so let's just use unpacked ints
        uint32_t m_size;            // number of properties
        uint32_t m_logCapacity;     // (log2 of capacity) + 1
    #else
        uint32_t m_size:27;         // number of properties
        uint32_t m_logCapacity:5;   // (log2 of capacity) + 1 -- gives us enough space for 2^32 entries
    #endif
    // ------------------------ DATA SECTION END
    };

    class HeapHashtable : public MMgc::GCFinalizedObject
    {
    protected:
        InlineHashtable ht;

    public:
        /**
         * initialize with a known capacity.  i.e. we can fit minSize
         * elements in without rehashing.
         * @param heap
         * @param capacity  # of logical slots
         */
        HeapHashtable(MMgc::GC* gc, int32_t capacity = InlineHashtable::kDefaultCapacity);
        virtual ~HeapHashtable();
        InlineHashtable* get_ht();

        void reset();
        uint32_t getSize() const;

        virtual int next(int index);
        Atom keyAt(int index);
        Atom valueAt(int index);

        virtual void add(Atom name, Atom value, Toplevel* toplevel=NULL);
        virtual Atom get(Atom name);
        virtual Atom remove(Atom name);
        virtual bool contains(Atom name) const;

        virtual bool weakKeys() const;
        virtual bool weakValues() const;

#ifdef DEBUGGER
    public:
        /**
         * Basically the same as AvmPlusScriptableObject::bytesUsed().
         */
        virtual uint64_t bytesUsed() const;
#endif
    };

    // Holds RCObject values, not Atom values.  Otherwise like HeapHashtable.
    class HeapHashtableRC : public MMgc::GCFinalizedObject
    {
    private:
        InlineHashtable ht;

    public:
        HeapHashtableRC(MMgc::GC* gc, int32_t capacity = InlineHashtable::kDefaultCapacity);
        virtual ~HeapHashtableRC();

        void reset();
        uint32_t getSize() const;

        int next(int index);
        Atom keyAt(int index);
        MMgc::RCObject* valueAt(int index);

        void add(Atom name, MMgc::RCObject* value, Toplevel* toplevel=NULL);
        MMgc::RCObject* get(Atom name);
        MMgc::RCObject* remove(Atom name);
        bool contains(Atom name) const;

        // used by Flash
        size_t getAllocatedSize() const;

    private:
        Atom tagObject(MMgc::RCObject* obj);
        MMgc::RCObject* untagAtom(Atom a);
    };

    /**
     * If key is an object, weak refs are used
     */
    class WeakKeyHashtable : public HeapHashtable
    {
    public:
        WeakKeyHashtable(MMgc::GC* _gc);

        virtual int next(int index);

        virtual void add(Atom key, Atom value, Toplevel* toplevel=NULL);
        virtual Atom get(Atom key);
        virtual Atom remove(Atom key);
        virtual bool contains(Atom key) const;

        virtual bool weakKeys() const;
    private:
        Atom getKey(Atom key) const;
        void prune();
    };

    /**
     * If value is an object, weak refs are used
     */
    class WeakValueHashtable : public HeapHashtable
    {
    public:
        WeakValueHashtable(MMgc::GC* _gc);

        virtual void add(Atom key, Atom value, Toplevel* toplevel=NULL);
        virtual Atom get(Atom key);
        virtual Atom remove(Atom key);

        virtual bool weakValues() const;
    private:
        Atom getValue(Atom key, Atom value);
        void prune();
    };
}

#endif /* __avmplus_Hashtable__ */
