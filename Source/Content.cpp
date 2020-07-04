#include "Content.hpp"
#include "Log.hpp"
#include "Text.hpp"

#include <algorithm>
#include <filesystem>

#if __has_include(<format>)
 #include <format>
#endif

// Remember to wear protective goggles :)

namespace
{
    static void* NewFont()
    {
        return new sf::Font;
    }

    static void DeleteFont(void* data)
    {
        delete static_cast<sf::Font*>(data);
    }

    static void* NewImage()
    {
        return new sf::Image;
    }

    static void DeleteImage(void* data)
    {
        delete static_cast<sf::Image*>(data);
    }

    static void* NewRenderTexture()
    {
        return new sf::RenderTexture;
    }

    static void DeleteRenderTexture(void* data)
    {
        delete static_cast<sf::RenderTexture*>(data);
    }

    static void* NewSoundBuffer()
    {
        return new sf::SoundBuffer;
    }

    static void DeleteSoundBuffer(void* data)
    {
        delete static_cast<sf::SoundBuffer*>(data);
    }

    static void* NewSprite()
    {
        return new sf::Sprite;
    }

    static void DeleteSprite(void* data)
    {
        delete static_cast<sf::Sprite*>(data);
    }

    static void* NewTexture()
    {
        return new sf::Sprite;
    }

    static void DeleteTexture(void* data)
    {
        delete static_cast<sf::Texture*>(data);
    }
}

//

EWAN::Content::Cache::Cache(const std::string& name, CallbackNewFunction callbackNew, CallbackDeleteFunction callbackDelete, const std::vector<std::string>& extensions /*= {} */) :
    Name(name),
    Extensions(extensions),
    CallbackNew(callbackNew), CallbackDelete(callbackDelete)
{}

EWAN::Content::Cache::~Cache()
{
    if(!CacheMap.empty())
    {
        sf::Lock lock(CacheLock);

        #if __has_include(<format>)
        Log::Raw(std::format("WARNING : Size={}", CacheMap.size()));
        #else
        Log::Raw("WARNING : Size=" + std::to_string(CacheMap.size()));
        #endif

        DeleteAll();
    }
}

//

EWAN::Content::Info* EWAN::Content::Cache::Attach(const std::string& id, void* data, Content::Info* info)
{
    if(Text::IsBlank(id))
    {
        Log::Raw("ERROR : Blank ID");

        return nullptr;
    }
    else if(Exists(id))
    {
        #if __has_include(<format>)
        Log::Raw(std::format("({}) ERROR : ID already in use", id));
        #else
        Log::Raw("(" + id + ") ERROR : ID already in use");
        #endif

        return nullptr;
    }
    else if(!data)
    {
        #if __has_include(<format>)
        Log::Raw(std::format("({}) ERROR : Null data", id));
        #else
        Log::Raw("(" + id + ") ERROR : Null data");
        #endif

        return nullptr;
    }

    if(!info)
        info = new Info();

    sf::Lock lock(CacheLock);
    CacheMap.emplace(id, std::make_pair(data, info));

    return info;
}

void* EWAN::Content::Cache::Detach(const std::string& id)
{
    void* data;
    Info* info;

    if(GetDataInfo(id, data, info))
    {
        sf::Lock lock(CacheLock);

        delete info;
        CacheMap.erase(id);
        return data;
    }

    return nullptr;
}

bool EWAN::Content::Cache::Detach(const std::string& id, void*& data, EWAN::Content::Info*& info)
{
    data = nullptr;
    info = nullptr;

    if(GetDataInfo(id, data, info))
    {
        sf::Lock lock(CacheLock);

        CacheMap.erase(id);
        return true;
    }

    return false;
}

void* EWAN::Content::Cache::New(const std::string& id)
{
    void* data = CallbackNew();

    if(!Attach(id, data))
    {
        CallbackDelete(data);

        return nullptr;
    }

    return data;
}

bool EWAN::Content::Cache::Delete(const std::string& id)
{
    void* data = Detach(id);

    if(data)
    {
        CallbackDelete(data);
        return true;
    }

    return false;
}

