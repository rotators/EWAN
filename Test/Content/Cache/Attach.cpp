#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    {
        Content c;
        sf::Sprite* data = new sf::Sprite();
        Content::Info* info = nullptr;

        //
        info = c.Sprite.Attach("id", data);
        //

        TEST_ASSERT(info != nullptr);
        TEST_ASSERT(c.Sprite.Exists("id"));

        c.DeleteAll();
    }
    {
        Content c;
        void* oldData = nullptr;
        Content::Info* oldInfo = nullptr;
        Content::Info* newInfo = nullptr;

        c.Sprite.New("id");
        c.Sprite.Detach("id", oldData, oldInfo);

        //
        newInfo = c.Sprite.Attach("id", oldData, oldInfo);
        //
        TEST_ASSERT( newInfo != nullptr);
        TEST_ASSERT( oldInfo == newInfo);
        TEST_ASSERT( oldData == c.Sprite.Get("id"));

        c.DeleteAll();
    }

    return EXIT_SUCCESS;
}
