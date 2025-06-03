#include "TimeLibrary.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
#include <unordered_map>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

void TimeLibrary::initialize() {
    // Initialize time-related resources if needed
}

Value TimeLibrary::callFunction(const std::string& name, const std::vector<Value>& args) {
    if (name == "now") {
        auto now = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return makeNumber(static_cast<double>(ms));
    }
    
    if (name == "timestamp") {
        auto now = std::chrono::system_clock::now();
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        return makeNumber(static_cast<double>(sec));
    }
    
    if (name == "format") {
        if (args.size() < 1 || args.size() > 2) {
            throw std::runtime_error("format expects 1 or 2 arguments: [timestamp], format_string");
        }
        
        std::time_t time_val;
        std::string format_str;
        
        if (args.size() == 1) {
            time_val = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            format_str = valueToString(args[0]);
        } else {
            time_val = static_cast<std::time_t>(valueToNumber(args[0]));
            format_str = valueToString(args[1]);
        }
        
        std::tm* tm_info = std::localtime(&time_val);
        char buffer[256];
        std::strftime(buffer, sizeof(buffer), format_str.c_str(), tm_info);
        
        return makeString(buffer);
    }
    
    if (name == "diff") {
        if (args.size() != 2) {
            throw std::runtime_error("diff expects 2 arguments: timestamp1, timestamp2");
        }
        
        double time1 = valueToNumber(args[0]);
        double time2 = valueToNumber(args[1]);
        
        return makeNumber(time2 - time1);
    }
    
    if (name == "sleep") {
        if (args.size() != 1) {
            throw std::runtime_error("sleep expects 1 argument: milliseconds");
        }
        
        int ms = static_cast<int>(valueToNumber(args[0]));
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        
        return makeNumber(0.0);
    }
    
    if (name == "date_parts") {
        std::time_t time_val;
        
        if (args.empty()) {
            time_val = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        } else {
            time_val = static_cast<std::time_t>(valueToNumber(args[0]));
        }
        
        std::tm* tm_info = std::localtime(&time_val);
        
        std::unordered_map<std::string, Value> parts;
        parts["year"] = makeNumber(tm_info->tm_year + 1900);
        parts["month"] = makeNumber(tm_info->tm_mon + 1);
        parts["day"] = makeNumber(tm_info->tm_mday);
        parts["hour"] = makeNumber(tm_info->tm_hour);
        parts["minute"] = makeNumber(tm_info->tm_min);
        parts["second"] = makeNumber(tm_info->tm_sec);
        parts["weekday"] = makeNumber(tm_info->tm_wday);
        parts["yearday"] = makeNumber(tm_info->tm_yday + 1);
        
        return makeObject(parts);
    }
    
    if (name == "add") {
        if (args.size() != 3) {
            throw std::runtime_error("add expects 3 arguments: timestamp, amount, unit");
        }
        
        std::time_t time_val = static_cast<std::time_t>(valueToNumber(args[0]));
        int amount = static_cast<int>(valueToNumber(args[1]));
        std::string unit = valueToString(args[2]);
        
        std::tm* tm_info = std::localtime(&time_val);
        
        if (unit == "year" || unit == "years") {
            tm_info->tm_year += amount;
        } else if (unit == "month" || unit == "months") {
            tm_info->tm_mon += amount;
            while (tm_info->tm_mon >= 12) {
                tm_info->tm_mon -= 12;
                tm_info->tm_year++;
            }
        } else if (unit == "day" || unit == "days") {
            tm_info->tm_mday += amount;
        } else if (unit == "hour" || unit == "hours") {
            tm_info->tm_hour += amount;
        } else if (unit == "minute" || unit == "minutes") {
            tm_info->tm_min += amount;
        } else if (unit == "second" || unit == "seconds") {
            tm_info->tm_sec += amount;
        } else {
            throw std::runtime_error("Unknown time unit: " + unit);
        }
        
        std::time_t new_time = std::mktime(tm_info);
        return makeNumber(static_cast<double>(new_time));
    }
    
    if (name == "subtract") {
        if (args.size() != 3) {
            throw std::runtime_error("subtract expects 3 arguments: timestamp, amount, unit");
        }
        
        std::time_t time_val = static_cast<std::time_t>(valueToNumber(args[0]));
        int amount = static_cast<int>(valueToNumber(args[1]));
        std::string unit = valueToString(args[2]);
        
        std::tm* tm_info = std::localtime(&time_val);
        
        if (unit == "year" || unit == "years") {
            tm_info->tm_year -= amount;
        } else if (unit == "month" || unit == "months") {
            tm_info->tm_mon -= amount;
            while (tm_info->tm_mon < 0) {
                tm_info->tm_mon += 12;
                tm_info->tm_year--;
            }
        } else if (unit == "day" || unit == "days") {
            tm_info->tm_mday -= amount;
        } else if (unit == "hour" || unit == "hours") {
            tm_info->tm_hour -= amount;
        } else if (unit == "minute" || unit == "minutes") {
            tm_info->tm_min -= amount;
        } else if (unit == "second" || unit == "seconds") {
            tm_info->tm_sec -= amount;
        } else {
            throw std::runtime_error("Unknown time unit: " + unit);
        }
        
        std::time_t new_time = std::mktime(tm_info);
        return makeNumber(static_cast<double>(new_time));
    }
    
    if (name == "is_leap_year") {
        if (args.size() != 1) {
            throw std::runtime_error("is_leap_year expects 1 argument: year");
        }
        
        int year = static_cast<int>(valueToNumber(args[0]));
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        
        return makeNumber(isLeap ? 1.0 : 0.0);
    }
    
    if (name == "days_in_month") {
        if (args.size() != 2) {
            throw std::runtime_error("days_in_month expects 2 arguments: year, month");
        }
        
        int year = static_cast<int>(valueToNumber(args[0]));
        int month = static_cast<int>(valueToNumber(args[1]));
        
        if (month < 1 || month > 12) {
            throw std::runtime_error("Month must be between 1 and 12");
        }
        
        static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int daysInMonth = days[month];
        
        if (month == 2) {
            bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (isLeap) {
                daysInMonth = 29;
            }
        }
        
        return makeNumber(static_cast<double>(daysInMonth));
    }
    
    throw std::runtime_error("Unknown time function: " + name);
}
