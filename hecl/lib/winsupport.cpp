#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include "HECL/winsupport.hpp"

/*
* The memmem() function finds the start of the first occurrence of the
* substring 'needle' of length 'nlen' in the memory area 'haystack' of
* length 'hlen'.
*
* The return value is a pointer to the beginning of the sub-string, or
* NULL if the substring is not found.
*/
void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen)
{
    int needle_first;
    const uint8_t *p = static_cast<const uint8_t*>(haystack);
    size_t plen = hlen;

    if (!nlen)
        return NULL;

    needle_first = *(unsigned char *)needle;

    while (plen >= nlen && (p = static_cast<const uint8_t*>(memchr(p, needle_first, plen - nlen + 1))))
    {
        if (!memcmp(p, needle, nlen))
            return (void *)p;

        p++;
        plen = hlen - (p - static_cast<const uint8_t*>(haystack));
    }

    return NULL;
}

/* Clearly, MS doesn't require enough headers for this */
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"
#include "strsafe.h"

#define HECL_MAX_PATH 2048

HRESULT CreateShellLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc)
{
    std::wstring targetStr(lpszPathObj);
    for (wchar_t& ch : targetStr)
        if (ch == L'/')
            ch = L'\\';
    std::wstring linkStr(lpszPathLink);
    linkStr += L".lnk";
    for (wchar_t& ch : linkStr)
        if (ch == L'/')
            ch = L'\\';
    
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description. 
        WCHAR targetBuf[HECL_MAX_PATH];
        WCHAR linkBuf[HECL_MAX_PATH];
        WCHAR* linkFinalPart = nullptr;
        GetFullPathNameW(linkStr.c_str(), HECL_MAX_PATH, linkBuf, &linkFinalPart);
        if (linkFinalPart != linkBuf)
            *(linkFinalPart-1) = L'\0';
        StringCbPrintfW(targetBuf, HECL_MAX_PATH, L"%s\\%s", linkBuf, targetStr.c_str());
        if (linkFinalPart != linkBuf)
            *(linkFinalPart - 1) = L'\\';
        psl->SetPath(targetBuf);
        psl->SetRelativePath(linkBuf, 0);
        psl->SetDescription(lpszDesc);

        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(linkBuf, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}

HRESULT ResolveShellLink(LPCWSTR lpszLinkFile, LPWSTR lpszPath, int iPathBufferSize)
{
    HRESULT hres;
    IShellLink* psl;
    WCHAR szGotPath[HECL_MAX_PATH];
    WCHAR szDescription[HECL_MAX_PATH];
    WIN32_FIND_DATA wfd;

    *lpszPath = 0; // Assume failure 

                   // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
                   // has already been called. 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Get a pointer to the IPersistFile interface. 
        hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

        if (SUCCEEDED(hres))
        {
            // Load the shortcut. 
            hres = ppf->Load(lpszLinkFile, STGM_READ);

            if (SUCCEEDED(hres))
            {
                // Resolve the link. 
                HWND hwnd = GetConsoleWindow();
                if (!hwnd)
                    hwnd = GetTopWindow(nullptr);
                hres = psl->Resolve(hwnd, 0);

                if (SUCCEEDED(hres))
                {
                    // Get the path to the link target. 
                    hres = psl->GetPath(szGotPath, HECL_MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);

                    if (SUCCEEDED(hres))
                    {
                        // Get the description of the target. 
                        hres = psl->GetDescription(szDescription, HECL_MAX_PATH);

                        if (SUCCEEDED(hres))
                        {
                            hres = StringCbCopy(lpszPath, iPathBufferSize, szGotPath);
                        }
                    }
                }
            }

            // Release the pointer to the IPersistFile interface. 
            ppf->Release();
        }

        // Release the pointer to the IShellLink interface. 
        psl->Release();
    }
    return hres;
}


