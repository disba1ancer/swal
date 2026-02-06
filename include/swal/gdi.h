/*
 * gdi.h
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_GDI_H
#define SWAL_GDI_H

#include "win_headers.h"
#include "error.h"
#include "zero_or_resource.h"
#include "enum_bitwise.h"

namespace swal {

class GdiObj : public zero_or_resource<HGDIOBJ> {
public:
	GdiObj(HGDIOBJ obj) : zero_or_resource(obj) {}
	~GdiObj() {
		DeleteObject(*this);
	}
	GdiObj(const GdiObj&) = delete;
	GdiObj& operator=(const GdiObj&) = delete;
	GdiObj(GdiObj&&) = default;
	GdiObj& operator=(GdiObj&&) = default;
};

enum class PenStyle {
	Solid = PS_SOLID,
	Dash = PS_DASH,
	Dot = PS_DOT,
	DashDot = PS_DASHDOT,
	DashDotDot = PS_DASHDOTDOT,
	Null = PS_NULL,
	InsideFrame = PS_INSIDEFRAME
};

inline constexpr COLORREF Rgb(int r, int g, int b) { return RGB(r, g, b); }

class Pen : public GdiObj {
public:
	Pen(int style, int width, COLORREF color) :
		GdiObj(winapi_call(CreatePen(style, width, color))) {}
	Pen(PenStyle style, int width, COLORREF color) :
		Pen(static_cast<int>(style), width, color) {}
	Pen(PenStyle style, COLORREF color) :
		Pen(static_cast<int>(style), 1, color) {}
	Pen(int width, COLORREF color) :
		Pen(PenStyle::Solid, width, color) {}
	Pen(COLORREF color) :
		Pen(PenStyle::Solid, 1, color) {}
};

class DC : public zero_or_resource<HDC> {
public:
	DC(HDC hdc) : zero_or_resource(hdc) {}
	HGDIOBJ SelectObject(HGDIOBJ obj) const { return winapi_call(::SelectObject(get(), obj)); }
	void MoveTo(int x, int y) const { winapi_call(::MoveToEx(get(), x, y, nullptr)); }
	void MoveToEx(int x, int y, POINT* pt) const {
		winapi_call(::MoveToEx(get(), x, y, pt));
	}
	POINT MoveToEx(int x, int y) const {
		POINT pt;
		MoveToEx(x, y, &pt);
		return pt;
	}
	void LineTo(int x, int y) const { winapi_call(::LineTo(get(), x, y)); }
	COLORREF SetPenColor(COLORREF color) const { return winapi_call(::SetDCPenColor(get(), color), invalid_color_error_check); }
	COLORREF SetPixel(int x, int y, COLORREF color) const { return winapi_call(::SetPixel(get(), x, y, color), invalid_color_error_check); }
    void FillRect(const RECT* rc, HBRUSH brush) const { winapi_call(::FillRect(get(), rc, brush)); }
    void FillRect(const RECT& rc, HBRUSH brush) const { FillRect(&rc, brush); }
	int GetCaps(int index) const { return ::GetDeviceCaps(*this, index); }
};

class PaintDC : private PAINTSTRUCT, public DC {
public:
	PaintDC(HWND hWnd) : DC(winapi_call(::BeginPaint(hWnd, this))), hWnd(hWnd) {}
	~PaintDC() { EndPaint(hWnd, this); }
	PaintDC(const PaintDC&) = delete;
	PaintDC& operator=(const PaintDC&) = delete;
	PaintDC(PaintDC&&) = default;
	PaintDC& operator=(PaintDC&&) = default;
	const PAINTSTRUCT* operator ->() const { return this; }
private:
	HWND hWnd;
};

enum class GetDCExFlags {
	Window = DCX_WINDOW,
	Cache = DCX_CACHE,
	NoResetAttrs = DCX_NORESETATTRS,
	ClipChildren = DCX_CLIPCHILDREN,
	ClipSiblings = DCX_CLIPSIBLINGS,
	ParentClip = DCX_PARENTCLIP,
	ExcludeRGN = DCX_EXCLUDERGN,
	IntersectRGN = DCX_INTERSECTRGN,
	ExcludeUpdate = DCX_EXCLUDEUPDATE,
	IntersectUpdate = DCX_INTERSECTUPDATE,
	LockWindowUpdate = DCX_LOCKWINDOWUPDATE
};

template <> struct enable_enum_bitwise<GetDCExFlags> : std::true_type {};

class WindowDC : public DC {
public:
	WindowDC(HWND hWnd) : DC(winapi_call(GetDC(hWnd))), hWnd(hWnd) {}
	WindowDC(HWND hWnd, HRGN clip, DWORD flags) : DC(winapi_call(GetDCEx(hWnd, clip, flags))), hWnd(hWnd) {}
	WindowDC(HWND hWnd, HRGN clip, GetDCExFlags flags) : WindowDC(hWnd, clip, DWORD(flags)) {}
	~WindowDC() { ReleaseDC(hWnd, *this); }
	WindowDC(const WindowDC&) = delete;
	WindowDC& operator=(const WindowDC&) = delete;
	WindowDC(WindowDC&&) = default;
	WindowDC& operator=(WindowDC&&) = default;
private:
	HWND hWnd;
};

}

#endif /* SWAL_GDI_H */
