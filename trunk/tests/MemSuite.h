//#include <iostream>
#include <cxxtest/TestSuite.h>
#include <glib.h>
#include "../memmanager.h"
#include "../estadistica.h"
#include "../mutation.h"


class MemSuite : public CxxTest::TestSuite 
{
	public:
        void setUp() {
		REC_reset();
		memmanager_reset();
	}

	void tearDown() {
		memmanager *mman = memmanager_get();
		g_slist_foreach(mman->free_heap, MemSuite::assertSegment, 0);
        	g_slist_foreach(mman->used_heap, MemSuite::assertSegment, 0);	
	}

	void testNewSegment(void) {
		segment *pseg = segment_new(0, SOUP_SIZE);
		TS_ASSERT_EQUALS(0, pseg->inicio);
		TS_ASSERT_EQUALS(SOUP_SIZE, pseg->size);
		TS_ASSERT_EQUALS(SOUP_SIZE-1, pseg->fin);
	}

	void testNotResizeSegment(void) {
		int size=10;	
		segment *pseg = segment_new(0, size);
		segment_resize(pseg, 0);
		TS_ASSERT_EQUALS(0, pseg->inicio);
		TS_ASSERT_EQUALS(size, pseg->size);
		TS_ASSERT_EQUALS(size-1, pseg->fin);
	}

	void testNegResizeSegment(void) {
		int inicio=20;
		int size=10;
		int inc=-1;	
		segment *pseg = segment_new(inicio, size);
		segment_resize(pseg, inc);
		TS_ASSERT_EQUALS(inicio+inc, pseg->inicio);
		TS_ASSERT_EQUALS(size-inc, pseg->size);
		TS_ASSERT_EQUALS(inicio+size-1, pseg->fin);
	}
	
	void testPosResizeSegment(void) {
		int inicio=20;
		int size=10;
		int inc=2;	
		segment *pseg = segment_new(inicio, size);
		segment_resize(pseg, inc);
		TS_ASSERT_EQUALS(inicio+inc, pseg->inicio);
		TS_ASSERT_EQUALS(size-inc, pseg->size);
		TS_ASSERT_EQUALS(inicio+size-1, pseg->fin);
	}
	
	void testFitSearchOne(void) {
		segment *pseg = 0;
		pseg = segment_fit_search(10);
		TS_ASSERT_EQUALS(pseg->inicio, 0);	
	}
	
	void testFitSearchLimit(void) {
		segment *pseg = 0;
		pseg = segment_fit_search(SOUP_SIZE);
		TS_ASSERT_EQUALS(pseg->inicio, 0);	
		TS_ASSERT_EQUALS(pseg->size, SOUP_SIZE);	
	}
	
	void testFitSearchExceed(void) {
		segment *pseg = 0;
		pseg = segment_fit_search(SOUP_SIZE+1);
		TS_ASSERT(!pseg);	
	}
	
	void testFitSearchAfterMalocar(void) {
		int size=10;
		segment *pseg = 0;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->Vmalloc(size) != -1);
		pseg = segment_fit_search(10);
		TS_ASSERT_EQUALS(pseg->inicio, size);	
	}
	
	void testBadFitSearchAfterMalocar(void) {
		int size=SOUP_SIZE;
		segment *pseg = 0;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->Vmalloc(size) != -1);
		pseg = segment_fit_search(1);
		TS_ASSERT(!pseg);	
	}

	void testManySearchAfterMalocar(void) {
		return;
/*		int i;
		int cant = 100;
		int size = SOUP_SIZE/cant;
		segment *pseg = 0;
		segment** segs = (segment**)malloc(cant*sizeof(segment*));
		memmanager *mman = memmanager_get();
		for (i=0; i<cant; i++) {
			TS_ASSERT(segs[i] = mman->Vmalloc(size));
			pseg = segment_fit_search(1);
			if (i < cant-1)
				TS_ASSERT_EQUALS(pseg->inicio, segs[i]->fin+1);	
		}
		TS_ASSERT_EQUALS(0, mman->total_free());
*/	}
	
	void testSingleton(void) {	
		memmanager *mman1 = memmanager_get();
		memmanager *mman2 = memmanager_get();
        	TS_ASSERT_DIFFERS((int)mman1, 0);
	        TS_ASSERT_DIFFERS((int)mman1, 0);
		TS_ASSERT_EQUALS(mman1, mman2);
	}
	
	void testInit(void) {
		memmanager *mman = memmanager_get();
        	TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);
	}
	
	static void assertSegment(gpointer p, gpointer u_data) {
		segment *pseg = (segment *)p;
		TS_ASSERT_EQUALS(pseg->fin - pseg->inicio + 1, pseg->size);
	}
	
	void testMalocarNada(void) {
		int size = 0;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->Vmalloc(size) != -1);
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE-size);	
	}

	void testMalocarUno(void) {
		int size = SOUP_SIZE/10;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->Vmalloc(size) != -1);
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE-size);	
	}

	void testMalocarMax(void) {
		int size = SOUP_SIZE;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->Vmalloc(size) != -1);
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE-size);	
	}
	
	void testMalocarDemas(void) {
		int size = SOUP_SIZE*2;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->Vmalloc(size) == -1);
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);	
	}

	void testLiberarUno(void) {
		int size = SOUP_SIZE/10;
		memmanager *mman = memmanager_get();
		mman->Vfree(mman->Vmalloc(size));
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);	
	}

	void testLiberarMuchos(void) {
		return;
/*		int i;
		int cant = 100;
		int size = SOUP_SIZE/cant;
		segment** segs = (segment**)malloc(cant*sizeof(segment*));
		memmanager *mman = memmanager_get();
		for (i=0; i<cant; i++) {
			TS_ASSERT(segs[i] = mman->Vmalloc(size));
		}
		TS_ASSERT_EQUALS(0, mman->total_free());
		for (i=0; i<cant; i++) {
			TS_ASSERT(mman->Vfree(segs[i]));
		}
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);
*/	}
};
