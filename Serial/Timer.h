#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>

std::string to_string_fixed(double val, uint8_t n = 2)
{
    std::ostringstream out;
    out << std::setprecision(n) << std::fixed << val;
    return out.str();
}

namespace Time
{
    std::time_t timestamp()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();
    }

    std::string format(std::time_t microseconds)
    {
        auto milliseconds = microseconds / 1000.0;
        if (milliseconds < 1)
        {
            return to_string_fixed(microseconds) + "mcs";
        }
        auto seconds = milliseconds / 1000.0;
        if (seconds < 1)
        {
            return to_string_fixed(milliseconds) + "ms";
        }
        auto minutes = seconds / 60.0;
        if (minutes < 1)
        {
            return to_string_fixed(seconds) + "s";
        }
        auto hours = minutes / 60.0;
        if (hours < 1)
        {
            return to_string_fixed(minutes) + "m";
        }

        return to_string_fixed(hours) + "h";
    }
}


//class Timer{
//public:
//    Timer()
//    {
//        m_StartTimepoint = std::chrono::high_resolution_clock::now();
//    }
//
//    ~Timer()
//    {
//        Stop();
//    }
//
//    void Stop()
//    {
//        auto endTimepoint = std::chrono::high_resolution_clock::now();
//
//        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
//        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
//
//        auto duration = end - start;
//        double ms = duration * 0.001;
//
//        std::cout << duration << "us(" << ms << "ms)\n";
//    }
//
//private:
//    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
//};
