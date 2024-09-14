#pragma once

#include <chrono>
#include <ctime>

class Timer
{
public:
    Timer()
    {
        start_ = std::chrono::system_clock::now();
    }

    ~Timer()
    {
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start_;
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        wprintf(L"---\nTotal elapsed time: %f seconds\n", elapsed_seconds.count());
    }

private:
    std::chrono::time_point<std::chrono::system_clock> start_;
};
