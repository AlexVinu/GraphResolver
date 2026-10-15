#include <string_view>

#include "core.h"

namespace graph_resolver{
    using namespace graph_core;

    class GraphResolver;
    // ----------------- Functions signatures --------------------
    template<class T>
    using ConstructFn = T*(*)(GraphResolver*);

    template<class T>
    using PostConstructFn = void(*)(GraphResolver*, T*);

    template<class T>
    using DestructFn = void(*)(T*);

    template<class T>
    DestructFn<T> DefaultDestruct = [](T* ptr){delete ptr;};
    // -----------------------------------------------------------

    // --------------------- ModuleSlot --------------------------
    class ModuleSlotBase
    {
    public:
        virtual void Construct(GraphResolver*) = 0;
        virtual void PostConstruct(GraphResolver*) = 0;
        virtual void Destruct() = 0;
        virtual ModIdView GetDeps() const = 0;
        virtual ModIdView GetPostDeps() const = 0;
        virtual void* GetInterfacePtr() const = 0;
        virtual void DestroySelf() = 0;
        virtual StringView GetInterfaceName() const = 0;
        virtual StringView GetTypeName() const = 0;    
    protected:
        virtual ~ModuleSlotBase() = default;
    };

    template<class I, class T,
        typename DEPS = TypeList<>,
        typename POSTDEPS = TypeList<>
    > requires std::is_base_of_v<I, T> && IsTypeList<DEPS> && IsTypeList<POSTDEPS>
    class ModuleSlot : public ModuleSlotBase
    {
    public:
        ModuleSlot(
            ConstructFn<T> fn1,
            PostConstructFn<T> fn2 = nullptr,
            DestructFn<T> fn_d = DefaultDestruct<T>)
            : m_Construct(fn1), m_PostConstruct(fn2), m_Destruct(fn_d), m_ModulePtr(nullptr){}

        void Construct(GraphResolver* loader) override 
        {
            if(m_Construct) m_ModulePtr = m_Construct(loader);
        }
        void PostConstruct(GraphResolver* loader) override 
        {
            if(m_PostConstruct && m_ModulePtr) m_PostConstruct(loader, m_ModulePtr);
        }
        void Destruct() override 
        {
            if(m_ModulePtr) m_Destruct(m_ModulePtr);
            m_ModulePtr = nullptr;
        }

        ModIdView GetDeps() const override {return DEPS::get();}
        ModIdView GetPostDeps() const override {return POSTDEPS::get();}
        void* GetInterfacePtr() const override {return static_cast<I*>(m_ModulePtr);}
        void DestroySelf() override {delete this;}
        StringView GetInterfaceName() const override 
        {
            std::string_view view = GET_TYPE_NAME(I);
            return {view.data(), view.size()};
        }
        StringView GetTypeName() const override 
        {
            std::string_view view = GET_TYPE_NAME(T);
            return {view.data(), view.size()};
        }
    private:    
        ~ModuleSlot() override
        {
            Destruct();
        }

        T* m_ModulePtr;
        ConstructFn<T> m_Construct;
        PostConstructFn<T> m_PostConstruct;
        DestructFn<T> m_Destruct;
    };
    // -------------------------------------------------------------------
    class GraphResolver
    {
    public:
        GraphResolver();
        ~GraphResolver();

        template<
            class I, class T,
            typename DEPS = TypeList<>,
            typename POSTDEPS = TypeList<>
            >
        GraphResult AddModule(
            ConstructFn<T> construct,
            PostConstructFn<T> post_construct = nullptr,
            DestructFn<T> destruct = DefaultDestruct<T>) 
        {
            modid_t indx = hash_string(GET_TYPE_NAME(I));
            std::string_view view = GET_TYPE_NAME(T);
            if(HasModule(indx)){
                return {GraphErrorCode::DuplicateModuleID, {view.data(), view.size()}}; 
            }

            ModuleSlotBase* module_base = new ModuleSlot<I, T, DEPS, POSTDEPS>(
                std::move(construct), std::move(post_construct), std::move(destruct));
            AddModule(indx, module_base);
            return {GraphErrorCode::Success};
        }

        
        GraphResult CompileGraph();
        void ExecuteGraph();
        void DestructInOrder();
        void DestroyGraph();

        template<class I>
        I* GetModule()
        {
            modid_t indx(hash_string(GET_TYPE_NAME(I)));
            return static_cast<I*>(GetModuleInt(indx));
        }

    private:
        struct Impl;
        Impl* m_Impl;

        void* GetModuleInt(modid_t indx);
        void AddModule(modid_t indx, ModuleSlotBase* module_base);
        bool HasModule(modid_t indx);
    };
}