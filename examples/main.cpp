#include "Chronometro.hpp"
#include <fstream>
#include <iostream>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  while (std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

void stopwatch_tests()
{
  using namespace Chronometro;

  // using default clock (high_resolution_clock)
  Stopwatch stopwatch_1;
  sleep_for_ms(30);
  stopwatch_1.pause();
  sleep_for_ms(500);   // not measured by the stopwatch
  stopwatch_1.unpause();
  sleep_for_ms(70);
  stopwatch_1.split(); // prints ~"elapsed time: 100000 us"
  stopwatch_1.reset();
  sleep_for_ms(250);
  stopwatch_1.split(); // prints ~"elapsed time: 250000 us"

  // using system clock
  Stopwatch stopwatch_2;
  sleep_for_ms(35);
  stopwatch_2.lap();   // prints ~"lap time: 35 ms"
  sleep_for_ms(90);
  stopwatch_2.lap();   // prints ~"lap time: 90 ms"
  stopwatch_2.split(); // prints ~"elapsed time: 125 ms"
  sleep_for_ms(65);
  stopwatch_2.lap();   // prints ~"lap time: 65 ms"
  stopwatch_2.pause();
  sleep_for_ms(100);
  stopwatch_2.unpause();
  sleep_for_ms(50);
  stopwatch_2.lap();   // prints ~"lap time: 50 ms"
  stopwatch_2.split(); // prints ~"elapsed time: 240 ms"

  // using steady clock
  Stopwatch stopwatch_3;
  sleep_for_ms(3);
  stopwatch_3.split(); // prints ~"elapsed time: 3000 us"
  stopwatch_3.pause(); // warning issued
  stopwatch_3.split(); // warning issued
  stopwatch_3.unpause();
  sleep_for_ms(4);
  stopwatch_3.split(); // prints ~"elapsed time: 7000 us"
  stopwatch_3.unpause();
  sleep_for_ms(10);
  stopwatch_3.split(); // prints ~"elapsed time: 17 ms"
}


int main()
{
  std::ofstream file_1("stopwatch_tests.txt");
  std::ofstream file_2("function_tests.txt");
  std::ofstream file_3("macro_tests.txt");

  Chronometro::Global::out.rdbuf(file_1.rdbuf()); // redirect output
  Chronometro::Global::wrn.rdbuf(file_1.rdbuf()); // redirect warnings
  std::cout << "running Stopwatch tests...\n";
  stopwatch_tests();
  std::cout << "Stopwatch tests done, see \"stopwatch_tests.txt\" for the output\n\n";

}
