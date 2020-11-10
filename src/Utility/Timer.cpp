#include "Timer.h"

Timer::Timer()
{
	now = std::chrono::high_resolution_clock::now();
	lastTime = std::chrono::high_resolution_clock::now();
}

bool Timer::IsSecondPassed()
{
	if (sum > 1000)
	{
		sum -= 1000;
		return true;
	}
	else
	{
		return false;
	}
}

double Timer::GetDelta()
{
	now = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration<double, std::nano>(now - lastTime);
	//auto elapsedM = std::chrono::duration<double, std::milli>(now - lastTime);
	lastTime = now;
	sum += elapsed.count() / 1000000.0;
	return elapsed.count() / nanoSecond;
}

void Timer::Set()
{
	now = std::chrono::high_resolution_clock::now();
	lastTime = now;
}