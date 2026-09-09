#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

static HWND profileList;
static HWND profileName;
static HWND statusText;
static fs::path appDir;
static fs::path profilesDir;

static std::wstring quote(const fs::path& value) {
    return L"\"" + value.wstring() + L"\"";
}

static void setStatus(const std::wstring& text) {
    SetWindowTextW(statusText, text.c_str());
}

static fs::path executableDirectory() {
    std::vector<wchar_t> buffer(32768);
    DWORD count = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    return fs::path(std::wstring(buffer.data(), count)).parent_path();
}

static std::wstring env(const wchar_t* key) {
    DWORD size = GetEnvironmentVariableW(key, nullptr, 0);
    if (!size) return L"";
    std::vector<wchar_t> value(size);
    GetEnvironmentVariableW(key, value.data(), size);
    return value.data();
}

static fs::path findBrowser() {
    const std::vector<fs::path> candidates = {
        appDir / L"browser" / L"chrome.exe",
        appDir / L"browser" / L"chromium.exe",
        appDir / L"chrome.exe",
        appDir / L"chromium.exe",
        fs::path(env(L"PROGRAMFILES")) / L"Google" / L"Chrome" / L"Application" / L"chrome.exe",
        fs::path(env(L"PROGRAMFILES(X86)")) / L"Google" / L"Chrome" / L"Application" / L"chrome.exe",
        fs::path(env(L"LOCALAPPDATA")) / L"Google" / L"Chrome" / L"Application" / L"chrome.exe",
        fs::path(env(L"PROGRAMFILES(X86)")) / L"Microsoft" / L"Edge" / L"Application" / L"msedge.exe"
    };
    for (const auto& candidate : candidates) {
        if (!candidate.empty() && fs::exists(candidate)) return candidate;
    }
    return {};
}

static std::wstring cleanProfileName(std::wstring name) {
    const std::wstring forbidden = L"<>:\"/\\|?*";
    name.erase(std::remove_if(name.begin(), name.end(), [&](wchar_t value) {
        return value < 32 || forbidden.find(value) != std::wstring::npos;
    }), name.end());
    while (!name.empty() && (name.back() == L'.' || name.back() == L' ')) name.pop_back();
    while (!name.empty() && name.front() == L' ') name.erase(name.begin());
    return name;
}

static void refreshProfiles() {
    SendMessageW(profileList, LB_RESETCONTENT, 0, 0);
    fs::create_directories(profilesDir);
    std::vector<std::wstring> names;
    for (const auto& item : fs::directory_iterator(profilesDir)) {
        if (item.is_directory()) names.push_back(item.path().filename().wstring());
    }
    std::sort(names.begin(), names.end());
    for (const auto& name : names) SendMessageW(profileList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name.c_str()));
    setStatus(names.empty() ? L"Crea il primo profilo." : L"Seleziona un profilo oppure usa Incognito.");
}

static std::wstring selectedProfile() {
    LRESULT selected = SendMessageW(profileList, LB_GETCURSEL, 0, 0);
    if (selected == LB_ERR) return L"";
    wchar_t value[260]{};
    SendMessageW(profileList, LB_GETTEXT, selected, reinterpret_cast<LPARAM>(value));
    return value;
}

static bool launchBrowser(const fs::path& dataDirectory, bool incognito) {
    fs::path browser = findBrowser();
    if (browser.empty()) {
        MessageBoxW(nullptr,
            L"Chrome o Chromium non trovato. Installa il browser nel container oppure copia la versione portatile nella cartella 'browser' accanto a Luminex-Winlator.exe.",
            L"Luminex Winlator", MB_OK | MB_ICONWARNING);
        return false;
    }
    fs::create_directories(dataDirectory);
    std::wstring command = quote(browser) + L" --user-data-dir=" + quote(dataDirectory)
        + L" --no-first-run --new-window https://www.google.com";
    if (incognito) command += L" --incognito";
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');
    STARTUPINFOW startup{sizeof(startup)};
    PROCESS_INFORMATION process{};
    BOOL started = CreateProcessW(browser.c_str(), mutableCommand.data(), nullptr, nullptr, FALSE, 0,
                                  nullptr, appDir.c_str(), &startup, &process);
    if (!started) {
        MessageBoxW(nullptr, L"Il browser non è stato avviato. Controlla l'installazione nel container Winlator.",
                    L"Luminex Winlator", MB_OK | MB_ICONERROR);
        return false;
    }
    CloseHandle(process.hThread);
    if (incognito) {
        HANDLE handle = process.hProcess;
        fs::path cleanupTarget = dataDirectory;
        std::thread([handle, cleanupTarget]() {
            WaitForSingleObject(handle, INFINITE);
            CloseHandle(handle);
            std::error_code error;
            fs::remove_all(cleanupTarget, error);
        }).detach();
    } else {
        CloseHandle(process.hProcess);
    }
    return true;
}

