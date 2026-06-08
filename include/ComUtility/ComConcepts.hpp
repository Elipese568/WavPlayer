#pragma once

#include <concepts>
#include <unknwn.h>

#include "ComTraits.hpp"

namespace ComUtility
{
    template<typename T>
    concept ComInterface =
        std::derived_from<T, IUnknown>;

    template<typename T>
    concept CoCreatable =
    requires
    {
        { ComClassTraits<T>::clsid } -> std::same_as<const CLSID&>;
    };
    
    template<typename T>
    concept HasActivate =
    requires(T* obj)
    {
        obj->Activate(
            IID{},
            DWORD{},
            nullptr,
            static_cast<void**>(nullptr)
        );
    };
}