/*
 * window.h
 *
 *  Created on: 10 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_WINDOW_H
#define SWAL_WINDOW_H

#include "win_headers.h"
#include <string>
#include "error.h"
#include "enum_bitwise.h"
#include "zero_or_resource.h"
#include "gdi.h"
#include "hinstance.h"

namespace swal {

enum class SetPosFlags {
	NoSize = SWP_NOSIZE,
	NoMove = SWP_NOMOVE,
	NoZOrder = SWP_NOZORDER,
	NoRedraw = SWP_NOREDRAW,
	NoActivate = SWP_NOACTIVATE,
	FrameChanged = SWP_FRAMECHANGED,
	ShowWindow = SWP_SHOWWINDOW,
	HideWindow = SWP_HIDEWINDOW,
	NoCopyBits = SWP_NOCOPYBITS,
	NoOwnerZOrder = SWP_NOOWNERZORDER,
	NoSendChanging = SWP_NOSENDCHANGING,
	DrawFrame = SWP_DRAWFRAME,
	NoReposition = SWP_NOREPOSITION,
	DeferErase = SWP_DEFERERASE,
	AsyncWindowPos = SWP_ASYNCWINDOWPOS,
};

template <> struct enable_enum_bitwise<SetPosFlags> : std::true_type {};

enum class ShowCmd {
	Hide = SW_HIDE,
	ShowNormal = SW_SHOWNORMAL,
	Normal = SW_NORMAL,
	ShowMinimized = SW_SHOWMINIMIZED,
	ShowMaximized = SW_SHOWMAXIMIZED,
	Maximize = SW_MAXIMIZE,
	NoActivate = SW_SHOWNOACTIVATE,
	Show = SW_SHOW,
	Minimize = SW_MINIMIZE,
	ShowMinNoActive = SW_SHOWMINNOACTIVE,
	ShowNA = SW_SHOWNA,
	Restore = SW_RESTORE,
	ShowDefault = SW_SHOWDEFAULT,
	ForceMinimize = SW_FORCEMINIMIZE,
};

class Wnd : public zero_or_resource<HWND> {
public:
	Wnd(HWND hWnd) : zero_or_resource(hWnd) {}
	LONG_PTR GetLongPtr(int index) const {
		SetLastError(ERROR_SUCCESS);
		return winapi_call(GetWindowLongPtr(*this, index), GetWindowLongPtr_error_check);
	}
	LONG_PTR SetLongPtr(int index, LONG_PTR value) const {
		SetLastError(ERROR_SUCCESS);
		return winapi_call(SetWindowLongPtr(*this, index, value), GetWindowLongPtr_error_check);
	}
	LONG_PTR GetClassLongPtr(int index) const {
		SetLastError(ERROR_SUCCESS);
		return winapi_call(::GetClassLongPtr(*this, index), GetWindowLongPtr_error_check);
	}
	LONG_PTR SetClassLongPtr(int index, LONG_PTR value) const {
		SetLastError(ERROR_SUCCESS);
		return winapi_call(::SetClassLongPtr(*this, index, value), GetWindowLongPtr_error_check);
	}
	void SetPos(const Wnd& wndAfter, int x, int y, int cx, int cy, SetPosFlags flags) const {
		winapi_call(SetWindowPos(*this, wndAfter, x, y, cx, cy, static_cast<UINT>(flags)));
	}
	RECT GetRect() const {
		RECT rc;
		winapi_call(GetWindowRect(*this, &rc));
		return rc;
	}
	RECT GetClientRect() const {
		RECT rc;
		winapi_call(::GetClientRect(*this, &rc));
		return rc;
	}
	bool Show(ShowCmd cmd) const {
		return ShowWindow(*this, static_cast<int>(cmd));
	}
	void InvalidateRect(bool erase = true) const {
		winapi_call(::InvalidateRect(*this, nullptr, erase));
	}
	void InvalidateRect(const RECT& rect, bool erase = true) const {
		winapi_call(::InvalidateRect(*this, &rect, erase));
	}
	void ValidateRect() const {
		winapi_call(::ValidateRect(*this, nullptr));
	}
	void ValidateRect(const RECT& rect) const {
		winapi_call(::ValidateRect(*this, &rect));
	}
	bool IsVisible() const {
		return IsWindowVisible(*this);
	}
	PaintDC BeginPaint() const {
		return { *this };
	}
	WindowDC GetDC() const {
		return { *this };
	}
	WindowDC GetDC(HRGN clip, DWORD flags) const {
		return { *this, clip, flags };
	}
	void UpdateWindow() const {
		winapi_call(::UpdateWindow(*this));
	}
};

class Window : public Wnd {
public:
    Window() : Wnd(NULL) {}
    Window(DWORD exStyle, LPCTSTR cls, LPCTSTR wndName, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE hInstance, void* param) :
		Wnd(winapi_call(CreateWindowEx(exStyle, reinterpret_cast<LPCTSTR>(cls), wndName, style, x, y, width, height, parent, menu, hInstance, param)))
	{}
	Window(DWORD exStyle, LPCTSTR cls, const tstring& wndName, DWORD style, int x, int y, int width, int height, const Wnd& parent, HMENU menu, HINSTANCE hInstance, void* param) :
		Window(exStyle, cls, wndName.c_str(), style, x, y, width, height, HWND(parent), menu, hInstance, param)
	{}
	Window(DWORD exStyle, LPCTSTR cls, DWORD style, int x, int y, int width, int height, const Wnd& parent, HMENU menu, HINSTANCE hInstance, void* param) :
		Window(exStyle, cls, nullptr, style, x, y, width, height, HWND(parent), menu, hInstance, param)
	{}
	Window(LPCTSTR cls, HINSTANCE hInstance, void* param = nullptr) :
		Window(0, cls, nullptr, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, param)
	{}
    ~Window() { if (*this != NULL) { DestroyWindow(*this); } }
	Window(Window&&) = default;
	Window& operator=(Window&&) = default;
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
};

template <typename Cls, LRESULT(Cls::*mth)(HWND, UINT, WPARAM, LPARAM) = nullptr, int clsPtrIdx = GWLP_USERDATA>
LRESULT CALLBACK ClsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
	Cls* obj;
	Wnd wnd(hWnd);
	if (message == WM_NCCREATE) {
		auto cr = reinterpret_cast<CREATESTRUCT*>(lParam);
		obj = static_cast<Cls*>(cr->lpCreateParams);
		wnd.SetLongPtr(clsPtrIdx, reinterpret_cast<LONG_PTR>(obj));
	} else {
		obj = reinterpret_cast<Cls*>(wnd.GetLongPtr(clsPtrIdx));
	}
	if (obj) {
		if constexpr (mth != nullptr) {
			return (obj->*mth)(hWnd, message, wParam, lParam);
		} else {
			return (*obj)(hWnd, message, wParam, lParam);
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

struct WindowClass
{
	WindowClass(const WNDCLASSEX& wcex) :
		className(wcex.lpszClassName)
	{
		winapi_call(::RegisterClassEx(&wcex));
	}
	WindowClass(const WNDCLASS &wc) :
		className(wc.lpszClassName)
	{
		winapi_call(::RegisterClass(&wc));
	}
	WindowClass(WindowClass&& oth) :
		className(std::exchange(oth.className, nullptr))
	{}
	WindowClass(const WindowClass&) = delete;
	WindowClass& operator=(WindowClass&& oth)
	{
		using std::swap;
		swap(className, oth.className);
		return *this;
	}
	WindowClass& operator=(const WindowClass&) = delete;
	~WindowClass() {
		Unregister();
	}
	void Unregister() noexcept
	{
		if (className == nullptr) {
			return;
		}
		::UnregisterClass(className, GetLocalInstance());
		className = nullptr;
	}
	auto ClassName() -> const TCHAR*
	{
		return className;
	}
	operator const TCHAR*()
	{
		return className;
	}
private:
	const TCHAR* className;
};

template <typename T>
struct basic_window_class : WindowClass
{
	basic_window_class() :
		WindowClass(fill_class_info())
	{}
private:
	auto fill_class_info() -> WNDCLASSEX
	{
		WNDCLASSEX wcex;
		static_cast<T*>(this)->fill_class_info(wcex);
		return wcex;
	}
};

template <typename Cls, LRESULT(Cls::*mth)(HWND, UINT, WPARAM, LPARAM) = nullptr>
struct auto_window_class : basic_window_class<auto_window_class<Cls, mth>>
{
	void fill_class_info(WNDCLASSEX& wcex)
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = window_proc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = GetLocalInstance();
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = class_name().c_str();
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	}
private:
    static auto conv_class_name(const char* clsName) -> tstring
    {
        std::string_view name{clsName};
        auto size = name.size();
        tstring result(size, TCHAR(0));
        for (decltype(size) i = 0; i < size; ++i) {
            if (0 < name[i] && name[i] < 128) {
                result[i] = name[i];
            } else {
                result[i] = TEXT('?');
            }
        }
        return result;
    }
    static auto window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
    -> LRESULT
    {
        Cls* obj;
        Wnd wnd(hWnd);
        if (message == WM_NCCREATE) {
            auto cr = reinterpret_cast<CREATESTRUCT*>(lParam);
            obj = static_cast<Cls*>(cr->lpCreateParams);
            wnd.SetLongPtr(GwlpThis, reinterpret_cast<LONG_PTR>(obj));
        } else {
            obj = reinterpret_cast<Cls*>(wnd.GetLongPtr(GwlpThis));
        }
        if (obj) {
            if constexpr (mth != nullptr) {
                return (obj->*mth)(hWnd, message, wParam, lParam);
            }
            if constexpr (requires {{obj->window_proc(hWnd, message, wParam, lParam)} -> std::same_as<LRESULT>;}) {
                return obj->window_proc(hWnd, message, wParam, lParam);
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    static auto class_name() ->  const tstring&
    {
        static const tstring className = conv_class_name(typeid(Cls).name());
        return className;
    }
    enum Constants {
        GwlpThis = 0
    };

    std::atomic_int refs = 0;
};

}

#endif /* SWAL_WINDOW_H */
