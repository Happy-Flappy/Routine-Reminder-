//-std=c++20 standard
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

#define WINSIMPLE_IMPL
#include "winsimple.hpp"
#define WINSIMPLE_CONTROLS_IMPL
#include "winsimple-controls.hpp"

#include "time.h"
struct Reminder
{
	bool done = false;
	int currentTone = 0;
	std::string timeFrame = "DAILY";
	std::string action = "";
};
float reminderRate = 30;

#include "saveload.h"
#include "actor.h"


int main()
{
	//tones
	std::vector<std::pair<std::string,std::string>> tones = {
		{"Did you ", " yet?"},
		{"Why didn't you ", " yet?"},
		{"Hey! You better ", " soon!"},
		{"Do you want me to destroy your PC!??? ", " OR I WILL FRY THIS PC!"},
		{"Ok...Ok..Maybe I overreacted a little. ", " OR ELSE"},
		{"WAAAAAAAaaaaaaaaaaaah~ Why didn't you ", " yet?"},
		{"Pleeeeaaase! Oh Pleeease ", " for me?"}
	};
	//
	
	std::vector<Reminder> reminders;
	ws::Vec2i actorSize = {0,0};
	loadState(reminders,actorSize);
	
	
	std::chrono::duration<int> reminderTime;
	if(reminderRate < 1.0f)
		reminderTime = seconds(static_cast<int>(reminderRate * 60));
	else	
		reminderTime = minutes(static_cast<int>(reminderRate));
	
	Actor actor("ASSETS/pikachu.png",actorSize);
	
	bool firstQuestion = true;
	actor.question("Hi there! Have you set your routine list yet?",0);
	
	int current = 0;
	std::function resetOldReminders = [&](){
		std::string today = get_today_date();
		std::string week = get_week_start_date();
		if(today != storedDate || week != storedWeekStart)
		{
			for(auto& r : reminders) 
			{
				if(r.timeFrame == "DAILY" && today != storedDate)
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
		actor.update(reminders,current);
	
		if(ws::Global::getButton(ws::Key::Escape) && ws::Global::getButton(ws::Key::Control))
		{
			saveState(reminders,storedDate,storedWeekStart);
			exit(0);
		}
		
		if(actor.wasAwake)
		{
			actor.wasAwake = false;
			last_reminder_time = steady_clock::now();
			
			if(firstQuestion)
			{
				firstQuestion = false;
				if(!actor.gotYes)
				{
					std::string folder = "";
					ws::ResolveRelativePath(folder);
					std::wstring wfolder = ws::toUTF16(folder);
					
					std::string file = "source.txt";
					std::wstring wfile = ws::toUTF16(file);
					
					HINSTANCE result = ShellExecute(NULL,L"open",wfile.c_str(),NULL,wfolder.c_str(),SW_SHOWNORMAL);
					
					if((INT_PTR)result <= 32)
						MessageBoxA(NULL,"Failed to open tasks source file called (sources.txt)","Routine Reminder",MB_OK | MB_ICONWARNING);				
					
				}
				continue;
			}
			if(actor.gotYes)
			{
				if(!reminders.empty() && current < (int)reminders.size() && !reminders[current].done)
				{
					reminders[current].done = true;
					saveState(reminders, storedDate, storedWeekStart);
					current = (current + 1) % reminders.size();
				}
				continue;				
			}				
			
		}
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

			if(!actor.sleeping)
				continue;

			std::string msg = tones[r.currentTone].first + r.action + tones[r.currentTone].second;
			msg += "\nIs it Done?";
			actor.question(msg,r.currentTone);

			r.currentTone = (r.currentTone+1) % tones.size();
		}
		// reset timer
		last_reminder_time = now;
	}
	saveState(reminders,storedDate,storedWeekStart);
}

