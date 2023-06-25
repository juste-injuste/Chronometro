#include "include/Chronometro.hpp"
#include <fstream>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(int ms)
{
  auto start = std::chrono::high_resolution_clock::now();
  while(std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

void stopwatch_tests()
{
  using namespace Chronometro;

  // using default clock (high_resolution_clock)
  Stopwatch<> stopwatch_1{Unit::ms};
  sleep_for_ms(30);
  stopwatch_1.pause();
  sleep_for_ms(1000); // not measured by the stopwatch
  stopwatch_1.start();
  sleep_for_ms(70);
  stopwatch_1.stop(); // prints "elapsed time: 100 ms"
  stopwatch_1.restart();
  sleep_for_ms(500);
  stopwatch_1.stop(); // prints "elapsed time: 500 ms"

  // using steady clock
  Stopwatch<steady_clock> stopwatch_2{Unit::ns};
  sleep_for_ms(35);
  stopwatch_2.stop();  // prints "elapsed time: 35000000 ns"
  stopwatch_2.pause(); // warning issued
  stopwatch_2.stop();  // warning issued, prints "elapsed time: 35000000 ns"
  stopwatch_2.set(Unit(97)); // invalid, sets unit to automatic
  stopwatch_2.stop();  // warning issued, prints "elapsed time: 35 ms"
  stopwatch_2.start();
  sleep_for_ms(100);
  stopwatch_2.stop();  // prints "elapsed time: 135 ms"
  stopwatch_2.start();
  sleep_for_ms(50);
  stopwatch_2.restart();
  sleep_for_ms(75);
  stopwatch_2.stop();  // prints "elapsed time: 75 ms"
  stopwatch_2.restart();
  sleep_for_ms(5);
  stopwatch_2.stop();  // prints "elapsed time: 5000 us"
}

void function_tests()
{
  using namespace Chronometro;
  
  // using default clock (high_resolution_clock)
  execution_time(sleep_for_ms, 10, 25); // prints "elapsed time: 250 ms"
  
  // using steady clock
  execution_time<steady_clock>(sleep_for_ms, 5, 60); // prints "elapsed time: 300 ms"
}

void macro_tests()
{
  CHRONOMETRO_EXECUTION_TIME(sleep_for_ms, 7, 50); // prints "elapsed time: 350 ms"
}

int main()
{
  std::ofstream file_1("stopwatch_tests.txt");
  std::ofstream file_2("stopwatch_tests_warnings.txt");
  std::ofstream file_3("function_tests.txt");
  std::ofstream file_4("macro_tests.txt");

  Chronometro::out_stream.rdbuf(file_1.rdbuf()); // redirect output
  Chronometro::wrn_stream.rdbuf(file_2.rdbuf()); // redirect warnings
  stopwatch_tests();

  Chronometro::out_stream.rdbuf(file_3.rdbuf()); // redirect output
  Chronometro::wrn_stream.rdbuf(file_3.rdbuf()); // redirect warnings
  function_tests();

  Chronometro::out_stream.rdbuf(file_4.rdbuf()); // redirect output
  Chronometro::wrn_stream.rdbuf(file_4.rdbuf()); // redirect warnings
  macro_tests();
}
