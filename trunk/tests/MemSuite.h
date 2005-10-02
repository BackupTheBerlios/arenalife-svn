#include <cxxtest/TestSuite.h>
#include "../memmanager.h"
#include "../estadistica.h"
#include "../mutation.h"

class MemSuite : public CxxTest::TestSuite 
{
	public:
        void setUp() {
		REC_reset();
		memmanager *mman = memmanager_get();
        }
		
	void testAddition( void )
	{
		TS_ASSERT( 1 + 1 > 1 );
		TS_ASSERT_EQUALS( 1 + 1, 2 );
	}
};
