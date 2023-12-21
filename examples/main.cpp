#define CHRONOMETRO_WARNINGS
#include "Chronometro.hpp"
#include <iostream>

// scuffed sleep function to demonstrate the basic usage of the library
inline void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
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
  std::cout << stopwatch.lap();   // prints ~"elapsed time: 100 ms"
  sleep_for_ms(80);
  std::cout << stopwatch.lap();   // prints ~"elapsed time: 80 ms"
  std::cout << stopwatch.split(); // prints ~"elapsed time: 180 ms"
  stopwatch.reset();
  sleep_for_ms(250);
  std::cout << stopwatch.split<Chronometro::Unit::ns>(); // prints ~"elapsed time: 250000000 ns"
  stopwatch.pause();
  std::cout << stopwatch.split(); // warning issued, prints ~"elapsed time: 250 ms"

  std::cout << '\n';
  CHRONOMETRO_MEASURE()
  std::cout << "once\n";
  
  std::cout << '\n';
  CHRONOMETRO_MEASURE(2)
  std::cout << "twice\n";
  
  std::cout << '\n';
  CHRONOMETRO_MEASURE(3, "iteration %# took %us")
  std::cout << "thrice\n";
  
  std::cout << '\n';
  CHRONOMETRO_MEASURE(4, "iteration %# took %us",  "took %ms overall")
  std::cout << "four times\n";
  
  std::cout << '\n';
  CHRONOMETRO_MEASURE(5, "",  "took %ms overall")
  std::cout << "five times\n";

  std::cout << '\n';
  CHRONOMETRO_MEASURE(1, nullptr, "should take 800 ms, took %ms")
  {
    unsigned count1 = 0, count2 = 0;
    while (count1 < 5)
    {
      CHRONOMETRO_ONLY_EVERY_MS(200) // first execution does not wait 500 ms
      {
        std::cout << "incrementing...\n";
        ++count1;
      }
      ++count2;
    }
    std::cout << "while loop executions: " << count2 << '\n';
  }

  std::cout << '\n';
  for (auto iteration : Chronometro::Measure(4, "iteration %# took %ms", "iterations took %ms"))
  {
    std::cout << "currently doing iteration #" << iteration << '\n';
    sleep_for_ms(100);
  }
}
