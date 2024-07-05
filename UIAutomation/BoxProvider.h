#pragma once

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include "Box.h"

class BoxProvider : public IRawElementProviderSimple, public IRawElementProviderFragment
{
public:
    BoxProvider(Box* box, HWND hwnd) : box(box), hwnd(hwnd), refCount(1) {}

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
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IRawElementProviderSimple) || riid == __uuidof(IRawElementProviderFragment))
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
        *pRetVal = ProviderOptions_ServerSideProvider;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID iid, IUnknown** pRetVal)
    {
        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal)
    {
        pRetVal->vt = VT_EMPTY;
        if (idProp == UIA_ControlTypePropertyId)
        {
            pRetVal->vt = VT_I4;
            pRetVal->lVal = UIA_ButtonControlTypeId;
        }
        else if (idProp == UIA_NamePropertyId)
        {
            pRetVal->vt = VT_BSTR;
            pRetVal->bstrVal = SysAllocString(box->GetText().c_str());
        }
        else if (idProp == UIA_BoundingRectanglePropertyId)
        {
            RECT rect = box->GetRect();
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
                return hr; // Return the error code if any SafeArrayPutElement failed
            }

            return E_FAIL; // Return appropriate error if SafeArrayCreateVector failed
        }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(IRawElementProviderSimple** pRetVal)
    {
        return UiaHostProviderFromHwnd(hwnd, pRetVal);
    }

    // IRawElementProviderFragment methods
    HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction, IRawElementProviderFragment** pRetVal)
    {
        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY** pRetVal)
    {
        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect* pRetVal)
    {
        RECT rect = box->GetRect();
        pRetVal->left = (double)rect.left;
        pRetVal->top = (double)rect.top;
        pRetVal->width = (double)(rect.right - rect.left);
        pRetVal->height = (double)(rect.bottom - rect.top);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal)
    {
        *pRetVal = NULL;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetFocus()
    {
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal)
    {
        *pRetVal = NULL;
        return S_OK;
    }

private:
    Box* box;
    HWND hwnd;
    ULONG refCount;
};
