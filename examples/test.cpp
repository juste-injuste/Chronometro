// #define CHRONOMETRO_WARNINGS
#include "../include/Chronometro.hpp"
#include <iostream>

// scuffed sleep function to demonstrate the basic usage of the library
inline void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  while (std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

int test()
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
  std::cout << stopwatch.split().unit<Chronometro::Unit::us>(); // prints ~"elapsed time: 250000 ns"
  stopwatch.pause();
  std::cout << stopwatch.split().decimals<2>(); // warning issued, prints ~"elapsed time: 250.00 ms"

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
  CHRONOMETRO_MEASURE(4, "iteration %# took %us",  "took %us overall")
  std::cout << "four times\n";

  std::cout << '\n';
  CHRONOMETRO_MEASURE(5, "",  "took %us overall")
  std::cout << "five times\n";

  std::cout << '\n';
  CHRONOMETRO_MEASURE(1, nullptr, "should take ~800 ms, took %ms")
  {
    unsigned inner_loops = 0, outer_loops = 0;
    while (inner_loops < 5)
    {
      ++outer_loops;
      CHRONOMETRO_ONLY_EVERY_MS(200) // first execution does not wait 200 ms
      {
        ++inner_loops;
        std::cout << "executing inner loop...\n"; // measured
      }
    }
    
    std::cout << "inner loop executions: " << inner_loops << '\n'; // measured
    std::cout << "outer loop executions: " << outer_loops << '\n'; // measured
  }

  std::cout << '\n';
  for (auto measurement : Chronometro::Measure(4, "iteration %# took %ms", "iterations took %ms"))
  {
    measurement.guard(), std::cout << "currently doing iteration #" << measurement.iteration << '\n';

    sleep_for_ms(100);
  }

  std::cout << '\n';
  CHRONOMETRO_MEASURE(100, "", "average iteration took %Dms")
  {
    sleep_for_ms(1);
  }

  std::cout << '\n';
  for (Chronometro::Measure measurement(10, "iteration %# took %ms", "average iteration took %Dms, total took %ms"); measurement; ++measurement)
  {
    sleep_for_ms(10);

    measurement.pause();
    sleep_for_ms(100); // not measured
  }

  std::cout << '\n';
  for (auto iteration : Chronometro::Measure(10, "iteration %# took %ms", "average iteration took %Dms, total took %ms"))
  {
    sleep_for_ms(7);

    iteration.guard(), sleep_for_ms(100); // not measured
  }

  return 0;
}
