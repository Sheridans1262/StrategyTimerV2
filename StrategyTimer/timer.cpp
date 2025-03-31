#include "timer.h"
#include "logger.h"
#include "wcharUtils.h"

#include <TlHelp32.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <filesystem>

#include <stdlib.h>
#include <chrono>

#pragma comment(lib, "Winmm.lib")
#include <queue>

Timer::Timer()
    : m_IsWaiting(false)
{
    LoadSettings();
    logmsg("Settings loaded");
    std::cout << "Settings loaded" << std::endl;
    LoadProcessesFromFile();
    logmsg("Processes loaded");
    std::cout << "Processes loaded" << std::endl;
}

Timer::~Timer()
{
    SaveProcessesToFile();
    logmsg("Processes saved\n");
    std::cout << "Processes saved" << std::endl;
}

#pragma region File Operations

bool Timer::LoadSettings()
{
    if (!std::filesystem::exists(SETTINGS_INI_FILE))
    {
        throw std::runtime_error(std::format("Settings file {} does not exists.", SETTINGS_INI_FILE));
    }
    logmsg(std::format("{}: Started loading settings.", SETTINGS_INI_FILE));

    wchar_t* wstrSettings = StrToWCHAR(".\\" + SETTINGS_INI_FILE);
    wchar_t* wstrReturnedString = new wchar_t[255];
    
    GetPrivateProfileString(L"SETTINGS", L"ProcessesFileName", L"processesToKill.txt", wstrReturnedString, 255, wstrSettings);
    m_Settings.m_ProcessesFileName = WCHARToStr(wstrReturnedString);
    if (!std::filesystem::exists(m_Settings.m_ProcessesFileName))
    {
        throw std::runtime_error(std::format("Processes file {} does not exists.", m_Settings.m_ProcessesFileName));
    }

    GetPrivateProfileString(L"SETTINGS", L"SoundAlertFileName", L"soundAlert.mp3", wstrReturnedString, 255, wstrSettings);
    m_Settings.m_SoundAlertFileName = WCHARToStr(wstrReturnedString);
    if (!std::filesystem::exists(m_Settings.m_SoundAlertFileName))
    {
        throw std::runtime_error(std::format("Sound alert file {} does not exists.", m_Settings.m_SoundAlertFileName));
    }
    
    m_Settings.m_TimeAfterAlert = GetPrivateProfileInt(L"SETTINGS", L"TimeAfterAlert", 60, wstrSettings);

    logmsg(std::format("{}: Settings loaded sucessfully.", SETTINGS_INI_FILE));

    return true;
}

bool Timer::LoadProcessesFromFile()
{
    if (!std::filesystem::exists(m_Settings.m_ProcessesFileName))
    {
        throw std::runtime_error(std::format("Processes file {} does not exists.", m_Settings.m_ProcessesFileName));
    }
    logmsg(std::format("{}: Started loading processes.", m_Settings.m_ProcessesFileName));

    std::string fileLine;
    std::ifstream fileStream(m_Settings.m_ProcessesFileName);
    if (fileStream.is_open())
    {
        while (getline(fileStream, fileLine))
        {
            this->m_ProcessNameList.insert(fileLine);
        }
        fileStream.close();
    }
    logmsg(std::format("{}: Processes loaded.", m_Settings.m_ProcessesFileName));
    return true;
}

void Timer::SaveProcessesToFile()
{
    logmsg(std::format("{}: Saving processes.", m_Settings.m_ProcessesFileName));
    std::ofstream fileStream(m_Settings.m_ProcessesFileName);
    if (fileStream.is_open())
    {
        for (auto i = this->m_ProcessNameList.begin(); i != m_ProcessNameList.end(); i++)
        {
            fileStream << *i << std::endl;
        }

        fileStream.close();
    }
    logmsg(std::format("{}: Processes saved sucessfully.", m_Settings.m_ProcessesFileName));
}

#pragma endregion

