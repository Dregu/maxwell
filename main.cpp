#include <Windows.h> // for PROCESS_INFORMATION, CloseHandle, GetEnvironm...
#include <chrono>    // for operator<=>, operator-, operator+, operator""s
#include <compare>   // for operator<, operator<=, operator>
#include <conio.h>
#include <ctime> // for errno_t
#include <detours.h>
#include <filesystem> // for exists, path
#include <fstream>    // for basic_ostream, basic_ofstream, ofstream, basi...
#include <iostream>
#include <locale>   // for num_put, num_get
#include <new>      // for operator new
#include <optional> // for optional
#include <sstream>  // for basic_stringstream
#include <stdint.h> // for uint64_t
#include <stdio.h>  // for NULL, fclose, sprintf_s, fflush, fopen_s, fwrite
#include <stdlib.h>
#include <string.h>    // for strlen
#include <string>      // for char_traits, string, basic_string, operator==
#include <string_view> // for string_view
#include <strsafe.h>   // for DWORD
#include <thread>      // for sleep_for
#include <type_traits> // for move
#include <utility>     // for max, min
#include <wininet.h>   // for InternetCloseHandle, InternetOpenA, InternetG...

#include "cmd_line.h" // for GetCmdLineParam, CmdLineParser
#include "injector.h" // for Process, ProcessInfo, call, find_function
#include "logger.h"   // for INFO, PANIC
#include "version.h"  // for get_version

#pragma warning(disable : 4706 4996)

namespace fs = std::filesystem;
using namespace std::chrono_literals;
bool g_console = true; // TODO
std::string g_exe = "Animal Well.exe";

fs::path get_dll_path(const char *rel_path) {
  char buf[0x1000];
  GetModuleFileNameA(NULL, buf, sizeof(buf));
  fs::path path(buf);
  return path.parent_path().concat(rel_path);
}

void wait() {
  if (g_console) {
    while (true) {
      if (auto res = find_process(g_exe))
        std::this_thread::sleep_for(200ms);
      else
        break;
    }
  }
}

bool inject_search(fs::path maxwell_path) {
  SetConsoleTitle("MAXWELL | Start your game or press ENTER to launch game "
                  "from parent directory!");
  INFO("Searching for Animal Well.exe process...");
  INFO("Start your game or press ENTER to launch ../Animal Well.exe!");
  Process proc;
  while (true) {
    if (auto res = find_process("Animal Well.exe")) {
      proc = res.value();
      break;
    }
    if (kbhit()) {
      if (getche() == '\r') {
        return true;
      }
    }
    std::this_thread::sleep_for(200ms);
  }
  SetConsoleTitle("MAXWELL");
  INFO("Found Spel2.exe PID: {}", proc.info.pid);
  if (find_dll_in_process(proc.info.pid, "MAXWELL.dll")) {
    INFO("Already injected, let's not do that again. If you want to inject "
         "multiple game processes, use the --launch_game parameter.");
    return false;
  }
  inject_dll(proc, maxwell_path.string());
  INFO("DLL injected");
  wait();
  return false;
}

bool launch(fs::path exe_path, fs::path maxwell_path, bool &do_inject) {
  auto exe_dir = fs::canonical(exe_path).parent_path();
  auto cwd = fs::current_path();
  g_exe = exe_path.filename().string();

  std::string cmdline{"Animal Well.exe"};

  char dll_path[MAX_PATH] = {};
  sprintf_s(dll_path, MAX_PATH, "%s", maxwell_path.string().c_str());

  const char *dll_paths[] = {
      dll_path,
  };

  if (do_inject)
    INFO("Launching game... {}", exe_path.string());
  else
    INFO("Launching game with DLL... {}", exe_path.string());

  const auto child_env = []() {
    std::string child_env_ = "SteamAppId=813230";

    const auto this_env = GetEnvironmentStrings();
    auto lpszVariable = this_env;
    while (*lpszVariable) {
      child_env_ += '\0';
      child_env_ += lpszVariable;
      lpszVariable += strlen(lpszVariable) + 1;
    }
    FreeEnvironmentStrings(this_env);

    child_env_ += '\0';
    return child_env_;
  }();

  PROCESS_INFORMATION pi{};
  STARTUPINFOA si{};
  si.cb = sizeof(STARTUPINFO);

  if (!do_inject &&
      DetourCreateProcessWithDlls(
          (LPSTR)exe_path.string().c_str(), (LPSTR)cmdline.c_str(), NULL, NULL,
          TRUE, CREATE_DEFAULT_ERROR_MODE, (LPVOID)child_env.c_str(),
          exe_dir.string().c_str(), &si, &pi, 1, dll_paths, NULL)) {
    INFO("Game launched with DLL");
    wait();
    CloseHandle(pi.hThread);
    return true;
  } else if (CreateProcess((LPSTR)exe_path.string().c_str(),
                           (LPSTR)cmdline.c_str(), NULL, NULL, TRUE, 0,
                           (LPVOID)child_env.c_str(), exe_dir.string().c_str(),
                           &si, &pi)) {
    auto proc = Process{pi.hProcess, {g_exe, pi.dwProcessId}};
    INFO("Game launched, injecting DLL...");
    if (find_dll_in_process(proc.info.pid, "MAXWELL.dll")) {
      INFO("Already injected, let's not do that again. If you want to inject "
           "multiple game processes, use the --launch_game parameter.");
      return false;
    }
    inject_dll(proc, maxwell_path.string());
    INFO("DLL injected");
    wait();
    CloseHandle(pi.hThread);
    return false;
  }

  return false;
}

