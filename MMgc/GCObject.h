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
 *   leon.sha@sun.com
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

#ifndef __GCObject__
#define __GCObject__

#ifdef __GNUC__
    #define GNUC_ONLY(x) x
#else
    #define GNUC_ONLY(x)
#endif

// VC++ wants these declared
//void *operator new(size_t size);
//void *operator new[] (size_t size);

// Sun Studio doesn't support default parameters for operator new, so break up as two functions
REALLY_INLINE void *operator new(size_t size, MMgc::GC *gc)
{
    return gc->AllocPtrZero(size);
}

REALLY_INLINE void *operator new(size_t size, MMgc::GC *gc, int flags)
{
    return gc->Alloc(size, flags);
}

namespace MMgc
{
    /**
     * Baseclass for GC managed objects that aren't finalized
     */
    class GCObject
    {
    public:
        // 'throw()' annotation to avoid GCC warning: 'operator new' must not return NULL unless it is declared 'throw()' (or -fcheck-new is in effect)
        static void *operator new(size_t size, GC *gc, size_t extra) GNUC_ONLY(throw());

        static void *operator new(size_t size, GC *gc) GNUC_ONLY(throw());

        static void operator delete(void *gcObject);

        GCWeakRef *GetWeakRef() const;
    };

    /**
     *  Baseclass for GC managed objects that are finalized
     */
    class GCFinalizedObject
    //: public GCObject can't do this, get weird compile errors in AVM plus, I think it has to do with
    // the most base class (GCObject) not having any virtual methods)
    {
    public:
        GCWeakRef *GetWeakRef() const;

        virtual ~GCFinalizedObject();
        static void* operator new(size_t size, GC *gc, size_t extra);
        static void* operator new(size_t size, GC *gc);
        static void operator delete (void *gcObject);
    };

    REALLY_INLINE void *GCObject::operator new(size_t size, GC *gc, size_t extra) GNUC_ONLY(throw())
    {
        return gc->AllocExtraPtrZero(size, extra);
    }

    REALLY_INLINE void *GCObject::operator new(size_t size, GC *gc) GNUC_ONLY(throw())
    {
        return gc->AllocPtrZero(size);
    }

    REALLY_INLINE void GCObject::operator delete(void *gcObject)
    {
        GC::GetGC(gcObject)->FreeNotNull(gcObject);
    }

    REALLY_INLINE GCWeakRef* GCObject::GetWeakRef() const
    {
        return GC::GetWeakRef(this);
    }

    REALLY_INLINE GCFinalizedObject::~GCFinalizedObject()
    {
        // Nothing
    }

    REALLY_INLINE GCWeakRef* GCFinalizedObject::GetWeakRef() const
    {
        return GC::GetWeakRef(this);
    }

    REALLY_INLINE void* GCFinalizedObject::operator new(size_t size, GC *gc, size_t extra)
    {
        return gc->AllocExtraPtrZeroFinalized(size, extra);
    }

    REALLY_INLINE void* GCFinalizedObject::operator new(size_t size, GC *gc)
    {
        return gc->AllocPtrZeroFinalized(size);
    }

    REALLY_INLINE void GCFinalizedObject::operator delete (void *gcObject)
    {
        GC::GetGC(gcObject)->FreeNotNull(gcObject);
    }

