#include "Content.hpp"
#include "Test.hpp"

TEST_MAIN
{
    {
        Content _old;
        Content _new;

        _old.Sprite.New("test");

        void* oldData = nullptr;
        Content::Info* oldInfo = nullptr;
        _old.Sprite.GetDataInfo("test", oldData, oldInfo);

        //
        TEST_ASSERT(_old.Sprite.Move(_new.Sprite) > 0);
        //

        void* newData = nullptr;
        Content::Info* newInfo = nullptr;
        _new.Sprite.GetDataInfo("test", newData, newInfo);

        TEST_ASSERT(!_old.Sprite.Exists("test"));
        TEST_ASSERT(_new.Sprite.Exists("test"));

        TEST_ASSERT(oldData == newData);
        TEST_ASSERT(oldInfo == newInfo);

        _old.DeleteAll();
        _new.DeleteAll();
    }
    {
        Content _old;
        Content _new;

        _old.Sprite.New("test");

        //
        TEST_ASSERT(_old.Sprite.Move(_new.Image) == 0);
        //

        _old.DeleteAll();
        _new.DeleteAll();
    }
    return EXIT_SUCCESS;
}
