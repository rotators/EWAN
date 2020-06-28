#pragma once

#include "GameInfo.hpp"

#include "Libs/AngelScript.hpp"

#include <list>
#include <map>
#include <string>

namespace EWAN
{
    class App;

    class Script
    {
    public:
        class Builder : public as::CScriptBuilder
        {
        public:
            Builder();

            int StartNewModule(as::asIScriptEngine* inEngine, const char *moduleName);

            std::map<int, std::vector<std::string>>& GetAllMetadataForType();
            std::map<int, std::vector<std::string>>& GetAllMetadataForFunc();
            std::map<int, std::vector<std::string>>& GetAllMetadataForVar();

            std::string NormalizePath(const std::string& path);
        };

        class Callback
        {
        public:
            static int Include(const char *include, const char *from, as::CScriptBuilder* builder, void* data);
            static int Pragma(const std::string& pragmaText, as::CScriptBuilder& builder, void* data);

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
                std::list<std::string> Params;
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
            bool RunOnBuild(as::asIScriptModule* module);
            bool RunOnInit(as::asIScriptEngine* engine, as::asIScriptFunction*& function);

            template<typename... Args>
            bool Run(std::list<as::asIScriptFunction*>& functions, Args&&... args)
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

        struct UserData
        {
            struct Context
            {
                bool Dummy = true;
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
                bool Debug = false;
                bool Optional = false;
                bool Poison = false;
                bool Unload = false;
            };
        };

    //

    public:
        EWAN::Script::Event Event;

    private:
        std::string RootDirectory;
        as::asIScriptEngine* AS = nullptr;

    public:
        Script();
        virtual ~Script();

    public:
        bool Init(App* app);
        bool InitAPI(App* app, as::asIScriptEngine* engine, const std::string& ns = "EWAN");
        bool InitMessageCallback(as::asIScriptEngine* engine);
        void Finish();

        static UserData::Context* GetUserData(as::asIScriptContext* context);
        static UserData::Engine* GetUserData(as::asIScriptEngine* engine);
        static UserData::Function* GetUserData(as::asIScriptFunction* function);
        static UserData::Module* GetUserData(as::asIScriptModule* module);

        bool LoadModule(as::asIScriptEngine* engine, const std::string& fileName, const std::string& moduleName);
        bool LoadModule_Call(const std::string& fileName, const std::string& moduleName);
        bool LoadInitModule(const GameInfo& game, as::asIScriptEngine* engine);
        bool UnloadModule(as::asIScriptModule*& module);
        bool UnloadModule_Call(const std::string& moduleName);

        bool LoadModuleMetadata(Builder& builder);

    protected:
        bool BindImportedFunctions(as::asIScriptEngine* engine);
        bool BindImportedFunctions(as::asIScriptModule* module);
        as::asIScriptContext* CreateContext(as::asIScriptEngine* engine);
        as::asIScriptEngine* CreateEngine();
        void DestroyEngine(as::asIScriptEngine*& engine);

    public:
        void WriteInfo(as::asIScriptEngine* engine, const std::string& message, const std::string& section = {}, int row = 0, int col = 0);
        void WriteWarning(as::asIScriptEngine* engine, const std::string& message, const std::string& section = {}, int row = 0, int col = 0);
        void WriteError(as::asIScriptEngine* engine, const std::string& message, const std::string& section = {}, int row = 0, int col = 0);

        int CallbackInclude(Builder& builder, const std::string& include, const std::string& fromSection, void* data);
        void CallbackMessage(const as::asSMessageInfo& msg);
        int CallbackPragma(Builder& builder, const std::string& pragmaText, void* data);
    };
}
