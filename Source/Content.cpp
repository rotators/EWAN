#include "Content.hpp"
#include "Log.hpp"
#include "Text.hpp"

#include <algorithm>
#include <filesystem>

#if __has_include(<format>)
 #include <format>
#endif

// Remember to wear protective goggles :)

template class EWAN::Content::Cache<sf::Font>;
template class EWAN::Content::Cache<sf::Sprite>;
template class EWAN::Content::Cache<sf::Texture>;

template<typename T>
EWAN::Content::Cache<T>::Cache(std::string name) :
    Name(name)
{}

template<typename T>
EWAN::Content::Cache<T>::~Cache()
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

template<typename T>
EWAN::Content::Info* EWAN::Content::Cache<T>::Attach(const std::string& id, T* data, Content::Info* info)
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

template<typename T>
T* EWAN::Content::Cache<T>::Detach(const std::string& id)
{
    T*    data;
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

template<typename T>
bool EWAN::Content::Cache<T>::Detach(const std::string& id, T*& data, EWAN::Content::Info*& info)
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

template<typename T>
T* EWAN::Content::Cache<T>::New(const std::string& id)
{
    T* data = new T();

    if(!Attach(id, data))
    {
        delete data;

        return nullptr;
    }

    return data;
}

template<typename T>
bool EWAN::Content::Cache<T>::Delete(const std::string& id)
{
    T* data = Detach(id);

    if(data)
    {
        delete data;
        return true;
    }

    return false;
}

template<typename T>
size_t EWAN::Content::Cache<T>::DeleteAll()
{
    size_t count = 0;

    // As CacheMap is cleared here, there's no need to Detach()

    sf::Lock lock(CacheLock);
    for(const auto& it : CacheMap)
    {
        delete std::get<0>(it.second); // data
        delete std::get<1>(it.second); // info
        count++;
    }

    CacheMap.clear();

    return count;
}

template<typename T>
size_t EWAN::Content::Cache<T>::Move(EWAN::Content::Cache<T>& other)
{
    sf::Lock lockSelf(CacheLock);
    sf::Lock lockOther(other.CacheLock);
    size_t moved = 0;

    std::vector<std::string> keys = Keys();
    T* data;
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

template<typename T>
size_t EWAN::Content::Cache<T>::Size() const
{
    sf::Lock lock(CacheLock);

    return CacheMap.size();
}

template<typename T>
std::vector<std::string> EWAN::Content::Cache<T>::Keys() const
{
    std::vector<std::string> keys;

    sf::Lock lock(CacheLock);
    for(const auto& it : CacheMap)
    {
        keys.emplace_back(it.first);
    }

    return keys;
}

template<typename T>
bool EWAN::Content::Cache<T>::Exists(const std::string& id) const
{
    sf::Lock lock(CacheLock);

    return CacheMap.find(id) != CacheMap.end();
}

template<typename T>
T* EWAN::Content::Cache<T>::Get(const std::string& id, bool silent /*= false */) const
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

template<typename T>
const EWAN::Content::Info* EWAN::Content::Cache<T>::GetInfo(const std::string& id, bool silent /* = false */) const
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

template<typename T>
bool EWAN::Content::Cache<T>::GetDataInfo(const std::string& id, T*& data, EWAN::Content::Info*& info) const
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
    Font("Font"),
    Image("Image"),
    RenderTexture("RenderTexture"),
    SoundBuffer("SoundBuffer"),
    Sprite("Sprite"),
    Texture("Texture"),
    FontExtensions({".ttf"}),
    SoundBufferExtensions({".wav"}),
    TextureExtensions({".png"})
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

    size += Font.Size();
    size += Image.Size();
    size += RenderTexture.Size();
    size += Sprite.Size();
    size += SoundBuffer.Size();
    size += Texture.Size();

    return size;
}

template<typename T>
EWAN::Content::Cache<T>& EWAN::Content::GetCache()
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
const EWAN::Content::Cache<T>& EWAN::Content::GetCache() const
{
    return GetCache<T>();
}

template<typename T>
T* EWAN::Content::LoadFile(const std::string& filename)
{
    std::string id;

    return LoadFile<T>(filename, id);
}

template<typename T>
T* EWAN::Content::LoadFile(const std::string& filename, std::string& id)
{
    const std::string useFilename = Text::ConvertPath(filename);

    if(Text::IsBlank(id))
        id = useFilename;

    // Check if
    Cache<T>& cache = GetCache<T>();
    T* data = cache.Get(id, true);

    if(data)
        return data;

    data = new T();

    if(data->loadFromFile(useFilename) && cache.Attach(id, data))
        return data;

    #if __has_include(<format>)
    Log::Raw(std::format("({}) ERROR", id));
    #else
    Log::Raw("(" + id + ") ERROR");
    #endif

    id.clear();
    delete data;

    return nullptr;
}

size_t EWAN::Content::LoadDirectory(const std::string& directory)
{
    size_t loaded = 0, total = 0;

    if(!std::filesystem::exists(directory))
    {
        #if __has_include(<format>)
        Log::Raw(std::format( "({}) ERROR Directory does not exists", directory));
        #else
        Log::Raw("(" + directory + ") ERROR Directory does not exists");
        #endif

        return loaded;
    }
    else if(!std::filesystem::is_directory(directory))
    {
        #if __has_include(<format>)
        Log::Raw(std::format( "({}) ERROR Not a directory", directory));
        #else
        Log::Raw("(" + directory + ") ERROR Not a directory");
        #endif

        return loaded;
    }

    for(const auto& file : std::filesystem::recursive_directory_iterator(directory))
    {
        if(!std::filesystem::is_regular_file(file))
            continue;

        total++;

        const std::string extension = Text::ToLower(file.path().extension().string());

        if(std::find(FontExtensions.begin(), FontExtensions.end(), extension) != FontExtensions.end())
            loaded += LoadFile<sf::Font>(file.path().string()) ? 1 : 0;
        else if(std::find(SoundBufferExtensions.begin(), SoundBufferExtensions.end(), extension) != SoundBufferExtensions.end())
            loaded += LoadFile<sf::SoundBuffer>(file.path().string()) ? 1 : 0;
        else if(std::find(TextureExtensions.begin(), TextureExtensions.end(), extension) != TextureExtensions.end())
            loaded += LoadFile<sf::Texture>(file.path().string()) ? 1 : 0;
    }

    #if __has_include(<format>)
    Log::Raw(std::format("{}/{} files", loaded, total));
    #else
    Log::Raw(std::to_string(loaded) + "/" + std::to_string(total) + " files");
    #endif

    return loaded;
}