    /**
     * Base class for reference counted objects.
     *
     * This object always has a finalizer (the C++ destructor).  The C++ destructor /must/
     * leave the object with all-fields-zero.
     *
     * Reference counting is deferred: when an object's reference count drops to zero it
     * is inserted into the zero-count table (the ZCT), see ZCT.h etc.  If the reference
     * count grows above zero the object is removed from the ZCT again.  Every new object
     * is added to the ZCT initially - reference counts start at zero.
     *
     * Occasionally the ZCT is reaped: objects in the ZCT that are not referenced from stack
     * memory or special ZCT roots, and that are not explicitly pinned by client code, are
     * deleted by calling their finalizers and reclaiming their memory.   A finalizer may
     * make the reference counts of more referenced objects drop to zero, whereupon they
     * too are entered into the ZCT (and may be deleted by the ongoing reap, or a later
     * reap).
     *
     * (Under complicated scenarios it is possible for an object allocated during reaping
     * to be erroneously deleted, see https://bugzilla.mozilla.org/show_bug.cgi?id=506644,
     * so client code may want to take note of that.)
     *
     *
     * Invariants on 'composite'.
     *
     * The field 'compsite' holds reference counts, the ZCT index, and flag bits.  We depend
     * on object layout being such that this field is the second word of the object.  This
     * field is used for a heuristic defense-in-depth mechanism:
     *
     *  - In release builds its value is zero iff the object's destructor has been run and
     *    the object has not been reallocated (ie the object may or may not yet have been
     *    placed on a free list).
     *  - In debug builds its value is zero iff the object's destructor has been run and the
     *    object has not yet been put on the free list or been reallocated.  Once it's been
     *    put on the free list the composite word is a cookie: GCHeap::GCFreedPoison or
     *    GCHeap::GCSweptPoison.
     *
     * The RC operations check whether the object is free - and if it is, then the operations
     * have no effect.  This is useful because of the current finalization semantics:
     * finalization and destruction are intertwined, so that the C++ destructor performs both
     * jobs.  Thus an object can't be finalized without being destroyed.  Finalizers run
     * in arbitrary order; thus a finalizer for an object that's part of an object network
     * will not know whether it is safe to operate on other objects in that network.  The
     * extra guards on RC operations makes this safer.  It only works because all objects that
     * are finalizable are finalized together, before any of them are swept or reallocated.
     * It is also an expense that everyone pays for.  The only fix is a change to how
     * finalization is handled.
     */

    class RCObject : public GCFinalizedObject
    {
        friend class GC;
        friend class ZCT;
    public:
        REALLY_INLINE static void *operator new(size_t size, GC *gc, size_t extra)
        {
            return gc->AllocExtraRCObject(size, extra);
        }

        REALLY_INLINE static void *operator new(size_t size, GC *gc)
        {
            return gc->AllocRCObject(size);
        }

        REALLY_INLINE RCObject()
        {
            composite = 1;
            GC::GetGC(this)->AddToZCT(this REFCOUNT_PROFILING_ARG(true));
        }

        REALLY_INLINE ~RCObject()
        {
            // The ZCT reaper removes the object from the ZCT before deleting it,
            // but when non-GC code explicitly deletes the object we may find that
            // it is already in the ZCT.  So remove it if necessary.
            if (InZCT())
                GC::GetGC(this)->RemoveFromZCT(this REFCOUNT_PROFILING_ARG(true));
            composite = 0;
        }

        /**
         * @return true if the object is currently pinned (explicitly or by
         *         the ZCT's stack pinner).
         */
        REALLY_INLINE bool IsPinned()
        {
            return (composite & STACK_PIN) != 0;
        }

#ifdef _DEBUG
        REALLY_INLINE bool IsGCPoisoned() {
            return composite == uint32_t(GCHeap::GCFreedPoison) || composite == uint32_t(GCHeap::GCSweptPoison);
        }
#endif
        
        /**
         * Explicitly pin the object, protecting it from ZCT reaping.  The pin
         * flag /will/ be cleared if the object is subsequently added to the
         * ZCT and reaping is not ongoing.  It is not advised to call Pin()
         * except from prereap() callback handlers.
         */
        void Pin()
        {
#ifdef _DEBUG
            // This is a deleted object so ignore it.
            if(IsGCPoisoned())
                return;
#endif
            // This is a deleted/free object so ignore it.
            if (composite == 0)
                return;

            composite |= STACK_PIN;
        }

        /**
         * Explicitly unpin the object, allowing it to be reaped by the ZCT.
         * It is not advised to unpin objects that weren't pinned explicitly
         * by a call to Pin(), and calls to Unpin() should come from postreap()
         * callback handlers.
         */
        void Unpin()
        {
#ifdef _DEBUG
            // This is a deleted object so ignore it.
            if(IsGCPoisoned())
                return;
#endif
            // This is a deleted/free object so ignore it.
            if (composite == 0)
                return;

            composite &= ~STACK_PIN;
        }

        /**
         * @return the object's current reference count.  The value is not
         * valid unless Sticky() returns 0.
         */
        REALLY_INLINE uint32_t RefCount() const
        {
            return (composite & RCBITS) - 1;
        }

        /**
         * @return non-zero if the object is sticky (RC operations have no effect because
         *         the RC field is invalid, either as a consequence of an RC overflow or
         *         because the sticky bit was set explicitly).
         */
        REALLY_INLINE uint32_t Sticky() const
        {
            return composite & STICKYFLAG;
        }

