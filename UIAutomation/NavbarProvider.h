#pragma once

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include "Navbar.h"
#include "BoxProvider.h"
#include <iostream>

class NavbarProvider : public IRawElementProviderSimple, public IRawElementProviderFragmentRoot
{
public:
    NavbarProvider(Navbar* navbar, HWND hwnd) : navbar(navbar), hwnd(hwnd), refCount(1)
    {
        if (!navbar)
        {
            std::cerr << "Navbar is null in constructor!" << std::endl;
        }
        if (!hwnd)
        {
            std::cerr << "HWND is null in constructor!" << std::endl;
        }
    }

    // IUnknown methods
    ULONG STDMETHODCALLTYPE AddRef() { return ++refCount; }
    ULONG STDMETHODCALLTYPE Release()
    {
        if (--refCount == 0)
        {
            delete this;
            return 0;
        }
        return refCount;
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface)
    {
        if (!ppInterface) return E_POINTER;

        if (riid == __uuidof(IUnknown) || riid == __uuidof(IRawElementProviderSimple) || riid == __uuidof(IRawElementProviderFragment) || riid == __uuidof(IRawElementProviderFragmentRoot))
        {
            *ppInterface = static_cast<IRawElementProviderSimple*>(this);
            AddRef();
            return S_OK;
        }
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    // IRawElementProviderSimple methods
    HRESULT STDMETHODCALLTYPE get_ProviderOptions(ProviderOptions* pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = ProviderOptions_ServerSideProvider;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID iid, IUnknown** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        pRetVal->vt = VT_EMPTY;
        if (idProp == UIA_ControlTypePropertyId)
        {
            pRetVal->vt = VT_I4;
            pRetVal->lVal = UIA_PaneControlTypeId;
        }
        else if (idProp == UIA_NamePropertyId)
        {
            pRetVal->vt = VT_BSTR;
            pRetVal->bstrVal = SysAllocString(L"Navbar");
        }
        else if (idProp == UIA_BoundingRectanglePropertyId)
        {
            RECT rect = navbar->GetRect();
            UiaRect uiaRect = { (double)rect.left, (double)rect.top, (double)(rect.right - rect.left), (double)(rect.bottom - rect.top) };
            pRetVal->vt = VT_R8 | VT_ARRAY;
            SAFEARRAY* psa = SafeArrayCreateVector(VT_R8, 0, 4);

            if (psa != NULL)
            {
                LONG index = 0;
                HRESULT hr = SafeArrayPutElement(psa, &index, &uiaRect.left);
                if (SUCCEEDED(hr))
                {
                    index = 1;
                    hr = SafeArrayPutElement(psa, &index, &uiaRect.top);
                    if (SUCCEEDED(hr))
                    {
                        index = 2;
                        hr = SafeArrayPutElement(psa, &index, &uiaRect.width);
                        if (SUCCEEDED(hr))
                        {
                            index = 3;
                            hr = SafeArrayPutElement(psa, &index, &uiaRect.height);
                            if (SUCCEEDED(hr))
                            {
                                pRetVal->parray = psa;
                                return S_OK;
                            }
                        }
                    }
                }

                // Cleanup on failure
                SafeArrayDestroy(psa);
            }

            // Return appropriate error if creation or element insertion failed
            return E_FAIL;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(IRawElementProviderSimple** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        return UiaHostProviderFromHwnd(hwnd, pRetVal);
    }

    // IRawElementProviderFragment methods
    HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction, IRawElementProviderFragment** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect* pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        RECT rect = navbar->GetRect();
        pRetVal->left = (double)rect.left;
        pRetVal->top = (double)rect.top;
        pRetVal->width = (double)(rect.right - rect.left);
        pRetVal->height = (double)(rect.bottom - rect.top);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetFocus()
    {
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = NULL;
        return S_OK;
    }

    // IRawElementProviderFragmentRoot methods
    HRESULT STDMETHODCALLTYPE ElementProviderFromPoint(double x, double y, IRawElementProviderFragment** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetFocus(IRawElementProviderFragment** pRetVal)
    {
        if (!pRetVal) return E_POINTER;

        *pRetVal = NULL;
        return S_OK;
    }

private:
    Navbar* navbar;
    HWND hwnd;
    ULONG refCount;
};
