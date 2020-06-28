#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    Content c;

    TEST_ASSERT(c.Sprite.New("") == nullptr);
    TEST_ASSERT(c.Sprite.New(" ") == nullptr);
    TEST_ASSERT(c.Sprite.New("\t") == nullptr);
    TEST_ASSERT(c.Sprite.New(" \t") == nullptr);

    return EXIT_SUCCESS;
}