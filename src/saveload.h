
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

bool loadState(std::vector<Reminder> &reminders,ws::Vec2i &actorSize)
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

		std::string command = line.substr(0, spacePos);
		std::string action = line.substr(spacePos + 1);

		if(command == "ACTORSIZE")
		{
			std::istringstream iss(action);
			int first = 0,last = 0;
			if(!(iss >> first)) continue;
			if(!(iss >> last)) last = first;
			if(first <= 0 || last <= 0)
			{
				MessageBoxA(NULL,"Invalid actor size given to Routine Reminder. \nUse something like (ACTORSIZE 250 250) without closing parenthesis.\n Quitting...","Routine Reminder",MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			actorSize = {first,last};
			continue;
		}
		if(command == "DELAY")
		{
			std::istringstream iss(action);
			float val = 0;
			
			if(!(iss >> val) || val <= 0)
			{
				MessageBoxA(NULL,"Invalid reminder delay given to Routine Reminder. \nUse something like (DELAY 30) without closing parenthesis.\n Quitting...","Routine Reminder",MB_OK | MB_ICONINFORMATION);
				exit(0);
			}
			reminderRate = val;
			continue;			
		}
		if(action.size() >= 2 && action.front() == '"' && action.back() == '"')
			action = action.substr(1, action.size() - 2);
		
		sourceData.push_back({false, 0, command, action});
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