size_t EWAN::Content::Cache::DeleteAll()
{
    size_t count = 0;

    // As CacheMap is cleared here, there's no need to Detach()

    sf::Lock lock(CacheLock);
    for(const auto& it : CacheMap)
    {
        CallbackDelete(std::get<0>(it.second)); // data
        delete std::get<1>(it.second); // info
        count++;
    }

    CacheMap.clear();

    return count;
}

size_t EWAN::Content::Cache::Move(Cache& other)
{
    if(Name != other.Name)
        return 0;

    sf::Lock lockSelf(CacheLock);
    sf::Lock lockOther(other.CacheLock);
    size_t moved = 0;

    std::vector<std::string> keys = Keys();
    void* data;
    Info* info;

    for(const auto& key : keys)
    {
        if(Detach(key, data, info))
        { 
            moved += other.Attach(key, data, info) ? 1 : 0;
        }
    }

    return moved;
}

size_t EWAN::Content::Cache::Size() const
{
    sf::Lock lock(CacheLock);

    return CacheMap.size();
}

std::vector<std::string> EWAN::Content::Cache::Keys() const
{
    std::vector<std::string> keys;

    sf::Lock lock(CacheLock);
    for(const auto& it : CacheMap)
    {
        keys.emplace_back(it.first);
    }

    return keys;
}

bool EWAN::Content::Cache::Exists(const std::string& id) const
{
    sf::Lock lock(CacheLock);

    return CacheMap.find(id) != CacheMap.end();
}

void* EWAN::Content::Cache::Get(const std::string& id, bool silent /*= false */) const
{
    sf::Lock lock(CacheLock);

    auto it = CacheMap.find(id);
    if(it != CacheMap.end())
        return std::get<0>(it->second);

    // Error handling intentionally left blank

    if(!silent)
    {
        #if __has_include(<format>)
        Log::Raw(std::format("({}) ERROR", id));
        #else
        Log::Raw("(" + id + ") ERROR");
        #endif
    }

    return nullptr;
}

const EWAN::Content::Info* EWAN::Content::Cache::GetInfo(const std::string& id, bool silent /* = false */) const
{
    sf::Lock lock(CacheLock);

    auto it = CacheMap.find(id);
    if(it != CacheMap.end())
        return std::get<1>(it->second);
    
    // Error handling intentionally left blank

    if(!silent)
    {
        #if __has_include(<format>)
        Log::Raw(std::format("({}) ERROR", id));
        #else
        Log::Raw("(" + id + ") ERROR");
        #endif
    }

    return nullptr;
}

bool EWAN::Content::Cache::GetDataInfo(const std::string& id, void*& data, EWAN::Content::Info*& info) const
{
    data = nullptr;
    info = nullptr;

    sf::Lock lock(CacheLock);

    auto it = CacheMap.find(id);
    if(it != CacheMap.end())
    {
        data = std::get<0>(it->second);
        info = std::get<1>(it->second);

        return true;
    }

    return false;
}

//

EWAN::Content::Content() :
    Font("Font", NewFont, DeleteFont, {".bdf", ".pcf", ".ttf"}),
    Image("Image", NewImage, DeleteImage),
    RenderTexture("RenderTexture", NewRenderTexture, DeleteRenderTexture),
    SoundBuffer("SoundBuffer", NewSoundBuffer, DeleteSoundBuffer, {".wav"}),
    Sprite("Sprite", NewSprite, DeleteSprite),
    Texture("Texture", NewTexture, DeleteTexture, {".png"})
{
    // Ugly way to make sure all containers are supported by GetCache()

    GetCache<sf::Font>();
    GetCache<sf::Image>();
    GetCache<sf::RenderTexture>();
    GetCache<sf::SoundBuffer>();
    GetCache<sf::Sprite>();
    GetCache<sf::Texture>();
}

EWAN::Content::~Content()
{
    DeleteAll();
}

//

bool EWAN::Content::Init(const GameInfo& game)
{
    RootDirectory = std::filesystem::path(game.Path).remove_filename().make_preferred().string();

    return true;
}

void EWAN::Content::Finish()
{
    DeleteAll();
}

//

void EWAN::Content::DeleteAll()
{
    Font.DeleteAll();
    Image.DeleteAll();
    RenderTexture.DeleteAll();
    SoundBuffer.DeleteAll();
    Sprite.DeleteAll();
    Texture.DeleteAll();
}

