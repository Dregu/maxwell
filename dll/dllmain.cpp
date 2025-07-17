#define UNICODE

#include <Windows.h>

#include <TlHelp32.h>
#include <chrono>
#include <fmt/format.h>
#include <fstream>
#include <thread>
#include <utility>
#include <vector>

#include "hook.h"
#include "logger.h"
#include "max.h"
#include "search.h"
#include "version.h"
#include "settings.h"
#include "memory.h"

using namespace std::chrono_literals;

struct ProcessInfo {
  std::string name;
  DWORD pid;
};

struct Process {
  HANDLE handle;
  ProcessInfo info;
};

std::vector<ProcessInfo> get_processes() {
  // No unicode
#undef Process32First
#undef Process32Next
#undef PROCESSENTRY32
  std::vector<ProcessInfo> res;
  auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == nullptr)
    return {};

  PROCESSENTRY32 ppe = {sizeof(ppe)};
  auto proc = Process32First(snapshot, &ppe);

  while (proc) {
    auto name = ppe.szExeFile;
    if (auto delim = strrchr(name, '\\'))
      name = delim;
    res.push_back({name, ppe.th32ProcessID});
    proc = Process32Next(snapshot, &ppe);
  }
  return res;
}

std::optional<Process> find_process(std::string name) {
  for (auto &proc : get_processes()) {
    if (proc.name == name) {
      return Process{OpenProcess(PROCESS_ALL_ACCESS, 0, proc.pid), proc};
    }
  }
  return {};
}

BOOL WINAPI ctrl_handler(DWORD ctrl_type) {
  switch (ctrl_type) {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT: {
    DEBUG("Console detached, you can now close this window.");
    FreeConsole();
    return TRUE;
  }
  }
  return TRUE;
}

void attach_stdout(DWORD pid) {
  AttachConsole(pid);
  SetConsoleCtrlHandler(ctrl_handler, 1);

  FILE *stream;
  freopen_s(&stream, "CONOUT$", "w", stdout);
  freopen_s(&stream, "CONOUT$", "w", stderr);
  // freopen_s(&stream, "CONIN$", "r", stdin);
  INFO("Do not close this window or the game will also die. Press Ctrl+C to "
       "detach this window from the game process.");
}

DWORD WINAPI AttachThread(LPVOID lParam) {
  // std::this_thread::sleep_for(500ms);
  Process proc;
  if (auto res = find_process("MAXWELL.exe")) {
    proc = res.value();
    attach_stdout(proc.info.pid);
  }

  register_application_version(fmt::format("MAXWELL {}", get_version()));

  if (D3D12::Init() == D3D12::Status::Success) {
    D3D12::InstallHooks(lParam);
  }
  return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH: {
    DisableThreadLibraryCalls(hModule);
    settings.LoadINI();

    // disable steam restart before the game has a chance to get to it
    if(get_address("steam_restart")) {
        write_mem_recoverable("steam_restart", get_address("steam_restart"), get_nop(19), true);
    }

    Max::get();
    CreateThread(nullptr, 0, &AttachThread, static_cast<LPVOID>(hModule), 0, nullptr);
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

auto xinput = LoadLibraryA("c:\\Windows\\system32\\xinput9_1_0.dll");
typedef DWORD(__stdcall *XInputFun)(DWORD, void *);
auto xinput_get_state =
    reinterpret_cast<XInputFun>(GetProcAddress(xinput, "XInputGetState"));
auto xinput_set_state =
    reinterpret_cast<XInputFun>(GetProcAddress(xinput, "XInputSetState"));
extern "C" __declspec(dllexport) DWORD XInputGetState(DWORD a, void *b) {
  return xinput_get_state(a, b);
}
extern "C" __declspec(dllexport) DWORD XInputSetState(DWORD a, void *b) {
  return xinput_set_state(a, b);
}
