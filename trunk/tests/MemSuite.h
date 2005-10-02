#include <cxxtest/TestSuite.h>
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
		
	void testInit(void) {
		memmanager *mman = memmanager_get();
        	TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);
	}

	void testMalocarNada(void) {
		int size = 0;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->malocar(size));
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE-size);	
	}

	void testMalocarUno(void) {
		int size = SOUP_SIZE/10;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->malocar(size));
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE-size);	
	}

	void testMalocarMax(void) {
		int size = SOUP_SIZE;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->malocar(size));
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE-size);	
	}
	
	void testMalocarDemas(void) {
		int size = SOUP_SIZE*2;
		memmanager *mman = memmanager_get();
		TS_ASSERT(!mman->malocar(size));
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);	
	}

	void testLiberarUno(void) {
		int size = SOUP_SIZE/10;
		memmanager *mman = memmanager_get();
		TS_ASSERT(mman->liberar(mman->malocar(size)));
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);	
	}

	void testLiberarFalla(void) {
		memmanager *mman = memmanager_get();
		TS_ASSERT(!mman->liberar(0));
		//segment* pseg = (segment*)malocar(sizeof(char));
		//TS_ASSERT(!mman->liberar(pseg));
	}

	void testLiberarMuchos(void) {
		int i;
		int cant = 100;
		int size = SOUP_SIZE/cant;
		segment** segs = (segment**)malloc(cant*sizeof(segment*));
		memmanager *mman = memmanager_get();
		for (i=0; i<cant; i++) {
			TS_ASSERT(segs[i] = mman->malocar(size));
		}
		for (i=0; i<cant; i++) {
			TS_ASSERT(mman->liberar(segs[i]));
		}
		TS_ASSERT_EQUALS(mman->total_free(), SOUP_SIZE);
	}
	
};