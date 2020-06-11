#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    {
        Content::Cache<sf::Sprite> c("c");
        sf::Sprite* data = new sf::Sprite();
        Content::Info* info = nullptr;

        //
        info = c.Attach("id", data);
        //

        TEST_ASSERT(info != nullptr);
        TEST_ASSERT(c.Exists("id"));

        c.DeleteAll();
    }
    {
        Content::Cache<sf::Sprite> c("c");
        sf::Sprite* oldData;
        Content::Info* oldInfo = nullptr;
        Content::Info* newInfo = nullptr;

        c.New("id");
        c.Detach("id", oldData, oldInfo);

        //
        newInfo = c.Attach("id", oldData, oldInfo);
        //
        TEST_ASSERT( newInfo != nullptr);
        TEST_ASSERT( oldInfo == newInfo);
        TEST_ASSERT( oldData == c.Get("id"));

        c.DeleteAll();
    }

    return EXIT_SUCCESS;
}