        /**
         * Make RC operations on the object be no-ops.
         */
        REALLY_INLINE void Stick()
        {
#ifdef _DEBUG
            // This is a deleted object so ignore it.
            if(IsGCPoisoned())
                return;
#endif
            // This is a deleted/free object so ignore it.
            if (composite == 0)
                return;

            if (InZCT())
                GC::GetGC(this)->RemoveFromZCT(this REFCOUNT_PROFILING_ARG(true));
            composite |= STICKYFLAG;
        }

        /**
         * Increment the object's reference count.
         *
         * OPTIMIZEME: this is too expensive and too large.  It should be
         * possible to do better if we play around with the positive/negative
         * boundary (eg, operations that don't cross that boundary are
         * cheap, and those that do aren't), but the special-casing of
         * composite==0 and invariants throughtout the system relying on
         * composite==0 make it hard.  We need profiling data on the breakdown
         * of the frequency of various paths through this function.
         */
        REALLY_INLINE void IncrementRef()
        {
#ifdef _DEBUG
            // This is a deleted object so ignore it.
            if(IsGCPoisoned())
                return;
#endif
            // This is a deleted/free object so ignore it.
            if (composite == 0)
                return;

            REFCOUNT_PROFILING_ONLY( GC::GetGC(this)->policy.signalIncrementRef(); )
            if(Sticky())
                return;
#ifdef _DEBUG
            GC* gc = GC::GetGC(this);
            GCAssert(gc->IsRCObject(this));
            GCAssert(this == gc->FindBeginningGuarded(this));
#endif

            composite++;
            if((composite&RCBITS) == RCBITS) {
                composite |= STICKYFLAG;
            } else if(InZCT()) {
                GCAssert(RefCount() == 1);
                GC::GetGC(this)->RemoveFromZCT(this);
            }

#ifdef MMGC_RC_HISTORY
            if(GC::GetGC(this)->keepDRCHistory)
                history.Add(GCHeap::GetGCHeap()->GetProfiler()->GetStackTrace());
#endif
        }

        /**
         * Decrement the object's reference count.
         *
         * OPTIMIZEME: this is too expensive and too large.  It should be
         * possible to do better if we play around with the positive/negative
         * boundary (eg, operations that don't cross that boundary are
         * cheap, and those that do aren't), but the special-casing of
         * composite==0 and invariants throughtout the system relying on
         * composite==0 make it hard.  We need profiling data on the breakdown
         * of the frequency of various paths through this function.
         */
        REALLY_INLINE void DecrementRef()
        {
#ifdef _DEBUG
            // This is a deleted object so ignore it.
            if(IsGCPoisoned())
                return;
#endif
            // This is a deleted/free object so ignore it.
            if (composite == 0)
                return;

            REFCOUNT_PROFILING_ONLY( GC::GetGC(this)->policy.signalDecrementRef(); )
            if(Sticky())
                return;

#ifdef _DEBUG
            GC* gc = GC::GetGC(this);
            GCAssert(gc->IsRCObject(this));
            GCAssert(this == gc->FindBeginningGuarded(this));

            // ???
            if(gc->Destroying())
                return;

            if(RefCount() == 0) {
#ifdef MMGC_RC_HISTORY
                DumpHistory();
#endif
                GCAssert(false);
            }
#endif

            if (RefCount() == 0)
            {
                // This is a defensive measure.  If DecrementRef is
                // ever called on a zero ref-count object, composite--
                // will cause an underflow, flipping all kinds of bits
                // in bad ways and resulting in a crash later.  Often,
                // such a DecrementRef bug would be caught by the
                // _DEBUG asserts above, but sometimes we have
                // release-only crashers like this.  Better to fail
                // gracefully at the point of failure, rather than
                // push the failure to some later point.
                return;
            }

            composite--;

#ifdef MMGC_RC_HISTORY
            if(GC::GetGC(this)->keepDRCHistory)
                history.Add(GCHeap::GetGCHeap()->GetProfiler()->GetStackTrace());
#endif

            if(RefCount() == 0)
                GC::GetGC(this)->AddToZCT(this);
        }

#ifdef MMGC_RC_HISTORY
        void DumpHistory();
#endif

    private:

