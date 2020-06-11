#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    Content _old;
    Content::Cache<sf::Sprite> _new("spr");

    _old.Sprite.New("test");

    sf::Sprite* oldData = nullptr;
    Content::Info* oldInfo = nullptr;
     _old.Sprite.GetDataInfo("test", oldData, oldInfo);

    //
    _old.Sprite.Move(_new);
    //

    sf::Sprite* newData = nullptr;
    Content::Info* newInfo = nullptr;
     _new.GetDataInfo("test", newData, newInfo);

    TEST_ASSERT(!_old.Sprite.Exists("test"));
    TEST_ASSERT(_new.Exists("test"));

    TEST_ASSERT(oldData == newData);
    TEST_ASSERT(oldInfo == newInfo);

    _old.DeleteAll();
    _new.DeleteAll();

    return EXIT_SUCCESS;
}
