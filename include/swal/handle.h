/*
 * handle.h
 *
 *  Created on: 6 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_HANDLE_H
#define SWAL_HANDLE_H

#include "win_headers.h"
#include <string>
#include "enum_bitwise.h"
#include "error.h"
#include "strconv.h"
#include "zero_or_resource.h"

namespace swal {

class Handle : public zero_or_resource<HANDLE> {
public:
	Handle(HANDLE handle) : zero_or_resource(handle) {}
};

template<typename T>
class OwnableHandle {
public:
	OwnableHandle() = default;
	~OwnableHandle() { CloseHandle(static_cast<T&>(*this)); }
	OwnableHandle(const OwnableHandle&) = delete;
	OwnableHandle& operator=(const OwnableHandle&) = delete;
	OwnableHandle(OwnableHandle&&) noexcept = default;
	OwnableHandle& operator=(OwnableHandle&&) noexcept = default;
};

template<typename T>
class WaitableHandle {
public:
	DWORD WaitFor(DWORD milliseconds = INFINITE) const { return winapi_call(WaitForSingleObject(static_cast<const T&>(*this), milliseconds), wait_func_error_check); }
};

template <typename T>
class EventHandle {
	void Set() const { winapi_call(SetEvent(static_cast<const T&>(*this))); }
	void Reset() const { winapi_call(ResetEvent(static_cast<const T&>(*this))); }
};

#if _WIN32_WINNT >= 0x0600
enum class EventFlags {
	InitialSet = CREATE_EVENT_INITIAL_SET,
	ManualReset = CREATE_EVENT_MANUAL_RESET
};

template <> struct enable_enum_bitwise<enum EventFlags> : std::true_type {};
#endif

class Event : public Handle, public OwnableHandle<Event>, public EventHandle<Event>, public WaitableHandle<Event> {
public:
	Event() noexcept : Handle(NULL) {}
	Event(SECURITY_ATTRIBUTES* sattrs, bool manualReset, bool initialState, tstring name)
		: Handle(winapi_call(CreateEvent(sattrs, manualReset, initialState, name.c_str()))) {}
	Event(bool manualReset, bool initialState)
		: Event(nullptr, manualReset, initialState, tstring()) {}
	Event(SECURITY_ATTRIBUTES& sattrs, bool manualReset, bool initialState)
		: Event(&sattrs, manualReset, initialState, tstring()) {}
	Event(SECURITY_ATTRIBUTES& sattrs, bool manualReset, bool initialState, tstring name)
		: Event(&sattrs, manualReset, initialState, name) {}
#if _WIN32_WINNT >= 0x0600
	Event(SECURITY_ATTRIBUTES* sattrs, tstring name, EventFlags flags, DWORD access)
		: Handle(winapi_call(CreateEventEx(sattrs, name.c_str(), static_cast<DWORD>(flags), access))) {}
	Event(EventFlags flags, DWORD access)
		: Event(nullptr, tstring(), flags, access) {}
	Event(SECURITY_ATTRIBUTES& sattrs, EventFlags flags, DWORD access)
		: Event(&sattrs, tstring(), flags, access) {}
	Event(tstring name, EventFlags flags, DWORD access)
		: Event(nullptr, name, flags, access) {}
	Event(SECURITY_ATTRIBUTES& sattrs, tstring name, EventFlags flags, DWORD access)
		: Event(&sattrs, name, flags, access) {}
#endif
};

enum class SetPointerModes {
	Begin = FILE_BEGIN,
	Current = FILE_CURRENT,
	End = FILE_END,
};

template <typename T>
class FileOps {
private:
	const Handle& handle() const {
		return static_cast<const T&>(*this);
	}
public:
    BOOL Read(LPVOID buffer, DWORD size, DWORD* bytesRead, OVERLAPPED* ovl) const
    {
        return winapi_call(
            ReadFile(handle(), buffer, size, bytesRead, ovl),
            OverlappedFile_error_check
        );
    }
    DWORD Read(LPVOID buffer, DWORD size) const
    {
		DWORD result;
        Read(buffer, size, &result, nullptr);
		return result;
	}
    bool Read(LPVOID buffer, DWORD size, OVERLAPPED& ovl) const
    {
        return Read(buffer, size, nullptr, &ovl);
	}
    bool Read(LPVOID buffer, DWORD size, DWORD& bytesRead, OVERLAPPED& ovl) const
    {
        return Read(buffer, size, &bytesRead, &ovl);
    }
    void GetOverlappedResult(OVERLAPPED* ovl, DWORD* result, BOOL wait) const
    {
        winapi_call(::GetOverlappedResult(handle(), ovl, result, wait));
    }
    auto GetOverlappedResult(OVERLAPPED& ovl) const -> DWORD
    {
        DWORD result = 0;
        GetOverlappedResult(&ovl, &result, TRUE);
		return result;
    }
    void GetOverlappedResult(OVERLAPPED& ovl, DWORD& transferred, bool wait = false) const
    {
        GetOverlappedResult(&ovl, &transferred, wait);
    }
    BOOL Write(LPCVOID buffer, DWORD size, DWORD* bytesWritten, OVERLAPPED* ovl) const
    {
        return winapi_call(
            WriteFile(handle(), buffer, size, bytesWritten, ovl),
            OverlappedFile_error_check
        );
    }
    DWORD Write(LPCVOID buffer, DWORD size) const
    {
		DWORD result;
        Write(buffer, size, &result, nullptr);
		return result;
	}
    bool Write(LPCVOID buffer, DWORD size, OVERLAPPED& ovl) const
    {
        return Write(buffer, size, nullptr, &ovl);
	}
    bool Write(LPCVOID buffer, DWORD size, DWORD& bytesWritten, OVERLAPPED& ovl) const
    {
        return Write(buffer, size, &bytesWritten, &ovl);
    }
	LARGE_INTEGER SetPointerEx(LARGE_INTEGER dist, SetPointerModes mode = SetPointerModes::Begin) const {
		LARGE_INTEGER result;
		winapi_call(SetFilePointerEx(handle(), dist, &result, static_cast<DWORD>(mode)));
		return result;
	}
	void SetEndOfFile() const {
		winapi_call(::SetEndOfFile(handle()));
	}
	void CancelIo() const {
		winapi_call(::CancelIo(handle()), CancelIoEx_error_check);
	}
#if _WIN32_WINNT >= 0x0600
	void CancelIoEx() const {
		winapi_call(::CancelIoEx(handle(), nullptr), CancelIoEx_error_check);
	}
	void CancelIoEx(OVERLAPPED& ovl) const {
		winapi_call(::CancelIoEx(handle(), &ovl), CancelIoEx_error_check);
	}
#endif
	LARGE_INTEGER GetSizeEx() const {
		LARGE_INTEGER result;
		winapi_call(GetFileSizeEx(handle(), &result));
		return result;
	}
};

enum class ShareMode {
	Zero = 0,
	Exclusive = 0,
	Read = FILE_SHARE_READ,
	Write = FILE_SHARE_WRITE,
	Delete = FILE_SHARE_DELETE
};

template <> struct enable_enum_bitwise<enum ShareMode> : std::true_type {};

enum class CreateMode {
	CreateNew = CREATE_NEW,
	CreateAlways = CREATE_ALWAYS,
	OpenExisting = OPEN_EXISTING,
	OpenAlways = OPEN_ALWAYS,
	TruncateExisting = TRUNCATE_EXISTING
};

class FileHandle : public Handle, public FileOps<FileHandle> {
public:
	FileHandle(HANDLE handle) : Handle(handle) {}
};

class File : public FileHandle, public OwnableHandle<File>, public WaitableHandle<File> {
public:
	File() noexcept : FileHandle(INVALID_HANDLE_VALUE) {}
	File(const tstring& filename, DWORD access, ShareMode shareMode, SECURITY_ATTRIBUTES* secattrs, CreateMode createMode, DWORD flags, HANDLE tmplt)
        : FileHandle(winapi_call(CreateFile(filename.c_str(), access, static_cast<DWORD>(shareMode), secattrs, static_cast<DWORD>(createMode), flags, tmplt), CreateFile_error_check)) {}
    File(const tstring& filename, DWORD access, ShareMode shareMode, SECURITY_ATTRIBUTES& secattrs, CreateMode createMode, DWORD flags, const Handle& tmplt)
        : File(filename, access, shareMode, &secattrs, createMode, flags, tmplt) {}
    File(const tstring& filename, DWORD access, ShareMode shareMode, SECURITY_ATTRIBUTES& secattrs, CreateMode createMode, DWORD flags)
        : File(filename, access, shareMode, &secattrs, createMode, flags, NULL) {}
    File(const tstring& filename, DWORD access, ShareMode shareMode, CreateMode createMode, DWORD flags, const Handle& tmplt)
        : File(filename, access, shareMode, nullptr, createMode, flags, tmplt) {}
    File(const tstring& filename, DWORD access, ShareMode shareMode, CreateMode createMode, DWORD flags)
        : File(filename, access, shareMode, nullptr, createMode, flags, NULL) {}
};

struct CompletionStatusResult {
	DWORD error;
	DWORD bytesTransfered;
	ULONG_PTR key;
	OVERLAPPED* ovl;
};

template <typename T>
class IOCompletionPortHandle {
private:
	const Handle& handle() const {
		return static_cast<const T&>(*this);
	}
public:
	void AssocFile(const Handle& file, ULONG_PTR key) const {
		winapi_call(CreateIoCompletionPort(file, handle(), key, 0));
	}
	CompletionStatusResult GetQueuedCompletionStatus(DWORD timeout) const {
		CompletionStatusResult result;
		result.error = ERROR_SUCCESS;
		if (!::GetQueuedCompletionStatus(handle(), &result.bytesTransfered, &result.key, &result.ovl, timeout)) {
			result.error = GetLastError();
			if (result.ovl == nullptr && result.error != WAIT_TIMEOUT) {
				throw std::system_error(make_error_code(win32_errc(result.error)));
			}
		}
		return result;
	}
    CompletionStatusResult GetQueuedCompletionStatus2(DWORD timeout) const noexcept {
		CompletionStatusResult result;
		result.error = ERROR_SUCCESS;
		if (!::GetQueuedCompletionStatus(handle(), &result.bytesTransfered, &result.key, &result.ovl, timeout)) {
			result.error = GetLastError();
		}
		return result;
	}
	void PostQueuedCompletionStatus(DWORD transfered, ULONG_PTR key, OVERLAPPED* ovl) const {
		winapi_call(::PostQueuedCompletionStatus(handle(), transfered, key, ovl));
	}
};

class IOCompletionPort : public Handle, public IOCompletionPortHandle<IOCompletionPort>, public OwnableHandle<IOCompletionPort> {
public:
	IOCompletionPort(DWORD thrNum = 0)
		: Handle(winapi_call(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thrNum))) {}
	IOCompletionPort(const Handle& file, ULONG_PTR key, DWORD thrNum = 0)
		: Handle(winapi_call(CreateIoCompletionPort(file, NULL, key, thrNum))) {}
};

}

#endif /* SWAL_HANDLE_H */
