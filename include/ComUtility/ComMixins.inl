#pragma once

#include "ComMixins.hpp"
#include "ComConcepts.hpp"

namespace ComUtility
{
    template<typename T>
    T* ServiceMixin<T>::GetServiceSource() const noexcept
    {
        return static_cast<
            const ComObject<T>*
        >(this)->Get();
    }

    template<typename T>
    template<ComInterface TService>
    ComObject<TService>
    ServiceMixin<T>::Service()
    {
        ComObject<TService> result;

        HRESULT hr =
            GetServiceSource()->GetService(
                __uuidof(TService),
                reinterpret_cast<void**>(
                    result.Put()
                )
            );

        if(FAILED(hr))
            throw hr;

        return result;
    }
}
