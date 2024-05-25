#pragma once

#include <Windows.h>
#include <cstdint>

namespace D3D12 {

enum class Status {
  UnknownError = -1,
  NotSupportedError = -2,
  ModuleNotFoundError = -3,

  AlreadyInitializedError = -4,
  NotInitializedError = -5,

  Success = 0,
};

Status Init();

Status InstallHooks();
Status RemoveHooks();

extern DWORD PID;
extern HWND Window;

} // namespace D3D12
