#define CHRONOMETRO_WARNINGS
#include "Chronometro.hpp"
#include <iostream>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  while (std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

int main()
{
  Chronometro::Stopwatch stopwatch;
  sleep_for_ms(30);
  stopwatch.pause();
  sleep_for_ms(500);              // not measured by the stopwatch
  stopwatch.unpause();
  sleep_for_ms(70);
  std::cout << stopwatch.split(); // prints ~"elapsed time: 100 ms"
  stopwatch.reset();
  sleep_for_ms(250);
  std::cout << stopwatch.split(); // prints ~"elapsed time: 250 ms"

  stopwatch.reset();
  sleep_for_ms(35);
  std::cout << stopwatch.lap();   // prints ~"lap time: 35 ms"
  sleep_for_ms(90);
  std::cout << stopwatch.lap();   // prints ~"lap time: 90 ms"
  std::cout << stopwatch.split(); // prints ~"elapsed time: 125 ms"
  sleep_for_ms(65);
  std::cout << stopwatch.lap();   // prints ~"lap time: 65 ms"
  stopwatch.pause();
  sleep_for_ms(100);
  stopwatch.unpause();
  sleep_for_ms(50);
  std::cout << stopwatch.lap();   // prints ~"lap time: 50 ms"
  std::cout << stopwatch.split(); // prints ~"elapsed time: 240 ms"

  stopwatch.reset();
  sleep_for_ms(3);
  std::cout << stopwatch.split(); // prints ~"elapsed time: 3000 us"
  stopwatch.pause();              // warning issued
  std::cout << stopwatch.split(); // warning issued
  stopwatch.unpause();
  sleep_for_ms(4);
  std::cout << stopwatch.split(); // prints ~"elapsed time: 7000 us"
  stopwatch.unpause();
  sleep_for_ms(10);
  std::cout << stopwatch.split(); // prints ~"elapsed time: 17 ms"
}
