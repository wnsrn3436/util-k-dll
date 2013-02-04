#include "export.h"
#include <windows.h>

HWND hWnd='\0';
HIMC hIMC='\0';
HHOOK hHook='\0';
HINSTANCE hHookDll='\0';
wchar_t char_uni[3]={0,};
char char_ansi[3]={0,}, char_utf8[5]={0,}, *encode_char='\0';

extern "C" BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    switch(reason)
    {
        case DLL_PROCESS_ATTACH:
            {
                if(hHookDll=='\0')
                {
                    hHookDll=hInst;
                }
                break;
            }
        case DLL_PROCESS_DETACH:
            {
                if(hHook!='\0')
                {
                    UnhookWindowsHookEx(hHook);
                    hHook='\0';
                }
                if(encode_char!='\0')
                {
                    delete [] encode_char;
                    encode_char='\0';
                }
                break;
            }
        case DLL_THREAD_ATTACH:
            {
                break;
            }
        case DLL_THREAD_DETACH:
            {
                break;
            }
        default: ;
    }
    return 1;
}

LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
    MSG *msg=(MSG*)lParam;
    switch(msg->message)
    {
        case WM_IME_COMPOSITION:
            {
                if(hIMC=ImmGetContext(hWnd))
                {
                    tagRECT tagRect;
                    GetWindowRect(hWnd, &tagRect);
                    COMPOSITIONFORM cf;
                    cf.dwStyle=CFS_POINT;
                    cf.ptCurrentPos.x=tagRect.left-1000;
                    cf.ptCurrentPos.y=tagRect.top-1000;
                    ImmSetCompositionWindow(hIMC, &cf);
                    ImmReleaseContext(hWnd, hIMC);
                }
                break;
            }
        default: ;
    }
    return CallNextHookEx(hHook, code, wParam, lParam);
}

export double kdll_init()
{
    ULONG idProc;
    DWORD TempCurrProcId=GetCurrentProcessId();
    hWnd=FindWindow(NULL, NULL);
    
    while(hWnd!=NULL)
    {
        if(GetParent(hWnd)==NULL)
        {
            GetWindowThreadProcessId(hWnd, &idProc);
            if(TempCurrProcId==idProc){break;}
        }
        hWnd=GetWindow(hWnd, GW_HWNDNEXT);
    }
    
    return 0;
}

export char* ime_get_string_ansi()
{
    char_ansi[0]='\0';
    
    if(hIMC=ImmGetContext(hWnd))
    {
        ImmGetCompositionStringA(hIMC, GCS_COMPSTR, char_ansi, ImmGetCompositionStringA(hIMC, GCS_COMPSTR, NULL, 0));
        ImmReleaseContext(hWnd, hIMC);
    }
    
    return char_ansi;
}

export char* ime_get_string_utf8()
{
    char_uni[0]='\0';
    char_utf8[0]='\0';
    
    if(hIMC=ImmGetContext(hWnd))
    {
        ImmGetCompositionStringW(hIMC, GCS_COMPSTR, char_uni, ImmGetCompositionStringW(hIMC, GCS_COMPSTR, NULL, 0));
        int Ulen=lstrlenW(char_uni)+1;
        WideCharToMultiByte(CP_UTF8, 0, char_uni, Ulen, char_utf8, WideCharToMultiByte(CP_UTF8, 0, char_uni, Ulen, NULL, 0, NULL, NULL), NULL, NULL);
        ImmReleaseContext(hWnd, hIMC);
    }
    
    return char_utf8;
}

export double ime_set_language(double mode)
{
    if(hIMC=ImmGetContext(hWnd))
    {
        ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        
        DWORD dwConv, dwSent;
        BOOL bRet=ImmGetConversionStatus(hIMC, &dwConv, &dwSent);
        if(bRet)
        {
            ImmSetConversionStatus(hIMC, mode, dwSent);
        }
        ImmReleaseContext(hWnd, hIMC);
    }
    
    return 0;
}

export double ime_get_language()
{
    BOOL bRet=0;
    
    if(hIMC=ImmGetContext(hWnd))
    {
        bRet=ImmGetOpenStatus(hIMC);
        ImmReleaseContext(hWnd, hIMC);
    }
    
    return bRet;
}

export double ime_set_composition_window(double show)
{
    if(hIMC=ImmGetContext(hWnd))
    {
        COMPOSITIONFORM cf;
        if(show)
        {
            if(hHook!='\0')
            {
                UnhookWindowsHookEx(hHook);
                hHook='\0';
            }
            cf.dwStyle=CFS_DEFAULT;
        }
        else
        {
            if(hHook=='\0')
            {
                hHook=SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hHookDll, GetCurrentThreadId());
            }
            tagRECT tagRect;
            GetWindowRect(hWnd, &tagRect);
            cf.dwStyle=CFS_POINT;
            cf.ptCurrentPos.x=tagRect.left-1000;
            cf.ptCurrentPos.y=tagRect.top-1000;
        }
        ImmSetCompositionWindow(hIMC, &cf);
        ImmReleaseContext(hWnd, hIMC);
    }
    
    return 0;
}

export char* encoding_ansi_to_utf8(char *string)
{
    if(encode_char!='\0'){delete [] encode_char; encode_char='\0';}
    
    int Alen=strlen(string)+1, Ulen=MultiByteToWideChar(CP_ACP, 0, string, Alen, NULL, 0);
    wchar_t *stringW=new wchar_t[Ulen];
    MultiByteToWideChar(CP_ACP, 0, string, Alen, stringW, Ulen);
    //ansi to uni
    
    int Tlen=WideCharToMultiByte(CP_UTF8, 0, stringW, Ulen, NULL, 0, NULL, NULL);
    encode_char=new char[Tlen];
    WideCharToMultiByte(CP_UTF8, 0, stringW, Ulen, encode_char, Tlen, NULL, NULL);
    //uni to utf-8
    
    delete [] stringW;
    return encode_char;
}

export char* encoding_utf8_to_ansi(char *string)
{
    if(encode_char!='\0'){delete [] encode_char; encode_char='\0';}
    
    int Tlen=strlen(string)+1, Ulen=MultiByteToWideChar(CP_UTF8, 0, string, Tlen, NULL, 0);
    wchar_t *stringW=new wchar_t[Ulen];
    MultiByteToWideChar(CP_UTF8, 0, string, Tlen, stringW, Ulen);
    //utf-8 to uni
    
    int Alen=WideCharToMultiByte(CP_ACP, 0, stringW, Ulen, NULL, 0, NULL, NULL);
    encode_char=new char[Alen];
    WideCharToMultiByte(CP_ACP, 0, stringW, Ulen, encode_char, Alen, NULL, NULL);
    //uni to ansi
    
    delete [] stringW;
    return encode_char;
}

export double ansi_ord_at(unsigned char *string, double index)
{
    return (int)string[(int)index-1];
}
