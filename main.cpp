#include <windows.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

//-std=c++20 standard

using namespace std::chrono;


inline std::wstring WIDE(const std::string& str)
{
	int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);
	if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
		size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	std::wstring wstr(size, 0);
	if (size > 0)
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
	return wstr;
}

int safeStoi(std::string text)
{
	int result = 0;
    try {result = std::stoi(text);} 
    catch(...) {} 
	return result;
}

std::string storedDate = "";
std::string storedWeekStart = "";
std::string get_today_date() 
{
    auto now = system_clock::now();
    auto days = floor<std::chrono::days>(now);
    auto tp = system_clock::time_point(days);
    time_t tt = system_clock::to_time_t(tp);
    struct tm tm;
    localtime_s(&tm, &tt); 
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string get_week_start_date()
{
    auto now = system_clock::now();
    auto dayCount = floor<days>(now);
    time_t tt = system_clock::to_time_t(dayCount);
    struct tm tm;
    localtime_s(&tm, &tt);

    // go back to the most recent Sunday
    tm.tm_mday -= tm.tm_wday; 
	mktime(&tm);
	
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

struct Reminder
{
	bool done = false;
	int currentTone = 0;
	std::string timeFrame = "DAILY";
	std::string action = "";
};

bool saveState(const std::vector<Reminder> &reminders,const std::string &date,const std::string &weekStart) 
{
    std::ofstream ofs("reminder_state.txt");
    if(!ofs) return false;
    ofs << date << "\n";
	ofs << weekStart << "\n";
    for(const auto& r : reminders)
    {
		ofs << r.done << "\n";
		ofs << r.currentTone << "\n";
		ofs << r.timeFrame << "\n";
		ofs << r.action << "\n";
	}
    return true;
}

bool loadState(std::vector<Reminder> &reminders)
{
	std::vector<Reminder> sourceData;
	std::vector<Reminder> stateData;
	
	//scoped
	{
	std::ifstream ifs("source.txt");
	if(!ifs) return false;
    std::string line;    
	while(std::getline(ifs, line))
	{
		if(line.empty()) continue;

		size_t spacePos = line.find(' ');
		if(spacePos == std::string::npos) continue; // invalid line

		std::string timeframe = line.substr(0, spacePos);
		std::string action = line.substr(spacePos + 1);

		if(action.size() >= 2 && action.front() == '"' && action.back() == '"')
			action = action.substr(1, action.size() - 2);

		sourceData.push_back({false, 0, timeframe, action});
	}
	
	}
	
	std::ifstream ifs("reminder_state.txt");
	if(!ifs) 
	{
		reminders = sourceData;
		return false;
	}

	std::getline(ifs, storedDate); 
	std::getline(ifs, storedWeekStart); 
	
	
	std::string doneStr, toneStr, timeframe, action;
	while(std::getline(ifs, doneStr) && std::getline(ifs, toneStr) && std::getline(ifs, timeframe) && std::getline(ifs, action))
	{
		Reminder r;
		r.done = (doneStr == "1");
		r.currentTone = safeStoi(toneStr);
		r.timeFrame = timeframe;
		r.action = action;
		stateData.push_back(r);
	}


	

    bool changedSource = false;
    if(sourceData.size() != stateData.size())
        changedSource = true;
    else
    {
        for(int a = 0; a < sourceData.size(); a++)
        {
            if(sourceData[a].timeFrame != stateData[a].timeFrame ||
                sourceData[a].action != stateData[a].action)
            {
                changedSource = true;
                break;
            }
        }
    }
	
	if(!changedSource)
		reminders = stateData;
	else
		reminders = sourceData;
	return true;
}

int main()
{
	//tones
	std::vector<std::pair<std::string,std::string>> tones = {
		{"Did you ", " yet? \n:P"},
		{"Why didn't you ", " yet? \n٩(ʘ.ʘ)۶"},
		{"Hey! You better ", " soon! \n(v •̀_•́)v"},
		{"Are you goldbricking? You should ", "! \n(-_-)"},
		{"If you don't ", " you are going to regret it! \n(¬_¬)"},
		{"Get to work! If you don't ", " you are in BIG TROUBLE! \n(≖_≖)"},
		{"Do you want me to destroy your PC!??? ", " OR I WILL FRY THIS PC! \n(╯°□°)╯"},
		{"Ok...Ok..Maybe I overreacted a little. ", " OR ELSE \n(⩺_⩹)"},
		{"WAAAAAAAaaaaaaaaaaaah~ Why didn't you ", " yet? \n.°՞(っ-/\\-ς)՞°·."},
		{"Pleeeeaaase! Oh Pleeease ", " for me? \n(づ｡◕‿‿◕｡)づ"}
	};
	//
	
	
	std::chrono::duration<int> reminderTime;
	float reminderRate = 30;
	if(reminderRate < 1)
		reminderTime = seconds(static_cast<int>(reminderRate * 60));
	else	
		reminderTime = minutes(static_cast<int>(reminderRate));
	
	std::vector<Reminder> reminders;
	loadState(reminders);
	
	int current = 0;
	std::function resetOldReminders = [&](){
		std::string today = get_today_date();
		std::string week = get_week_start_date();
		if(today != storedDate || week != storedWeekStart)
		{
			for(auto& r : reminders) 
			{
				if(r.timeFrame == "DIALY" && today != storedDate)
				{
					r.done = false;
					r.currentTone = 0;
				}
				if(r.timeFrame == "WEEKLY" && week != storedWeekStart)
				{
					r.done = false;
					r.currentTone = 0;
				}
			}
			storedDate = today;
			storedWeekStart = week;
			saveState(reminders,today,week);
			current = 0;
		}
	};
	auto last_reminder_time = steady_clock::now() - reminderTime;
	while(true)
	{
		if(GetAsyncKeyState(VK_ESCAPE) & 0x8000 && GetAsyncKeyState('Q') & 0x8000)
		{
			if(MessageBoxA(NULL,"Quit Reminder Program?","Are you sure?",MB_YESNO) == IDYES)
				return 0;
		}
		
		std::this_thread::sleep_for(milliseconds(1000));
		
        auto now = steady_clock::now();
        if(now - last_reminder_time < reminderTime) 
			continue;
		
		resetOldReminders();
		
		int count = 0;
		while(count < reminders.size()) 
		{
			if(!reminders[current].done) break;
			current = (current + 1) % reminders.size();
			count++;
		}
		
		if(count < reminders.size()) 
		{
			auto &r = reminders[current];
			
			std::string msg = tones[r.currentTone].first + r.action + tones[r.currentTone].second;
			msg += "\nIs it Done?";
			if(MessageBoxW(NULL, WIDE(msg).c_str(), L"Routine Reminder", MB_YESNO) == IDYES) 
			{
				r.done = true;
				saveState(reminders, storedDate,storedWeekStart);
			}
			r.currentTone = (r.currentTone+1) % tones.size();
			current = (current + 1) % reminders.size();
		}		
		//reset timer
		last_reminder_time = now;
	}
	saveState(reminders,storedDate,storedWeekStart);
}

