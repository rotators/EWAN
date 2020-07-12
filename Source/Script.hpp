#pragma once

#include "GameInfo.hpp"

#include "Libs/AngelScript.hpp"

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
            Yield,
            Suspend
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
            static int RegisterContentCache(as::asIScriptEngine* engine, const std::string& cacheName, const std::string& typeName = {});
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
            class Data
            {
            public:
                std::list<std::string>             Params;
                std::list<as::asIScriptFunction*>& List;

            public:
                Data(std::list<std::string> params, std::list<as::asIScriptFunction*>& list);

            public:
                std::string GetDeclaration(const std::string& name = "f") const;
                std::string GetDeclaration(as::asIScriptFunction* function) const;
            };

            // Special events; requires snowflake handling

            std::list<as::asIScriptFunction*> OnBuild;
            std::list<as::asIScriptFunction*> OnInit;
            std::list<as::asIScriptFunction*> OnFinish;

            // Regular events

            std::list<as::asIScriptFunction*> OnDraw;
            std::list<as::asIScriptFunction*> OnKeyDown;
            std::list<as::asIScriptFunction*> OnKeyUp;
            std::list<as::asIScriptFunction*> OnMouseMove;
            std::list<as::asIScriptFunction*> OnMouseDown;
            std::list<as::asIScriptFunction*> OnMouseUp;

        public:
            void Register(std::list<as::asIScriptFunction*>& functions, as::asIScriptFunction* function, const std::string& name);
            void Unregister(as::asIScriptModule* module);
            void Unregister(std::list<as::asIScriptFunction*>& functions, as::asIScriptEngine* engine, const std::string& name);
            void Unregister(std::list<as::asIScriptFunction*>& functions, as::asIScriptModule* module, const std::string& name);

            bool Run(as::asIScriptContext* context);
            bool Run(as::asIScriptContext* context, float& arg0);
            bool RunOnBuild(as::asIScriptModule* module);
            bool RunOnInit(as::asIScriptEngine* engine, as::asIScriptFunction*& function);
            void RunOnFinish(as::asIScriptEngine* engine);

            template<typename... Args>
            bool Run(const std::list<as::asIScriptFunction*>& functions, Args&&... args)
            {
                if(functions.empty())
                    return true;

                as::asIScriptContext* context = functions.front()->GetEngine()->RequestContext();

                bool result = true;
                for(const auto& function : functions)
                {
                    context->Prepare(function);

                    if(!Run(context, std::forward<Args>(args)...))
                    {
                        result = false;
                        break;
                    }
                }

                context->GetEngine()->ReturnContext(context);
                return result;
            }
        };

        class UserData
        {
        public:
            struct Context
            {
                EWAN::Script::SuspendReason SuspendReason = EWAN::Script::SuspendReason::Unknown;

                void Reset()
                {
                    SuspendReason = EWAN::Script::SuspendReason::Unknown;
                }
            };

            struct Engine
            {
                EWAN::Script* Script = nullptr;
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
        EWAN::Script::Event Event;

    private:
        std::string                      RootDirectory;
        as::asIScriptEngine*             AS = nullptr;
        std::list<as::asIScriptContext*> ContextCache;

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

        void Yield_Call();
        void Suspend_Call();

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
