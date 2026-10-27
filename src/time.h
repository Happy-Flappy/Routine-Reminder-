using namespace std::chrono;



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