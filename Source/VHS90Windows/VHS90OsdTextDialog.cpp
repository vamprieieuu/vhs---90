#include "VHS90OsdTextDialog.h"
#include "VHS90TextEditorResource.h"
#include <windows.h>
#include <string>

namespace {
struct DialogState {
    std::wstring initial;
    std::string updated;
    bool accepted=false;
};

bool utf8ToWide(const std::string& input,std::wstring& output) noexcept {
    output.clear();
    if(input.empty()) return true;
    if(input.size()>4096) return false;
    const int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,input.data(),
        static_cast<int>(input.size()),nullptr,0);
    if(count<=0) return false;
    try { output.resize(static_cast<std::size_t>(count)); }
    catch(...) { return false; }
    return MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,input.data(),
        static_cast<int>(input.size()),output.data(),count)==count;
}

bool wideToUtf8(const std::wstring& input,std::string& output) noexcept {
    output.clear();
    if(input.empty()) return true;
    const int count=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,input.data(),
        static_cast<int>(input.size()),nullptr,0,nullptr,nullptr);
    if(count<=0 || count>4096) return false;
    try { output.resize(static_cast<std::size_t>(count)); }
    catch(...) { return false; }
    return WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,input.data(),
        static_cast<int>(input.size()),output.data(),count,nullptr,nullptr)==count;
}

INT_PTR CALLBACK textDialogProc(HWND window,UINT message,WPARAM wParam,LPARAM lParam) noexcept {
    auto* state=reinterpret_cast<DialogState*>(GetWindowLongPtrW(window,DWLP_USER));
    switch(message) {
        case WM_INITDIALOG:
            state=reinterpret_cast<DialogState*>(lParam);
            SetWindowLongPtrW(window,DWLP_USER,reinterpret_cast<LONG_PTR>(state));
            if(state) {
                SetDlgItemTextW(window,IDC_VHS90_OSD_TEXT_EDIT,state->initial.c_str());
                SendDlgItemMessageW(window,IDC_VHS90_OSD_TEXT_EDIT,EM_SETLIMITTEXT,4096,0);
                SetFocus(GetDlgItem(window,IDC_VHS90_OSD_TEXT_EDIT));
            }
            return FALSE;
        case WM_COMMAND:
            if(LOWORD(wParam)==IDOK && state) {
                const int length=GetWindowTextLengthW(GetDlgItem(window,IDC_VHS90_OSD_TEXT_EDIT));
                if(length<0 || length>4096) {
                    MessageBoxW(window,L"Text is too long. Maximum is 4096 UTF-8 bytes.",L"VHS-90",MB_OK|MB_ICONWARNING);
                    return TRUE;
                }
                std::wstring wide;
                try { wide.resize(static_cast<std::size_t>(length)+1); }
                catch(...) { return TRUE; }
                GetDlgItemTextW(window,IDC_VHS90_OSD_TEXT_EDIT,wide.data(),length+1);
                wide.resize(static_cast<std::size_t>(length));
                if(!wideToUtf8(wide,state->updated)) {
                    MessageBoxW(window,L"Text is invalid or exceeds 4096 UTF-8 bytes.",L"VHS-90",MB_OK|MB_ICONWARNING);
                    return TRUE;
                }
                state->accepted=true;
                EndDialog(window,IDOK);
                return TRUE;
            }
            if(LOWORD(wParam)==IDCANCEL) { EndDialog(window,IDCANCEL); return TRUE; }
            break;
        case WM_CLOSE:
            EndDialog(window,IDCANCEL);
            return TRUE;
        default: break;
    }
    return FALSE;
}
}

bool VHS90ShowOsdTextDialog(const std::string& initial_utf8,std::string& updated_utf8) noexcept {
    try {
        DialogState state;
        if(!utf8ToWide(initial_utf8,state.initial)) return false;
        HMODULE module=nullptr;
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&VHS90ShowOsdTextDialog),&module) || !module) return false;
        HWND parent=GetActiveWindow();
        if(!parent) parent=GetForegroundWindow();
        const INT_PTR result=DialogBoxParamW(module,MAKEINTRESOURCEW(IDD_VHS90_OSD_TEXT_EDITOR),
            parent,textDialogProc,reinterpret_cast<LPARAM>(&state));
        if(result!=IDOK || !state.accepted) return false;
        updated_utf8=std::move(state.updated);
        return true;
    } catch(...) { return false; }
}
