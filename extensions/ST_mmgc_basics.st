// -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*-
// vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
//
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is [Open Source Virtual Machine.].
//
// The Initial Developer of the Original Code is
// Adobe System Incorporated.
// Portions created by the Initial Developer are Copyright (C) 2004-2006
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Adobe AS3 Team
//
// Alternatively, the contents of this file may be used under the terms of
// either the GNU General Public License Version 2 or later (the "GPL"), or
// the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
// in which case the provisions of the GPL or the LGPL are applicable instead
// of those above. If you wish to allow use of your version of this file only
// under the terms of either the GPL or the LGPL, and not to allow others to
// use your version of this file under the terms of the MPL, indicate your
// decision by deleting the provisions above and replace them with the notice
// and other provisions required by the GPL or the LGPL. If you do not delete
// the provisions above, a recipient may use your version of this file under
// the terms of any one of the MPL, the GPL or the LGPL.
//
// ***** END LICENSE BLOCK ***** */

%%component mmgc
%%category basics

%%methods
using namespace MMgc;
class DeleteInFinalizer : public GCFinalizedObject {
 public:
  // note "small" is a reserved identifier in Visual C++ for Windows Mobile (#defined to be 'char')
  DeleteInFinalizer(GCFinalizedObject *big, GCFinalizedObject *small_) : big(big), small_(small_) {};
  ~DeleteInFinalizer() { delete big; delete small_; }
 private:
  GCFinalizedObject *big;
  GCFinalizedObject *small_;
};

// Any small object would do
class AllocInFinalizer2 : public GCObject {
public:
    void* dummy;
};

class AllocInFinalizer : public GCFinalizedObject {
public:
    AllocInFinalizer() {}
    ~AllocInFinalizer() { new (GC::GetGC(this)) AllocInFinalizer2(); }
};

%%decls
private:
    MMgc::GC *gc;
    MMgc::FixedAlloc *fa;
    MMgc::FixedMalloc *fm;

%%prologue
    gc=new MMgc::GC(MMgc::GCHeap::GetGCHeap(), MMgc::GC::kIncrementalGC);
    if (gc==NULL) {
        MMgc::GCHeap::Init();
        gc=new MMgc::GC(MMgc::GCHeap::GetGCHeap(), MMgc::GC::kIncrementalGC);
    }

%%epilogue
delete gc;

%%test create_gc_instance
    %%verify gc != NULL

%%test create_gc_object
    MMGC_GCENTER(gc);
    MyGCObject *mygcobject;
    mygcobject = (MyGCObject *)new (gc) MyGCObject();
    %%verify mygcobject!=NULL
    mygcobject->i=10;
    %%verify mygcobject->i==10

%%test get_bytesinuse
    MMGC_GCENTER(gc);
    MyGCObject *mygcobject;
    int inuse=(int)gc->GetBytesInUse();
    mygcobject = (MyGCObject *)new (gc) MyGCObject();
//    AvmLog("bytes in use before %d after %d\n",inuse,(int)gc->GetBytesInUse());
    %%verify gc->GetBytesInUse()==inuse + sizeof(MyGCObject) + DebugSize()
    delete mygcobject;

%%test collect
    MMGC_GCENTER(gc);
    MyGCObject *mygcobject;
    int inuse=(int)gc->GetBytesInUse();
    mygcobject = (MyGCObject *)new (gc) MyGCObject();
    %%verify (int)gc->GetBytesInUse()>inuse
    delete mygcobject;
//    AvmLog("collect: inuse=%d current=%d\n",inuse,(int)gc->GetBytesInUse());
    gc->Collect();
//    AvmLog("collect: inuse=%d current=%d\n",inuse,(int)gc->GetBytesInUse());
    %%verify (int)gc->GetBytesInUse()<=inuse

%%test getgcheap
    %%verify gc->GetGCHeap()!=NULL

%%test fixedAlloc
    MMgc::FixedAlloc *fa;
    fa=new MMgc::FixedAlloc(2048,MMgc::GCHeap::GetGCHeap());
    %%verify (int)fa->GetMaxAlloc()==0
    %%verify (int)fa->GetNumBlocks()==0
    void *data1=fa->Alloc(2048);
    %%verify MMgc::FixedAlloc::GetFixedAlloc(data1)==fa
    %%verify fa->GetBytesInUse()==DebugSize()+2048
    %%verify fa->GetItemSize()==2048
    void *data2=fa->Alloc(2048);
    %%verify MMgc::FixedAlloc::GetFixedAlloc(data2)==fa
//    AvmLog("fa->GetItemSize=%d\n",(int)fa->GetItemSize());
    %%verify (int)fa->GetItemSize()==2048
    fa->Free(data1);
    %%verify (int)fa->GetItemSize()==2048
    %%verify (int)fa->GetMaxAlloc()==1
    %%verify (int)fa->GetNumBlocks()==1
    fa->Free(data2);
    delete fa;