static void createProfile() {
    wchar_t value[260]{};
    GetWindowTextW(profileName, value, 260);
    std::wstring name = cleanProfileName(value);
    if (name.empty()) {
        MessageBoxW(nullptr, L"Scrivi un nome per il profilo, per esempio Personale o Associazione.",
                    L"Luminex Winlator", MB_OK | MB_ICONINFORMATION);
        return;
    }
    fs::path target = profilesDir / name;
    if (fs::exists(target)) {
        MessageBoxW(nullptr, L"Esiste già un profilo con questo nome.", L"Luminex Winlator", MB_OK | MB_ICONWARNING);
        return;
    }
    fs::create_directories(target);
    SetWindowTextW(profileName, L"");
    refreshProfiles();
    setStatus(L"Profilo creato: " + name);
}

static void openProfile() {
    std::wstring name = selectedProfile();
    if (name.empty()) {
        MessageBoxW(nullptr, L"Seleziona prima un profilo.", L"Luminex Winlator", MB_OK | MB_ICONINFORMATION);
        return;
    }
    if (launchBrowser(profilesDir / name, false)) setStatus(L"Profilo aperto: " + name);
}

static void openIncognito() {
    auto stamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    fs::path temporary = appDir / L"incognito" / std::to_wstring(stamp);
    if (launchBrowser(temporary, true)) setStatus(L"Sessione incognito avviata.");
}

static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            HWND title = CreateWindowW(L"STATIC", L"Luminex Winlator", WS_CHILD | WS_VISIBLE,
                                       20, 18, 420, 30, window, nullptr, nullptr, nullptr);
            SendMessageW(title, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            CreateWindowW(L"STATIC", L"Profili separati per account Gmail diversi", WS_CHILD | WS_VISIBLE,
                          20, 50, 440, 24, window, nullptr, nullptr, nullptr);
            profileList = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
                                        20, 82, 440, 180, window, reinterpret_cast<HMENU>(100), nullptr, nullptr);
            profileName = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                        20, 278, 290, 30, window, reinterpret_cast<HMENU>(101), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Crea profilo", WS_CHILD | WS_VISIBLE,
                          320, 276, 140, 34, window, reinterpret_cast<HMENU>(102), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Apri profilo", WS_CHILD | WS_VISIBLE,
                          20, 326, 210, 42, window, reinterpret_cast<HMENU>(103), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Nuova finestra incognito", WS_CHILD | WS_VISIBLE,
                          250, 326, 210, 42, window, reinterpret_cast<HMENU>(104), nullptr, nullptr);
            statusText = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE,
                                       20, 386, 440, 42, window, nullptr, nullptr, nullptr);
            for (HWND child = GetWindow(window, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT))
                SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            refreshProfiles();
            return 0;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 102: createProfile(); return 0;
                case 103: openProfile(); return 0;
                case 104: openIncognito(); return 0;
                case 100:
                    if (HIWORD(wParam) == LBN_DBLCLK) openProfile();
                    return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    appDir = executableDirectory();
    profilesDir = appDir / L"profiles";
    fs::create_directories(profilesDir);

    const wchar_t className[] = L"LuminexWinlatorWindow";
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&windowClass);

    HWND window = CreateWindowExW(0, className, L"Luminex Winlator 0.1",
                                  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  500, 490, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}
