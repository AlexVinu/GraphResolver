#pragma once

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
}