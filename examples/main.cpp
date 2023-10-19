#include "Chronometro.hpp"
#include <fstream>
#include <iostream>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  using namespace std::chrono;
  auto start = high_resolution_clock::now();
  while(nanoseconds(high_resolution_clock::now()-start).count() < ms*1000000);
}

void stopwatch_tests()
{
  using namespace Chronometro;

  // using default clock (high_resolution_clock)
  Stopwatch<> stopwatch_1{Unit::us};
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
  Stopwatch<system_clock> stopwatch_2;
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
  Stopwatch<steady_clock> stopwatch_3{Unit(42)}; // warning issued
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
  /* do not do this*/ const_cast<Unit&>(stopwatch_3.unit) = Unit(42); // force invalid unit
  stopwatch_3.lap();   // error issued
}

void function_tests()
{
  using namespace Chronometro;
  
  // using default clock (high_resolution_clock)
  execution_time(sleep_for_ms, 10, 25); // prints ~"elapsed time: 250 ms"
  
  // using steady clock
  execution_time<steady_clock>(sleep_for_ms, 5, 60); // prints ~"elapsed time: 300 ms"
}

void macro_tests()
{
  CHRONOMETRO_EXECUTION_TIME(sleep_for_ms, 7, 20); // prints ~"elapsed time: 140 ms"
}

int main()
{
  std::ofstream file_1("stopwatch_tests.txt");
  std::ofstream file_2("function_tests.txt");
  std::ofstream file_3("macro_tests.txt");

  Chronometro::Global::out.rdbuf(file_1.rdbuf()); // redirect output
  Chronometro::Global::wrn.rdbuf(file_1.rdbuf()); // redirect warnings
  Chronometro::Global::err.rdbuf(file_1.rdbuf()); // redirect errors
  std::cout << "running Stopwatch tests...\n";
  stopwatch_tests();
  std::cout << "Stopwatch tests done, see \"stopwatch_tests.txt\" for the output\n\n";

  Chronometro::Global::out.rdbuf(file_2.rdbuf()); // redirect output
  Chronometro::Global::wrn.rdbuf(file_2.rdbuf()); // redirect warnings
  Chronometro::Global::err.rdbuf(file_2.rdbuf()); // redirect errors
  std::cout << "running execution_time tests...\n";
  function_tests();
  std::cout << "execution_time tests done, see \"function_tests.txt\" for the output\n\n";

  Chronometro::Global::out.rdbuf(file_3.rdbuf()); // redirect output
  Chronometro::Global::wrn.rdbuf(file_3.rdbuf()); // redirect warnings
  Chronometro::Global::err.rdbuf(file_3.rdbuf()); // redirect errors
  std::cout << "running CHRONOMETRO_EXECUTION_TIME tests...\n";
  macro_tests();
  std::cout << "CHRONOMETRO_EXECUTION_TIME tests done, see \"macro_tests.txt\" for the output\n\n";
}
