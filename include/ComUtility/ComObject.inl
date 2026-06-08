#pragma once

#include <utility>

#include "ComConcepts.hpp"
#include "ComTraits.hpp"
#include "ComObject.hpp"

namespace ComUtility
{
    template<ComInterface T>
    ComObject<T>::ComObject(T* p) noexcept
        : ptr(p)
    {
    }

    template<ComInterface T>
    ComObject<T>::~ComObject()
    {
        Reset();
    }

    template<ComInterface T>
    ComObject<T>::ComObject(
        ComObject&& other
    ) noexcept
        : ptr(
            std::exchange(
                other.ptr,
                nullptr
            )
        )
    {
    }

    template<ComInterface T>
    ComObject<T>&
    ComObject<T>::operator=(
        ComObject&& other
    ) noexcept
    {
        if(this != &other)
        {
            Reset();

            ptr =
                std::exchange(
                    other.ptr,
                    nullptr
                );
        }

        return *this;
    }

    template<ComInterface T>
    void ComObject<T>::Reset() noexcept
    {
        if(ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

    template<ComInterface T>
    T* ComObject<T>::Get() const noexcept
    {
        return ptr;
    }

    template<ComInterface T>
    T** ComObject<T>::Put() noexcept
    {
        Reset();
        return &ptr;
    }

    template<ComInterface T>
    void ComObject<T>::Attach(
        T* p
    ) noexcept
    {
        Reset();
        ptr = p;
    }

    template<ComInterface T>
    T* ComObject<T>::Detach() noexcept
    {
        return std::exchange(
            ptr,
            nullptr
        );
    }

    template<ComInterface T>
    T* ComObject<T>::operator->()
    const noexcept
    {
        return ptr;
    }

    template<ComInterface T>
    T& ComObject<T>::operator*()
    const noexcept
    {
        return *ptr;
    }

    template<ComInterface T>
    ComObject<T>::operator bool()
    const noexcept
    {
        return ptr != nullptr;
    }

    template<ComInterface T>
    template<ComInterface U>
    ComObject<U>
    ComObject<T>::As() const
    {
        ComObject<U> result;

        HRESULT hr =
            ptr->QueryInterface(
                __uuidof(U),
                reinterpret_cast<void**>(
                    result.Put()
                )
            );

        if(FAILED(hr))
            throw hr;

        return result;
    }

    template<ComInterface T>
    template<ComInterface U>
    requires HasActivate<T>
    ComObject<U>
    ComObject<T>::Activate(
        DWORD clsctx
    ) const
    {
        ComObject<U> result;

        HRESULT hr =
            ptr->Activate(
                __uuidof(U),
                clsctx,
                nullptr,
                reinterpret_cast<void**>(
                    result.Put()
                )
            );

        if(FAILED(hr))
            throw hr;

        return result;
    }
}