        // @return non-zero if the object is in the ZCT
        uint32_t InZCT() const
        {
            return composite & ZCTFLAG;
        }

        // Clear the ZCT flag and the ZCT index
        void ClearZCTFlag()
        {
            composite &= ~(ZCTFLAG|ZCT_INDEX);
        }

        // @return the ZCT index.  This is only valid if InZCT returns non-zero.
        uint32_t getZCTIndex() const
        {
            return (composite & ZCT_INDEX) >> 8;
        }

        // Set the ZCT index and the ZCT flag and clear the pinned flag.
        void setZCTIndexAndUnpin(uint32_t index)
        {
            GCAssert(index <= (ZCT_INDEX>>8));
            composite = (composite&~(ZCT_INDEX|STACK_PIN)) | ((index<<8)|ZCTFLAG);
        }

        // Set the ZCT index and the ZCT flag.  If reaping==0 then clear the pinned flag,
        // otherwise preserve the pinned flag.
        void setZCTIndexAndMaybeUnpin(uint32_t index, uint32_t reaping)
        {
            GCAssert(reaping == 0 || reaping == 1);
            GCAssert(index <= (ZCT_INDEX>>8));
            composite = (composite&~(ZCT_INDEX|((~reaping&1)<<STACK_PIN_SHIFT))) | ((index<<8)|ZCTFLAG);
        }

        // Fields in 'composite'
        static const uint32_t ZCTFLAG            = 0x80000000;          // The object is in the ZCT
        static const uint32_t STICKYFLAG         = 0x40000000;          // The object is sticky (RC overflow)
        static const uint32_t STACK_PIN          = 0x20000000;          // The object has been pinned
        static const uint32_t STACK_PIN_SHIFT    = 29;
        static const uint32_t RCBITS             = 0x000000FF;          // 8 bits for the reference count
        static const uint32_t ZCT_INDEX          = 0x0FFFFF00;          // 20 bits for the ZCT index
        static const uint32_t ZCT_CAPACITY       = (ZCT_INDEX>>8) + 1;

        uint32_t composite;
#ifdef MMGC_RC_HISTORY
        // addref/decref stack traces
        BasicList<StackTrace*> history;
#endif // MMGC_MEMORY_INFO
    };

    template<class T>
    class ZeroPtr
    {
    public:
        ZeroPtr() { t = NULL; }
        ZeroPtr(T _t) : t(_t) { }
        ~ZeroPtr()
        {
            t = NULL;
        }

        operator T() { return t; }
        bool operator!=(T other) { return other != t; }
        T operator->() const { return t; }
    private:
        T t;
    };

    template<class T>
    class RCPtr
    {
    public:
        RCPtr() { t = NULL; }
        RCPtr(T _t) : t(_t)
        {
            if(valid())
                t->IncrementRef();
        }
        ~RCPtr()
        {
            if(valid())
                t->DecrementRef();

            // 02may06 grandma : I want to enable
            //  class DataIOBase { DRC(PlayerToplevel *) const m_toplevel; }
            //
            // DataIOBase is a virtual base class, so we don't know if the
            // subclass is GCObject or not. We need it to be const, or
            // a GCObject would require a DWB(), and if it's const, we
            // cannot zero it out during ~DataIOBase. The simplest solution
            // seemed to be zeroing out the member here.

            t = NULL;
        }

        T operator=(T tNew)
        {
            if(valid())
                t->DecrementRef();
            t = tNew;
            if(valid())
                t->IncrementRef();
            // this cast is safe b/c other wise compilation would fail
            return (T) t;
        }

        RCPtr<T>& operator=(const RCPtr<T>& other)
        {
            if(valid())
                t->DecrementRef();
            t = other.t;
            if(valid())
                t->IncrementRef();
            return *this;
        }

        operator T() const
        {
            return (T) t;
        }

        operator ZeroPtr<T>() const { return t; }

        bool operator!=(T other) { return other != t; }

        T operator->() const
        {
            return (T) t;
        }

        void Clear() { t = NULL; }

    private:
        // Hidden and meant not to be used at all.
        RCPtr(const RCPtr<T>& other);

        inline bool valid() { return (uintptr_t)t > 1; }
        T t;
    };


// put spaces around the template arg to avoid possible digraph warnings
#define DRC(_type) MMgc::RCPtr< _type >

#undef GNUC_ONLY

}


#endif /* __GCObject__ */
