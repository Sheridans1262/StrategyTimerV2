#pragma once
#include <Windows.h>
#include <string>
#include <atomic>
#include <set>
#include <thread>


class Timer
{
private:
    struct Settings
    {
        const std::string SETTINGS_SECTION = "SETTINGS";
        std::string m_ProcessesFileName;
        std::string m_SoundAlertFileName;
        unsigned int m_TimeAfterAlert;
    };

    const std::string SETTINGS_INI_FILE = "StrategyTimer.ini";
    Settings m_Settings;

    std::set<std::string> m_ProcessNameList;

    std::thread timerThread;
    std::atomic<bool> m_IsWaiting;
public:
    Timer();
    ~Timer();

    bool LoadSettings();
    bool LoadProcessesFromFile();
    void SaveProcessesToFile();

    void StartTimer(unsigned int minutes);
    void WaitSeconds(unsigned int seconds);
    void PlaySoundAlert() const;
    void KillProcesses() const;
};
