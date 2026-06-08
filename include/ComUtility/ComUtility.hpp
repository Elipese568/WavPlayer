#pragma once

#include <Windows.h>
#include <objbase.h>
#include <combaseapi.h>

#include <concepts>
#include <utility>

#include "ComConcepts.hpp"
#include "ComObject.hpp"

namespace ComUtility
{

    //====================================================
    // Create
    //====================================================

    template<CoCreatable T>
    [[nodiscard]]
    ComObject<T> Create(
        DWORD clsctx = CLSCTX_ALL
    )
    {
        ComObject<T> result;

        HRESULT hr =
            CoCreateInstance(
                ComClassTraits<T>::clsid,
                nullptr,
                clsctx,
                __uuidof(T),
                reinterpret_cast<void**>(result.Put())
            );

        if(FAILED(hr))
            throw hr;

        return result;
    }

    //====================================================
    // Utility
    //====================================================

    inline void CheckHR(HRESULT hr)
    {
        if(FAILED(hr))
            throw hr;
    }

    template<ComInterface T>
    [[nodiscard]]
    constexpr REFIID IIDOf()
    {
        return __uuidof(T);
    }

    class ComInitializeGuard{
        public:
        ComInitializeGuard(){
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        }
        ~ComInitializeGuard(){
            CoUninitialize();
        }
    };
}

#define DEFINE_TRAIT(interface, class) template<> struct ComUtility::ComClassTraits<interface> { static constexpr const CLSID& clsid =  __uuidof(class); };

#define START_COM_GUARD try {
#define FAIL_COM_GUARD(vn) } catch (HRESULT vn) {
#define END_COM_GUARD }