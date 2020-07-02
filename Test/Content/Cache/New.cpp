#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    Content c;

    for(auto& cache : CONTENT_CACHE_LIST(c))
    {
        Log::Raw(cache->Name);
        TEST_ASSERT(cache->New("id") != nullptr);
        TEST_ASSERT(cache->New("id") == nullptr);

        TEST_ASSERT(cache->New("") == nullptr);
        TEST_ASSERT(cache->New(" ") == nullptr);
        TEST_ASSERT(cache->New("\t") == nullptr);
        TEST_ASSERT(cache->New(" \t") == nullptr);

    }

    return EXIT_SUCCESS;
}
