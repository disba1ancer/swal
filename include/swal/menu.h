#ifndef SWAL_MENU_H
#define SWAL_MENU_H

#include "error.h"
#include "zero_or_resource.h"
#include "enum_bitwise.h"

namespace swal {

enum class TrackPopupFlags : UINT {
	LeftButton = TPM_LEFTBUTTON,
	RightButton = TPM_RIGHTBUTTON,
	LeftAling = TPM_LEFTALIGN,
	CenterAlign = TPM_CENTERALIGN,
	RightAlign = TPM_RIGHTALIGN,
#if(WINVER >= 0x0400)
	TopAlign = TPM_TOPALIGN,
	VCenterAlign = TPM_VCENTERALIGN,
	BottomAlign = TPM_BOTTOMALIGN,
	Horizontal = TPM_HORIZONTAL,
	Vertical = TPM_VERTICAL,
	NoNotify = TPM_NONOTIFY,
	ReturnCmd = TPM_RETURNCMD,
#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0500)
	Rescue = TPM_RECURSE,
	HorPosAnimation = TPM_HORPOSANIMATION,
	HorNegAnimation = TPM_HORNEGANIMATION,
	VerPosAnimation = TPM_VERPOSANIMATION,
	VerNegAnimation = TPM_VERNEGANIMATION,
#if(_WIN32_WINNT >= 0x0500)
	NoAnimation = TPM_NOANIMATION,
#endif /* _WIN32_WINNT >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
	LayoutRTL = TPM_LAYOUTRTL,
#endif /* _WIN32_WINNT >= 0x0501 */
#endif /* WINVER >= 0x0500 */
#if(_WIN32_WINNT >= 0x0601)
	WorkArea = TPM_WORKAREA
#endif /* _WIN32_WINNT >= 0x0601 */
};

template <> struct enable_enum_bitwise<enum TrackPopupFlags> : std::true_type {};

class MenuHandle : public zero_or_resource<HMENU> {
public:
	MenuHandle(HMENU menu) : zero_or_resource(menu) {}
	void TrackPopup(TrackPopupFlags flags, int x, int y, HWND hWnd)
	{ winapi_call(TrackPopupMenu(*this, UINT(flags), x, y, 0, hWnd, nullptr)); }
	MenuHandle GetSubMenu(int pos)
	{ return ::GetSubMenu(*this, pos); }
	void GetItemInfo(UINT item, BOOL byPos, MENUITEMINFO* info)
	{ winapi_call(GetMenuItemInfo(*this, item, byPos, info)); }
	void SetItemInfo(UINT item, BOOL byPos, MENUITEMINFO* info)
	{ winapi_call(SetMenuItemInfo(*this, item, byPos, info)); }
};

template<typename T>
class OwnableMenuHandle {
public:
	OwnableMenuHandle() = default;
	~OwnableMenuHandle() { DestroyMenu(static_cast<T&>(*this)); }
	OwnableMenuHandle(const OwnableMenuHandle&) = delete;
	OwnableMenuHandle& operator=(const OwnableMenuHandle&) = delete;
	OwnableMenuHandle(OwnableMenuHandle&&) noexcept = default;
	OwnableMenuHandle& operator=(OwnableMenuHandle&&) noexcept = default;
};

class Menu : public MenuHandle {
public:
    Menu(HINSTANCE hInstance, LPTSTR name) :
        MenuHandle(winapi_call(LoadMenu(hInstance, name)))
    {}
    Menu(HINSTANCE hInstance, long resourceId) :
        Menu(hInstance, MAKEINTRESOURCE(resourceId))
    {}
};

} // namespace swal

#endif // SWAL_MENU_H
