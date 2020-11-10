#pragma once
#include <chrono>

class Timer
{
public:
	Timer();
	bool IsSecondPassed();
	double GetDelta();
	void Set();
private:
	const double nanoSecond = 1000000000.0 / 60.0;
	double sum = 0;
#ifdef _WIN32
	std::chrono::time_point<std::chrono::steady_clock> now;
	std::chrono::time_point<std::chrono::steady_clock> lastTime;
#else
	std::chrono::time_point<std::chrono::system_clock> now;
	std::chrono::time_point<std::chrono::system_clock> lastTime;
#endif
};