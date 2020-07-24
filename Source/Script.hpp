#pragma once

#include "GameInfo.hpp"

#include "Libs/AngelScript.hpp"

#include <functional>
#include <list>
#include <map>
#include <string>
#include <utility> // std::forward

namespace EWAN
{
    class App;

    class Script
    {
    public:
        enum class SuspendReason
        {
            Unknown,
            Delay,
            Yield
        };

        class API
        {
        public:
            static bool Init(App* app, as::asIScriptEngine* engine, const std::string& ns = "EWAN");
            static bool InitEngineCallback(Script* script, as::asIScriptEngine* engine);
            static bool InitContextCallback(Script* script, as::asIScriptContext* context);

        protected:
            static void AppLog(App*, std::string text);

        private:
            static int RegisterContentCache(as::asIScriptEngine* engine, const std::string& type, const std::string& subtype = {});
        };

        class Builder : public as::CScriptBuilder
        {
        public:
            Builder();

            int StartNewModule(as::asIScriptEngine* inEngine, const char* moduleName);

            std::map<int, std::vector<std::string>>& GetAllMetadataForType();
            std::map<int, std::vector<std::string>>& GetAllMetadataForFunc();
            std::map<int, std::vector<std::string>>& GetAllMetadataForVar();

            std::string NormalizePath(const std::string& path);
        };

        class Callback
        {
        public:
            static int Include(const char* include, const char* from, as::CScriptBuilder* builder, void* data);
            static int Pragma(const std::string& pragmaText, as::CScriptBuilder& builder, void* data);

            static as::asIScriptContext* ContextRequest(as::asIScriptEngine* engine, void* data);
            static void                  ContextReturn(as::asIScriptEngine* engine, as::asIScriptContext* context, void* data);

            static void ContextUserDataCleanup(as::asIScriptContext* context);
            static void EngineUserDataCleanup(as::asIScriptEngine* engine);
            static void FunctionUserDataCleanup(as::asIScriptFunction* function);
            static void ModuleUserDataCleanup(as::asIScriptModule* module);
        };

        class Event
        {
        public:
            const std::string      Name;
            std::list<std::string> Params;

            bool IgnoreExecuteErrors = false;

        protected:
            std::list<as::asIScriptFunction*> Functions;

            static std::function<void(as::asIScriptContext*)> NOP;

        public:
            Event(std::string name, std::list<std::string> params);
            virtual ~Event();

        private:
            Event() = delete;

        public:
            std::string GetDeclaration(const std::string& name = "f") const;
            std::string GetDeclaration(as::asIScriptFunction* function) const;

        public:
            void Register(as::asIScriptFunction* function);
            void Unregister(as::asIScriptEngine* engine);
            void Unregister(as::asIScriptModule* module);

            bool Run();
            bool Run(float& arg0);
            bool RunBool(bool& result);

        protected:
            bool Run(std::function<void(as::asIScriptContext* context)> init, std::function<void(as::asIScriptContext* context)> finish);
            bool Execute(as::asIScriptContext*& context, std::list<as::asIScriptContext*>& yield, std::function<void(as::asIScriptContext* context)> finish);

        public:
            bool RunOnBuild(as::asIScriptModule* module);
            bool RunOnInit(as::asIScriptEngine* engine, as::asIScriptFunction*& function);
        };

        class UserData
        {
        public:
            struct Context
            {
                EWAN::Script::Event*        Event         = nullptr;
                EWAN::Script::SuspendReason SuspendReason = EWAN::Script::SuspendReason::Unknown;

                void Reset()
                {
                    Event         = nullptr;
                    SuspendReason = EWAN::Script::SuspendReason::Unknown;
                }
            };

            struct Engine
            {
                EWAN::Script*                    Script = nullptr;
                std::list<as::asIScriptContext*> ContextCache;
            };

            struct Function
            {
                bool Dummy = true;
            };

            struct Module
            {
                bool Debug    = false;
                bool Optional = false;
                bool Poison   = false;
                bool Unload   = false;

                bool Import = false;
            };

        public:
            static Context*  Get(as::asIScriptContext* context);
            static Engine*   Get(as::asIScriptEngine* engine);
            static Function* Get(as::asIScriptFunction* function);
            static Module*   Get(as::asIScriptModule* module);
        };

        //

    public:
        // Special events; requires snowflake handling

        Event OnBuild;
        Event OnInit;

        // Regular events

        Event OnFinish;
        Event OnDraw;

    private:
        std::string          RootDirectory;
        as::asIScriptEngine* AS = nullptr;

    public:
        Script();
        virtual ~Script();

    public:
        bool Init(App* app);
        void Finish();

        static void WriteInfo(as::asIScriptEngine* engine, const std::string& message, const std::string& section = {}, int row = 0, int col = 0);
        static void WriteWarning(as::asIScriptEngine* engine, const std::string& message, const std::string& section = {}, int row = 0, int col = 0);
        static void WriteError(as::asIScriptEngine* engine, const std::string& message, const std::string& section = {}, int row = 0, int col = 0);

        bool LoadModule(as::asIScriptEngine* engine, const std::string& fileName, const std::string& moduleName);
        bool LoadModule_Call(const std::string& fileName, const std::string& moduleName);
        bool LoadInitModule(const GameInfo& game, as::asIScriptEngine* engine);
        bool UnloadModule(as::asIScriptModule*& module);
        bool UnloadModule_Call(const std::string& moduleName);

        bool LoadModuleMetadata(Builder& builder);

        void Delay_Call();
        void Yield_Call();

    protected:
        bool                 BindImportedFunctions(as::asIScriptEngine* engine);
        bool                 BindImportedFunctions(as::asIScriptModule* module);
        as::asIScriptEngine* CreateEngine();
        void                 DestroyEngine(as::asIScriptEngine*& engine);

    public:
        void                  CallbackContextLine(as::asIScriptContext* context);
        as::asIScriptContext* CallbackContextRequest(as::asIScriptEngine* engine, void* data);
        void                  CallbackContextReturn(as::asIScriptEngine* engine, as::asIScriptContext* context, void* data);
        int                   CallbackInclude(Builder& builder, const std::string& include, const std::string& fromSection, void* data);
        void                  CallbackMessage(const as::asSMessageInfo& msg);
        int                   CallbackPragma(Builder& builder, const std::string& pragmaText, void* data);

    public:
        template<typename C, typename T>
        void AppendStdContainerToArray(const C& container, as::CScriptArray* arr, bool addRef)
        {
            if(!container.empty() && arr)
            {
                as::asUINT current = arr->GetSize();
                arr->Resize(static_cast<as::asUINT>(current + container.size()));

                for(T containerObject : container)
                {
                    T* arrayObject = static_cast<T*>(arr->At(current++));
                    *arrayObject   = containerObject;
                    if(addRef)
                        (*arrayObject)->AddRef();
                }
            }
        }

        template<typename T>
        void AppendListToArray(const std::list<T>& container, as::CScriptArray* arr, bool addRef)
        {
            AppendStdContainerToArray<std::list<T>, T>(container, arr, addRef);
        }

        template<typename T>
        void AppendVectorToArray(const std::vector<T>& container, as::CScriptArray* arr, bool addRef)
        {
            AppendStdContainerToArray<std::vector<T>, T>(container, arr, addRef);
        }
    };
}
