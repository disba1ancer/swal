#ifndef SWAL_REG_H
#define SWAL_REG_H

#include "error.h"
#include "zero_or_resource.h"
#include "enum_bitwise.h"
#include "strconv.h"

namespace swal {

class RegistryKey;

inline DWORD RegOpenKeyEx_error_check(LSTATUS status) {
    return DWORD(status);
}

class RegKeyHandle : public zero_or_resource<HKEY> {
public:
    constexpr RegKeyHandle(HKEY hKey) : zero_or_resource(hKey)
    {}
    inline RegistryKey CreateKey(
        LPCTSTR name,
        LPTSTR cls,
        DWORD options,
        REGSAM sam,
        const LPSECURITY_ATTRIBUTES,
        DWORD* disposition
    ) const;
    inline RegistryKey CreateKey(
        const tstring& name,
        REGSAM sam
    ) const;
    inline RegistryKey OpenKey(LPCTSTR name, UINT options, REGSAM sam) const;
    inline RegistryKey OpenKey(const tstring& name, REGSAM sam) const;
    void GetValue(LPCTSTR name, LPCTSTR valName, DWORD flags, DWORD* type, void* data, DWORD* size) const
    {
        winapi_call(
            RegGetValue(*this, name, valName, flags, type, data, size),
            RegOpenKeyEx_error_check
        );
    }
    DWORD GetDWORD(const tstring& valName) const
    {
        DWORD result;
        DWORD size = sizeof(result);
        GetValue(nullptr, valName.c_str(), RRF_RT_REG_DWORD, nullptr, &result, &size);
        return result;
    }
    void QueryValue(LPCTSTR valName, DWORD* type, BYTE* data, DWORD* size) const
    {
        winapi_call(
            RegQueryValueEx(*this, valName, 0, type, data, size),
            RegOpenKeyEx_error_check
        );
    }
    DWORD QueryDWORD(const tstring& valName) const
    {
        DWORD type;
        DWORD result;
        DWORD size = sizeof(result);
        QueryValue(valName.c_str(), &type, reinterpret_cast<BYTE*>(&result), &size);
        if (type != REG_DWORD) {
            winapi_call(0, [](DWORD){ return ERROR_DATATYPE_MISMATCH; });
        }
        return result;
    }
    void SetValue(LPCTSTR valName, DWORD type, const BYTE* data, DWORD size) const
    {
        winapi_call(
            RegSetValueEx(*this, valName, 0, type, data, size),
            RegOpenKeyEx_error_check
        );
    }
    void SetDWORD(const tstring& valName, DWORD value) const
    {
        SetValue(valName.c_str(), REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
    }
    void SetString(const tstring& valName, const tstring& value)
    {
        SetValue(valName.c_str(), REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), DWORD(value.size() * sizeof(TCHAR)));
    }
    void DeleteValue(LPCTSTR valName)
    {
        winapi_call(RegDeleteValue(*this, valName), RegOpenKeyEx_error_check);
    }
    void DeleteValue(const tstring& valName)
    {
        DeleteValue(valName.c_str());
    }
};

template <typename T>
class OwnableRegKeyHandle {
public:
	OwnableRegKeyHandle() = default;
	~OwnableRegKeyHandle() { RegCloseKey(static_cast<T&>(*this)); }
	OwnableRegKeyHandle(const OwnableRegKeyHandle&) = delete;
	OwnableRegKeyHandle& operator=(const OwnableRegKeyHandle&) = delete;
	OwnableRegKeyHandle(OwnableRegKeyHandle&&) noexcept = default;
	OwnableRegKeyHandle& operator=(OwnableRegKeyHandle&&) noexcept = default;
};

template <HKEY hKey>
class RegRootKey : public RegKeyHandle {
	RegRootKey() : RegKeyHandle(hKey) {}
};

class RegistryKey : public RegKeyHandle, public OwnableRegKeyHandle<RegistryKey> {
	RegistryKey(HKEY hKey) : RegKeyHandle(hKey)
	{}
	friend class RegKeyHandle;
};

inline RegistryKey RegKeyHandle::CreateKey(LPCTSTR name, LPTSTR cls, DWORD options, REGSAM sam, const LPSECURITY_ATTRIBUTES secAttrs, DWORD* disposition) const
{
	HKEY hKeyResult;
	winapi_call(RegCreateKeyEx(*this, name, 0, cls, options, sam, secAttrs, &hKeyResult, disposition), RegOpenKeyEx_error_check);
	return { hKeyResult };
}

inline RegistryKey RegKeyHandle::CreateKey(const tstring& name, REGSAM sam) const
{
	return CreateKey(name.c_str(), nullptr, 0, sam, nullptr, nullptr);
}

inline RegistryKey RegKeyHandle::OpenKey(LPCTSTR name, UINT options, REGSAM sam) const
{
	HKEY hKeyResult;
	winapi_call(RegOpenKeyEx(*this, name, options, sam, &hKeyResult), RegOpenKeyEx_error_check);
	return { hKeyResult };
}

inline RegistryKey RegKeyHandle::OpenKey(const tstring& name, REGSAM sam) const
{
	return OpenKey(name.c_str(), 0, sam);
	RegCreateKeyEx(NULL, nullptr, 0, nullptr, 0, 0, nullptr, nullptr, nullptr);
}

#define GENERATE(name, key) inline auto name() { return RegKeyHandle(key); }

GENERATE(RegKey_ClassesRoot, HKEY_CLASSES_ROOT);
GENERATE(RegKey_CurrentConfig, HKEY_CURRENT_CONFIG);
GENERATE(RegKey_CurrentUser, HKEY_CURRENT_USER);
GENERATE(RegKey_LocalMachine, HKEY_LOCAL_MACHINE);
GENERATE(RegKey_PerformanceData, HKEY_PERFORMANCE_DATA);
GENERATE(RegKey_PerformanceNLSText, HKEY_PERFORMANCE_NLSTEXT);
GENERATE(RegKey_PerformanceText, HKEY_PERFORMANCE_TEXT);
GENERATE(RegKey_Users, HKEY_USERS);

#undef GENERATE

} // namespace swal

#endif // SWAL_REG_H
