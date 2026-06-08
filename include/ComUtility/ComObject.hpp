#pragma once

#include "ComConcepts.hpp"
#include "ComMixins.hpp"

namespace ComUtility
{
    template<ComInterface T>
    class ComObject :
        public ServiceBase<T>
    {
    private:

        T* ptr = nullptr;

    public:

        constexpr ComObject() noexcept = default;

        explicit ComObject(T* p) noexcept;

        ~ComObject();

        ComObject(const ComObject&) = delete;
        ComObject& operator=(const ComObject&) = delete;

        ComObject(ComObject&& other) noexcept;

        ComObject&
        operator=(ComObject&& other) noexcept;

    public:

        void Reset() noexcept;

        T* Get() const noexcept;

        T** Put() noexcept;

        void Attach(T* p) noexcept;

        T* Detach() noexcept;

    public:

        T* operator->() const noexcept;

        T& operator*() const noexcept;

        explicit operator bool() const noexcept;

    public:

        template<ComInterface U>
        ComObject<U> As() const;

        template<ComInterface U>
        requires HasActivate<T>
        ComObject<U> Activate(
            DWORD clsctx = CLSCTX_ALL
        ) const;
    };
}

#include "ComObject.inl"