%%test fixedMalloc
    fm=MMgc::FixedMalloc::GetFixedMalloc();
    int start=(int)fm->GetBytesInUse();
    int starttotal=(int)fm->GetTotalSize();
//    AvmLog("fm->GetBytesInUse()=%d\n",(int)fm->GetBytesInUse());
    %%verify (int)fm->GetBytesInUse()==start
//    AvmLog("fm->GetTotalSize()=%d\n",(int)fm->GetTotalSize());
    %%verify (int)fm->GetTotalSize()==starttotal
    void *obj=fm->Alloc(8192);
//    AvmLog("fm->GetBytesInUse()=%d\n",(int)fm->GetBytesInUse());
//    %%verify fm->GetBytesInUse()==start + 8192 + MMgc::DebugSize()
//    AvmLog("fm->GetTotalSize()=%d\n",(int)fm->GetTotalSize());
//    %%verify (int)fm->GetTotalSize()==starttotal+2
    fm->Free(obj);
//    AvmLog("fm->GetBytesInUse()=%d\n",(int)fm->GetBytesInUse());
    %%verify (int)fm->GetBytesInUse()==start
//    AvmLog("fm->GetTotalSize()=%d\n",(int)fm->GetTotalSize());
    %%verify (int)fm->GetTotalSize()==starttotal
    obj=fm->Calloc(1024,10);
//    AvmLog("fm->GetBytesInUse()=%d\n",(int)fm->GetBytesInUse());
// FixedMalloc is currently (as of redux 3229) tracking large allocs using a list of
// small objects, in some debug modes.  So we can't have a precise test here.
    %%verify (int)fm->GetBytesInUse()>=start+1024*12 && (int)fm->GetBytesInUse()<=start+1024*12+64
//    AvmLog("fm->GetTotalSize()=%d\n",(int)fm->GetTotalSize());
    %%verify (int)fm->GetTotalSize()==starttotal+3
    fm->Free(obj);
    %%verify (int)fm->GetBytesInUse()==start
    %%verify (int)fm->GetTotalSize()==starttotal

%%test gcheap
    MMgc::GCHeap *gh=MMgc::GCHeap::GetGCHeap();
    int startfreeheap=(int)gh->GetFreeHeapSize();
//    %%verify (int)gh->GetTotalHeapSize()==128
//    AvmLog("gh->GetFreeHeapSize()=%d\n",(int)gh->GetFreeHeapSize());
    %%verify (int)gh->GetFreeHeapSize()==startfreeheap
//gh->Config().heapLimit = 1024;
//    %%verify (int)gh->GetTotalHeapSize()==128
//    AvmLog("gh->GetFreeHeapSize()=%d\n",(int)gh->GetFreeHeapSize());
    %%verify (int)gh->GetFreeHeapSize()==startfreeheap
       void *data = gh->Alloc(10,MMgc::GCHeap::kExpand | MMgc::GCHeap::kZero);
    %%verify (int)gh->GetTotalHeapSize()>startfreeheap
//    AvmLog("gh->GetFreeHeapSize()=%d\n",(int)gh->GetFreeHeapSize());
	   gh->FreeNoProfile(data);
       
%%test gcheapAlign
    MMgc::GCHeap *gh=MMgc::GCHeap::GetGCHeap();

    // Tricky: try to provoke some internal asserts
    void *d[1000];
    for ( unsigned i=0 ; i < ARRAY_SIZE(d) ; i++ ) {
        d[i] = gh->Alloc(1);
        void *data = gh->Alloc(10,MMgc::GCHeap::flags_Alloc, 4);
        gh->Free(data);
    }
    for ( unsigned i=0 ; i < ARRAY_SIZE(d) ; i++ )
        gh->Free(d[i]);

    // 
    for ( size_t k=2 ; k <= 256 ; k *= 2 ) {
        void *data = gh->Alloc(10,MMgc::GCHeap::flags_Alloc, k);
        %%verify ((uintptr_t)data & (k*MMgc::GCHeap::kBlockSize - 1)) == 0
        %%verify gh->Size(data) == 10
        gh->Free(data);
    }

%%test gcmethods
    MMGC_GCENTER(gc);
    MyGCObject *mygcobject;
    mygcobject = (MyGCObject *)new (gc) MyGCObject();
    %%verify (MyGCObject *)gc->FindBeginningGuarded(mygcobject)==mygcobject
    %%verify (MyGCObject *)gc->FindBeginningFast(mygcobject)==mygcobject