void Timer::StartTimer(unsigned int minutes)
{
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << std::put_time(std::localtime(&time), "%T: "); 
    std::cout << std::format("Timer is set for {} minutes, ends in ", minutes);
    time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + std::chrono::minutes(minutes));
    std::cout << std::put_time(std::localtime(&time), "%T") << std::endl;
    logmsg(std::format("Timer is set for {} minutes", minutes));

    unsigned int seconds = minutes * 60;
    if (seconds > m_Settings.m_TimeAfterAlert)
    {
        timerThread = std::thread(&Timer::WaitSeconds, this, seconds - m_Settings.m_TimeAfterAlert);
        timerThread.detach();
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } while (m_IsWaiting);
    }

    PlaySoundAlert();
    std::cout << std::format("Sound played, {} seconds until process kill", m_Settings.m_TimeAfterAlert) << std::endl;
    logmsg(std::format("Sound played, {} seconds until process kill", m_Settings.m_TimeAfterAlert));

    timerThread = std::thread(&Timer::WaitSeconds, this, m_Settings.m_TimeAfterAlert);
    timerThread.detach();
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (m_IsWaiting);

    KillProcesses();
    std::cout << "Processes killed" << std::endl;
    logmsg("Processes killed");
}

void Timer::WaitSeconds(unsigned int seconds)
{
    logmsg("Timer: Started");

    m_IsWaiting = true;
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    m_IsWaiting = false;

    logmsg("Timer: Ended");
}

void Timer::PlaySoundAlert() const
{
    logmsg("Playing Sound Alert");
    if (!std::filesystem::exists(m_Settings.m_ProcessesFileName))
    {
        logmsg(std::format("Error: Sound alert file {} does not exists. Playing Beeps instead.", m_Settings.m_SoundAlertFileName), LogLevel::ERRORLEVEL);
        Beep(750, 930);
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        Beep(750, 930);
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        Beep(750, 930);
    }
    else
    {
        std::queue<MCIERROR> errQueue;

        // Compose command string, then convert it to WCHAR* (so it would work with WinAPI)
        std::string openSoundAlertCommand = "open \"" + m_Settings.m_SoundAlertFileName + "\" type mpegvideo alias soundalert";
        wchar_t* wstrOpenSoundAlertCommand = StrToWCHAR(openSoundAlertCommand);

        errQueue.push(mciSendString(wstrOpenSoundAlertCommand, NULL, 0, NULL));
        errQueue.push(mciSendString(L"setaudio soundalert volume to 500", NULL, 0, NULL));
        errQueue.push(mciSendString(L"play soundalert wait", NULL, 0, NULL));
        errQueue.push(mciSendString(L"close soundalert", NULL, 0, NULL));

        TCHAR lpszErrorText[256];
        std::string errString = "";
        bool errFlag = false;
        while (!errQueue.empty())
        {
            if (errQueue.front() != 0)
            {
                errFlag = true;
                mciGetErrorString(errQueue.front(), lpszErrorText, 256);
                errString += WCHARToStr(lpszErrorText) + "; ";
            }
            errQueue.pop();
        }
        if (errFlag)
        {
            logmsg(std::format("Errors occured while pragram tried to play sound alert: ", errString));
        }
    }
    logmsg("End of Playing Sound Alert");
}

void Timer::KillProcesses() const
{
    logmsg("Start of Process Killing");
    std::set<std::string> processesToKill = std::set<std::string>(m_ProcessNameList);

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry) == TRUE)
    {
        do 
        {
            for (std::string processName : processesToKill)
            {
                wchar_t* wstrProcessName = StrToWCHAR(processName);
                size_t wcharsNum = std::wcslen(wstrProcessName);
                if (wcsncmp(entry.szExeFile, wstrProcessName, wcharsNum) == 0)
                {
                    // We should not close system processes, thats why we have to check them by IsProcessCritical.
                    // Buuuut, its does not exactly shows critical processes in this case.
                    // Due to insufficient rights OpenProcess cannot open any processes besides ones created by User,
                    // so for processes from SYSTEM, NETWORK SERVICE, LOCAL SERVICE etc. IsProcessCritical returns error, 
                    // hence 0, and for User processes it returns 1. So it actually does the job of filtering system processes
                    // which shouldn't be closed by user, just not in an expected way. Now, if you run programm with
                    // Admin rights, OpenProcess opens most of the processes, except for few very important ones, 
                    // so we additionally checking for !isCrit
                    
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION + PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                    BOOL isCrit = FALSE;
                    if (IsProcessCritical(hProcess, &isCrit) || !isCrit)
                    {
                        TerminateProcess(hProcess, 1);
                        logmsg(std::format("Process name: {}, pid = {} - Kill Confirmed", processName, entry.th32ProcessID));
                    }

                    CloseHandle(hProcess);
                    delete[] wstrProcessName;

                    break;
                }
                delete[] wstrProcessName;
            }
            
        } while (Process32Next(snapshot, &entry) == TRUE);
    }
    CloseHandle(snapshot);
    logmsg("End of Process Killing");
}
