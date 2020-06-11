#pragma once

#include "SFML.hpp"

#include <cstdint>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>

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
        template<typename T>
        class Cache : sf::NonCopyable
        {
        public:
            const std::string Name;

        protected:
            std::unordered_map<std::string, std::pair<T*, Info*>> CacheMap;
            mutable sf::Mutex                                     CacheLock;

        public:
            Cache(std::string name);
            virtual ~Cache();

        private:
            // Makes sure there's no unnamed caches
            Cache() = delete;

        public:
            // Adds data to cache
            // Once data is added to cache, it's automagically deleted when needed
            // Returns nullptr if data cannot be added
            Info* Attach(const std::string& id, T* data, Info* info = nullptr);

            // Removes data from cache, deletes info
            // Returns cached data
            T* Detach(const std::string& id);

            // Removes data from cache
            // Returns cached data and info
            bool Detach(const std::string& id, T*& data, Info*& info);

            // Creates new data and adds it to cache
            // Supports default constructor only (new T())
            // Returns cached data
            T* New(const std::string& id);

            // Removes data from cache and deletes it
            // Returns false if data cannot be found
            bool Delete(const std::string& id);

            // Removes and deletes all data from cache
            // Returns amount of deleted entries
            size_t DeleteAll();

            size_t Move(Cache<T>& other);

            //  Returns amount of cached entries
            size_t Size() const;

            std::vector<std::string> Keys() const;

            // Returns true if data with given ID has been added to cache
            bool Exists(const std::string& id) const;

            // Returns cached data
            T* Get(const std::string& id, bool silent = false) const;

            // Returns cached info
            const Info* GetInfo(const std::string& id, bool silent = false) const;

            // Returns false if data with given ID wasn't added to cache
            bool GetDataInfo(const std::string& id, T*& data, Info*& info) const;
        };

    public:
        Cache<sf::Font>          Font;
        Cache<sf::Image>         Image;
        Cache<sf::RenderTexture> RenderTexture;
        Cache<sf::SoundBuffer>   SoundBuffer;
        Cache<sf::Sprite>        Sprite;
        Cache<sf::Texture>       Texture;

    // used by LoadDirectory() to guess
    public:
        std::vector<std::string> FontExtensions;
        std::vector<std::string> SoundBufferExtensions;
        std::vector<std::string> TextureExtensions;

    public:
        Content();
        virtual ~Content();

    public:
        void   DeleteAll();

        // Returns total size of all caches
        size_t Size() const;

        // Returns cache matching given type
        template<typename T>
        Cache<T>& GetCache();
        template<typename T>
        const Cache<T>& GetCache() const;

        template<typename T>
        T*  LoadFile(const std::string& filename);
        template<typename T>
        T*  LoadFile(const std::string& filename, std::string& id);

        // Load all known content in given directory, and cache it in
        size_t LoadDirectory(const std::string& directory);
    };
}

extern template class EWAN::Content::Cache<sf::Font>;
extern template class EWAN::Content::Cache<sf::Sprite>;
extern template class EWAN::Content::Cache<sf::Texture>;