%%test gcLargeAlloc
    MMGC_GCENTER(gc);
    MyGCObject *mygcobject;
    mygcobject = (MyGCObject *)new (gc) MyGCObject();
    void *obj=gc->Alloc(10024,0);
    %%verify MMgc::GCLargeAlloc::IsLargeBlock(GetRealPointer(obj))==true
    %%verify MMgc::GCLargeAlloc::FindBeginning(obj)==GetRealPointer(obj)
    %%verify MMgc::GCLargeAlloc::IsFinalized(obj)==false
    MMgc::GCLargeAlloc::SetFinalize(obj);
    %%verify MMgc::GCLargeAlloc::IsFinalized(obj)==true
    MMgc::GCLargeAlloc::ClearFinalized(obj);
    %%verify MMgc::GCLargeAlloc::IsFinalized(obj)==false
    %%verify MMgc::GCLargeAlloc::ContainsPointers(obj)==false
    %%verify MMgc::GCLargeAlloc::HasWeakRef(obj)==false
    MMgc::GCLargeAlloc::SetHasWeakRef(obj,true);
    %%verify MMgc::GCLargeAlloc::HasWeakRef(obj)==true
    MMgc::GCLargeAlloc::SetHasWeakRef(obj,false);
    %%verify MMgc::GCLargeAlloc::HasWeakRef(obj)==false
    %%verify MMgc::GCLargeAlloc::GetMark(obj)==false
    MMgc::GCLargeAlloc::SetMark(obj);
    %%verify MMgc::GCLargeAlloc::GetMark(obj)==true

// Bugzilla 542529 - in debug mode we would assert here due to logic flaws in the allocatr
%%test finalizerAlloc
    MMGC_GCENTER(gc);
    new (gc) AllocInFinalizer();
    gc->Collect(false);
    %%verify true

%%test finalizerDelete
    MMGC_GCENTER(gc);
    new (gc) DeleteInFinalizer(new (gc, 100) GCFinalizedObject(), new (gc) GCFinalizedObject());
    //delete m; delete m; // this verifies we crash, it does
    gc->Collect(false);
    %%verify true
    GCFinalizedObject *gcfo = new (gc) GCFinalizedObject();
    gcfo->~GCFinalizedObject();
    gcfo->~GCFinalizedObject(); // this used to be a deleteing dtor and would crash, not anymore



%%test nestedGCs
    GC *gcb = new GC(GCHeap::GetGCHeap(), GC::kIncrementalGC);
    MMGC_GCENTER(gc);
    void *a = gc->Alloc(8);
    {
        MMGC_GCENTER(gcb);
        a = gcb->Alloc(8);
        {
            MMGC_GCENTER(gc);
            a = gc->Alloc(8);
        }
        a = gcb->Alloc(8);
    }
    a = gc->Alloc(8);
    // just fishing for asserts/hangs/crashes
    %%verify true
    delete gcb;

%%test collectDormantGC
    {
        GC *gcb = new GC(GCHeap::GetGCHeap(), GC::kIncrementalGC);
        {
            MMGC_GCENTER(gcb);
            gcb->Alloc(8);
        }

        // this will cause a Collection in gcb
        GCHeap::SignalExternalFreeMemory();
        delete gcb;

        // just fishing for asserts/hangs/crashes
        %%verify true
    }

%%test regression_551169
    {
        GC *testGC = new GC(GCHeap::GetGCHeap(), GC::kIncrementalGC);
        {
            MMGC_GCENTER(testGC);
            testGC->StartIncrementalMark();
            // self test for tricky GCRoot deletion logic
            // do this a bunch, idea is to try to hit GetItemAbove border edge cases
            for(int i=0;i<10000;i++) {
                GCRoot *fauxRoot = new GCRoot(testGC, new char[GC::kMarkItemSplitThreshold*2], GC::kMarkItemSplitThreshold*2);
                testGC->MarkAllRoots();
                // tail of fauxRoot is on stack
                GCWorkItem *sentinel = fauxRoot->GetMarkStackSentinelPointer();
                if(sentinel) {
                    %%verify sentinel->GetSentinelPointer() == fauxRoot
                    GCWorkItem *tail = testGC->m_incrementalWork.GetItemAbove(sentinel);
                    %%verify tail->iptr + tail->GetSize() == (uintptr_t) fauxRoot->End()
                    %%verify sentinel != NULL
                }
                delete [] (char*)fauxRoot->Get();
                delete fauxRoot;
                if(sentinel) {
                    %%verify sentinel->GetSentinelType() == GCWorkItem::kDeadItem
                    %%verify testGC->m_incrementalWork.GetItemAbove(sentinel)->GetSentinelType() == GCWorkItem::kDeadItem
                }
            }
            testGC->Mark();
            testGC->ClearMarkStack();
            testGC->ClearMarks();
        }
        delete testGC;
    }
