#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    Content c;
    void* oldData =  nullptr;
    void* newData =  nullptr;
    Content::Info* oldInfo = nullptr;
    Content::Info* newInfo = nullptr;

    c.Sprite.New("id");
    c.Sprite.GetDataInfo( "id", oldData, oldInfo );

    //
    c.Sprite.Detach("id", newData, newInfo);
    //

    TEST_ASSERT(newData != nullptr);
    TEST_ASSERT(newInfo != nullptr);
    TEST_ASSERT(oldData == newData);
    TEST_ASSERT(oldInfo == newInfo);

    c.DeleteAll();

    return EXIT_SUCCESS;
}