std::string get_dll_version(fs::path maxwell_path) {
  static const HMODULE dll = LoadLibraryEx(maxwell_path.string().c_str(), NULL,
                                           DONT_RESOLVE_DLL_REFERENCES);
  if (!dll)
    return "UNKNOWN";
  typedef const char *(__stdcall * dll_version_fun)();
  dll_version_fun dll_version = nullptr;
  dll_version =
      reinterpret_cast<dll_version_fun>(GetProcAddress(dll, "dll_version"));
  if (!dll_version)
    return "UNKNOWN";
  return std::string((*dll_version)());
}

int main(int argc, char **argv) {
  CmdLineParser cmd_line_parser(argc, argv);

  auto maxwell_path = get_dll_path("\\MAXWELL.dll");
  bool version_info = GetCmdLineParam<bool>(cmd_line_parser, "version", false);
  bool help = GetCmdLineParam<bool>(cmd_line_parser, "help", false);
  if (help) {
    INFO("Usage:");
    INFO("Without --launch_game the launcher will search for a process called "
         "Animal Well.exe and inject when found.");
    INFO("You can press ENTER to stop searching and try to launch the game "
         "from the parent folder.");
    INFO("Command line switches:");
    INFO("  --launch_game [path]    launch ../Animal Well.exe, path/Animal "
         "Well.exe, or a specific exe, and load with Detours");
    INFO("  --inject                use the old injection method instead of "
         "Detours with --launch_game");
    INFO("  --help                  show this helpful help");
    INFO("  --version               show version information");
    return 0;
  }

  std::string version(get_version());
  INFO("MAXWELL EXE version: {}", version);
  if (version_info) {
    if (fs::exists(maxwell_path)) {
      std::string dllversion(get_dll_version(maxwell_path));
      INFO("MAXWELL DLL version: {}", dllversion);
    } else {
      INFO("MAXWELL DLL version: MISSING");
    }
    return 0;
  }

  if (!fs::exists(maxwell_path)) {
    PANIC("DLL not found! {}", maxwell_path.string().data());
  } else {
    std::string dllversion(get_dll_version(maxwell_path));
    INFO("MAXWELL DLL version: {}", dllversion);
  }

  auto launch_game_default =
      GetCmdLineParam<bool>(cmd_line_parser, "launch_game", false);
  auto launch_game =
      GetCmdLineParam<std::string_view>(cmd_line_parser, "launch_game", "");
  if (launch_game.empty() && launch_game_default)
    launch_game = "../Spel2.exe";

  bool do_inject = GetCmdLineParam<bool>(cmd_line_parser, "inject", false);
  fs::path exe;

  if (!launch_game.empty()) {
    auto launch_path = fs::canonical(launch_game);
    if (fs::is_directory(launch_path))
      exe = launch_path / "Animal Well.exe";
    else if (fs::is_regular_file(launch_path))
      exe = launch_path;
  }

  if (fs::exists(exe)) {
    if (launch(exe, maxwell_path, do_inject)) {
      FreeConsole();
      return 0;
    }
  } else {
    if (inject_search(maxwell_path)) {
      launch(fs::canonical("../Animal Well.exe"), maxwell_path, do_inject);
    }
  }
  FreeConsole();
  return 0;
}
