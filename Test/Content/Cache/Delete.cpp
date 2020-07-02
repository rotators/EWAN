#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    Content c;

    for(auto& cache : CONTENT_CACHE_LIST(c))
    {
        Log::Raw(cache->Name);
        cache->New("id");
        
        //
        cache->Delete("id");
        //

        TEST_ASSERT(cache->Exists("id") == false);
    }

    return EXIT_SUCCESS;
}