size_t EWAN::Content::Size() const
{
    size_t size = 0;

    for(auto& cache : {&Font, &Image, &RenderTexture, &Sprite, &SoundBuffer, &Texture})
    {
        size += cache->Size();
    }

    return size;
}

//

template<typename T>
EWAN::Content::Cache& EWAN::Content::GetCache()
{
    if constexpr(std::is_same_v<T,sf::Font>)
        return Font;
    else if constexpr(std::is_same_v<T,sf::Image>)
        return Image;
    else if constexpr(std::is_same_v<T,sf::RenderTexture>)
        return RenderTexture;
    else if constexpr(std::is_same_v<T,sf::SoundBuffer>)
        return SoundBuffer;
    else if constexpr(std::is_same_v<T,sf::Sprite>)
        return Sprite;
    else if constexpr(std::is_same_v<T,sf::Texture>)
        return Texture;
}

template<typename T>
const EWAN::Content::Cache& EWAN::Content::GetCache() const
{
    return GetCache<T>();
}

template<typename T>
T* EWAN::Content::LoadFileInternal(const std::string& filename, const std::string& id)
{
    // Check if id is already in use
    Cache& cache = GetCache<T>();
    T* data = cache.GetAs<T>(id, true);

    if(data)
        return data;

    // create SFML object
    data = new T(); 

    if(data->loadFromFile(filename) && cache.Attach(id, data))
        return data;

    #if __has_include(<format>)
    Log::Raw(std::format("({}) ERROR", id));
    #else
    Log::Raw("(" + id + ") ERROR");
    #endif

    delete data;

    return nullptr;
}

//

bool EWAN::Content::LoadFile(const std::string& filename, const std::string& id)
{
    const std::string extension = Text::ToLower(std::filesystem::path(filename).extension().string());

    if(std::find(Font.Extensions.begin(), Font.Extensions.end(), extension) != Font.Extensions.end())
        return LoadFileInternal<sf::Font>(filename, id) != nullptr;
    else if(std::find(SoundBuffer.Extensions.begin(), SoundBuffer.Extensions.end(), extension) != SoundBuffer.Extensions.end())
        return LoadFileInternal<sf::SoundBuffer>(filename, id) != nullptr;
    else if(std::find(Texture.Extensions.begin(), Texture.Extensions.end(), extension) != Texture.Extensions.end())
        return LoadFileInternal<sf::Texture>(filename, id) != nullptr;

    return false;
}

size_t EWAN::Content::LoadDirectory(const std::string& directory)
{
    std::string dir;
    size_t loaded = 0, total = 0;

    // Disallow escaping .game path
    if(directory.front() == '.')
    {
        if(directory == ".")
            dir = RootDirectory;
        else
            return loaded;
    }
    else if(directory.front() == '/' || directory.front() == '\\')
        return loaded;
    else  if(directory.length() >= 3 && directory[1] == ':' && (directory[2] == '/' || directory[2] == '\\'))
        return loaded;
    else  if(directory.length() >= 2 && directory[1] == ':')
        return loaded;
    // Directory is always relative to .game path
    else
        dir = RootDirectory + directory;

    if(!std::filesystem::exists(dir))
    {
        #if __has_include(<format>)
        Log::Raw(std::format( "({}) ERROR Directory does not exists", dir));
        #else
        Log::Raw("(" + dir + ") ERROR Directory does not exists");
        #endif

        return loaded;
    }
    else if(!std::filesystem::is_directory(dir))
    {
        #if __has_include(<format>)
        Log::Raw(std::format( "({}) ERROR Not a directory", dir));
        #else
        Log::Raw("(" + dir + ") ERROR Not a directory");
        #endif

        return loaded;
    }

    for(const auto& file : std::filesystem::recursive_directory_iterator(dir))
    {
        if(!std::filesystem::is_regular_file(file))
            continue;

        total++;

        std::filesystem::path path = file.path();
        path.make_preferred();

        // Set id to *NIX path relative to .game directory
        std::string id = Text::Replace(path.string().substr(RootDirectory.length()), "\\", "/");

        if(LoadFile(path.string(), id))
            loaded++;
    }

    #if __has_include(<format>)
    Log::Raw(std::format("{}/{} files", loaded, total));
    #else
    Log::Raw(std::to_string(loaded) + "/" + std::to_string(total) + " files");
    #endif

    return loaded;
}
