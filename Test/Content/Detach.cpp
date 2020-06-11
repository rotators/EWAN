#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    Content::Cache<sf::Sprite> c("c");
    sf::Sprite* oldData =  nullptr;
    sf::Sprite* newData =  nullptr;
    Content::Info* oldInfo = nullptr;
    Content::Info* newInfo = nullptr;

    c.New("id");
    c.GetDataInfo( "id", oldData, oldInfo );

    //
    c.Detach("id", newData, newInfo);
    //

    TEST_ASSERT(newData != nullptr);
    TEST_ASSERT(newInfo != nullptr);
    TEST_ASSERT(oldData == newData);
    TEST_ASSERT(oldInfo == newInfo);

    c.DeleteAll();

    return EXIT_SUCCESS;
}
