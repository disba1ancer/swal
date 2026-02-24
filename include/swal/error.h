/*
 * win32_error.h
 *
 *  Created on: 28 авг. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_ERROR_H
#define SWAL_ERROR_H

#include "win_headers.h"
//#include <wchar.h>
#include <exception>
#include <string>
#include <memory>
#include <type_traits>
#include <system_error>
#include "strconv.h"

namespace swal {
enum class win32_errc : DWORD {};
enum class com_errc : HRESULT {};
}
namespace std {
template <> struct is_error_code_enum<swal::win32_errc> : true_type {};
template <> struct is_error_code_enum<swal::com_errc> : true_type {};
}

namespace swal {

inline tstring get_error_string(DWORD error) {
	constexpr std::size_t resultStringMaxSize = 512;
	TCHAR wStr[resultStringMaxSize];
	auto wSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, error, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), wStr, resultStringMaxSize, nullptr);
	return tstring(wStr, wSize);
}

class win32_category : public std::error_category {
	win32_category() = default;
public:
	const char* name() const noexcept override {
		return "Win32 error";
	}
	std::string message(int condition) const override {
		return swal::fromTString(get_error_string(DWORD(condition)));
	}
	static const win32_category& instance() {
		static win32_category instance;
		return instance;
	}
};

class com_category : public std::error_category {
	com_category() = default;
public:
	const char* name() const noexcept override {
		return "COM error";
	}
	std::string message(int condition) const override {
		return swal::fromTString(_com_error(HRESULT(condition)).ErrorMessage());
	}
	static const com_category& instance() {
		static com_category instance;
		return instance;
	}
};

inline std::error_code make_error_code(win32_errc err) {
	return { int(DWORD(err)), win32_category::instance() };
}

inline std::error_code make_error_code(com_errc err) {
	return { int(HRESULT(err)), com_category::instance() };
}

template <typename T>
T winapi_call(T result) {
	if (result) {
        return result;
	}
    auto err = GetLastError();
    if (err == ERROR_SUCCESS) {
        return result;
    }
    throw std::system_error(win32_errc(err));
}

template <typename T, typename F>
auto winapi_call(T result, DWORD(*chk)(F)) -> typename std::remove_reference<F>::type {
	DWORD err = chk(result);
	if (err != ERROR_SUCCESS) {
		throw std::system_error(win32_errc(err));
	}
	return result;
}

template <typename T, typename F>
T winapi_call(T result, const F& chk) {
	DWORD err = chk(result);
	if (err != ERROR_SUCCESS) {
		throw std::system_error(win32_errc(err));
	}
	return result;
}

inline HRESULT com_call(HRESULT result) {
	if (FAILED(result)) {
		throw std::system_error(com_errc(result));
	}
	return result;
}

inline auto last_error() -> std::error_code {
	return win32_errc(GetLastError());
}

inline void throw_last_error() {
	auto err = last_error();
	if (err) {
		throw err;
	}
}

inline DWORD GetMessage_error_check(BOOL result) {
	return ((result != -1) ? ERROR_SUCCESS : GetLastError());
}

inline DWORD wait_func_error_check(DWORD result) {
	return ((result != WAIT_FAILED) ? ERROR_SUCCESS : GetLastError());
}

inline DWORD GetWindowLongPtr_error_check(LONG_PTR result) {
	return (result ? ERROR_SUCCESS : GetLastError());
}

inline DWORD invalid_color_error_check(COLORREF result) {
	return (result != CLR_INVALID ? ERROR_SUCCESS : GetLastError());
}

inline DWORD OverlappedFile_error_check(BOOL& result) {
    DWORD e;
    if (result || (e = GetLastError()) == ERROR_IO_PENDING) {
        return ERROR_SUCCESS;
    }
    return e;
}

inline DWORD CancelIoEx_error_check(BOOL result) {
	if (!result) {
		auto err = GetLastError();
		if (err != ERROR_NOT_FOUND) {
			return err;
		}
	}
	return ERROR_SUCCESS;
}

inline DWORD CreateFile_error_check(HANDLE result) {
	if (result == INVALID_HANDLE_VALUE) {
		return GetLastError();
	}
	return ERROR_SUCCESS;
}

inline DWORD GetDIBits_error_check(int result) {
	if (result == 0) throw 0;
	if (result == ERROR_INVALID_PARAMETER) {
		return result;
	}
	return ERROR_SUCCESS;
}

}

#endif /* SWAL_ERROR_H */
