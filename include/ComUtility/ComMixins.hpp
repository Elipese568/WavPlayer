#pragma once

#include "ComConcepts.hpp"

namespace ComUtility
{
    template<ComInterface T>
    class ComObject;

    class EmptyMixin
    {
    };

    template<typename T>
    class ServiceMixin
    {
    protected:

        T* GetServiceSource() const noexcept;

    public:

        template<ComInterface TService>
        ComObject<TService> Service();
    };

    template<typename T>
    constexpr bool has_get_service_v =
    requires(T* p)
    {
        p->GetService(
            GUID{},
            (void**)nullptr
        );
    };

    template<typename T>
    using ServiceBase =
        std::conditional_t<
            has_get_service_v<T>,
            ServiceMixin<T>,
            EmptyMixin
        >;
}

#include "ComMixins.inl"