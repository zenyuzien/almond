
#ifndef _sleep
#define _sleep
#include <chrono>
#include <thread>

void delay(int time)
{
    std::this_thread::sleep_for(std::chrono::seconds(time)); // sleep for 1 second
}

#endif 