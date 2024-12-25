#ifndef SWAL_HINSTANCE_H
#define SWAL_HINSTANCE_H

#include "error.h"
#include "win_headers.h"

namespace swal {

inline auto GetLocalInstance() -> HINSTANCE
{
    static struct InstanceHolder {
        InstanceHolder()
        {
            swal::winapi_call(::GetModuleHandleEx(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCTSTR>(this),
                &instance
            ));
        }
        HMODULE instance;
    } localInst;
    return localInst.instance;
}

}

#endif // SWAL_HINSTANCE_H
