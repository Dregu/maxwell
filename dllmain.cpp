#include <Windows.h>
#include <fstream>
#include <utility>

#include "hook.h"
#include "logger.h"
#include "version.h"

DWORD WINAPI CreateConsole() {
  AllocConsole();
  AttachConsole(GetCurrentProcessId());
  freopen_s(reinterpret_cast<FILE **>(stdin), "CONIN$", "r", stdin);
  freopen_s(reinterpret_cast<FILE **>(stdout), "CONOUT$", "w", stdout);
  freopen_s(reinterpret_cast<FILE **>(stderr), "CONOUT$", "w", stderr);
  return 0;
}

DWORD WINAPI AttachThread(LPVOID lParam) {
  if (D3D12::Init() == D3D12::Status::Success) {
    D3D12::InstallHooks();
  }
  return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH: {
#ifndef NDEBUG
    CreateConsole();
#endif
    DisableThreadLibraryCalls(hModule);
    CreateThread(nullptr, 0, &AttachThread, static_cast<LPVOID>(hModule), 0,
                 nullptr);
    break;
  }
  case DLL_PROCESS_DETACH: {
    D3D12::RemoveHooks();
    break;
  }
  }
  return TRUE;
}

extern "C" __declspec(dllexport) const char *dll_version() {
  return get_version_cstr();
}
