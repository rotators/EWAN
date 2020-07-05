#pragma once

#include "GameInfo.hpp"

#include "Libs/SFML.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility> // pair
#include <vector>

namespace EWAN
{
    class Content : sf::NonCopyable
    {
    public:
        class Info
        {
        public:
            Info() {}
            virtual ~Info() {}
        };

    public:
        class Cache : sf::NonCopyable
        {
        public:
            const std::string Name;

            // Used by Content::LoadFile() to guess target cache
            std::vector<std::string> Extensions;

        protected:
            std::unordered_map<std::string, std::pair<void*, Info*>> CacheMap;
            mutable sf::Mutex                                        CacheLock;

        private:
            typedef std::function<void* ()>     CallbackNewFunction;
            typedef std::function<void (void*)> CallbackDeleteFunction;

            CallbackNewFunction    CallbackNew;
            CallbackDeleteFunction CallbackDelete;

        public:
            Cache(const std::string& name, CallbackNewFunction callbackNew, CallbackDeleteFunction callbackDelete, const std::vector<std::string>& extensions = {});
            virtual ~Cache();

        private:
            // Makes sure there's no uninitialized caches
            Cache() = delete;

        public:
            // Adds data to cache
            // Once data is added to cache, it's automagically deleted when needed
            // Returns nullptr if data cannot be added
            Info* Attach(const std::string& id, void* data, Info* info = nullptr);

            // Removes data from cache, deletes info
            // Returns cached data
            void* Detach(const std::string& id);

            // Removes data from cache
            // Returns cached data and info
            bool Detach(const std::string& id, void*& data, Info*& info);

            // Creates new data and adds it to cache
            // Supports default constructor only (new T())
            // Returns cached data
            void* New(const std::string& id);

            template<typename T>
            T* NewAs(const std::string& id)
            {
                return static_cast<T*>(New(id));
            }

            // Removes data from cache and deletes it
            // Returns false if data cannot be found
            bool Delete(const std::string& id);

            // Removes and deletes all data from cache
            // Returns amount of deleted entries
            size_t DeleteAll();

            size_t Move(Cache& other);

            //  Returns amount of cached entries
            size_t Size() const;

            std::vector<std::string> Keys() const;

            // Returns true if data with given ID has been added to cache
            bool Exists(const std::string& id) const;

            // Returns cached data
            void* Get(const std::string& id, bool silent = false) const;

            template<typename T>
            T* GetAs(const std::string& id, bool silent = false) const
            {
                return static_cast<T*>(Get(id, silent));
            }

            // Returns cached info
            const Info* GetInfo(const std::string& id, bool silent = false) const;

            // Returns false if data with given ID wasn't added to cache
            bool GetDataInfo(const std::string& id, void*& data, Info*& info) const;
        };

    public:
        std::string RootDirectory;

        Cache Font;
        Cache Image;
        Cache RenderTexture;
        Cache SoundBuffer;
        Cache Sprite;
        Cache Texture;

    public:
        Content();
        virtual ~Content();

    public:
        bool Init(const GameInfo& game);
        void Finish();

        // Deletes stored data in all caches
        size_t DeleteAll();

        // Returns total size of all caches
        size_t Size() const;

    protected:
        // Returns cache matching given type
        template<typename T>
        Cache& GetCache();
        template<typename T>
        const Cache& GetCache() const;
        template<typename T>
        T*  LoadFileInternal(const std::string& filename, const std::string& id);
    
    public:
        bool   LoadFile(const std::string& filename, const std::string& id);
        size_t LoadDirectory(const std::string& directory);
    };
}
