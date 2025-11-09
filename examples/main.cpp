#define STZ_NOT_THREADSAFE
#include "Chronometro.hpp"
#include <iostream>
#include <cstring>

int main()
{
  stz::Stopwatch stopwatch;
  stz::sleep(30);
  stopwatch.pause();
  stz::sleep(50); // not measured by the stopwatch
  stopwatch.start();
  stz::sleep(70);

  // these print operations are avoided to avoid measuring them
  stopwatch.avoid(), std::cout << stopwatch.split().style<3, stz::Unit::us>(); // prints ~"elapsed time: 100000.000 us"
  stz::sleep(80);
  stopwatch.avoid(), std::cout << stopwatch.split(); // prints ~"elapsed time: 80 ms"
  stopwatch.avoid(), std::cout << stopwatch.total(); // prints ~"elapsed time: 180 ms"

  stopwatch.reset();
  stz::sleep(250);
  stopwatch.avoid(), std::cout << stopwatch.total().style<stz::Unit::us>(); // prints ~"elapsed time: 250000 us"
  stopwatch.pause();
  std::cout << stopwatch.total().style<2>(); // warning issued, prints ~"elapsed time: 250.00 ms"

  std::cout << '\n';
  stz::measure_block()
  {
    std::cout << "once\n";
  };

  std::cout << '\n';
  stz::measure_block(2)
  {
    std::cout << "twice\n";
  };

  std::cout << '\n';
  stz::measure_block(3, "iteration %# took %us")
  {
    std::cout << "thrice\n";
  };

  std::cout << '\n';
  stz::measure_block(4, "iteration %# took %us",  "took %us overall")
  {
    std::cout << "four times\n";
  };

  std::cout << '\n';
  stz::measure_block(5, "",  "took %us overall")
  {
    std::cout << "five times\n";
  };

  std::cout << '\n';
  stz::measure_block("should take ~800 ms, took %ms")
  {
    unsigned inner_loops = 0, outer_loops = 0;
    while (inner_loops < 5)
    {
      ++outer_loops;
      stz::if_elapsed(200)
      {
        ++inner_loops;
        std::cout << "executing inner loop...\n"; // measured
      };
    }
    
    std::cout << "inner loop executions: " << inner_loops << '\n'; // measured
    std::cout << "outer loop executions: " << outer_loops << '\n'; // measured
  };

  std::cout << '\n';
  for (auto iteration : stz::Measure(4, "iteration %# took %ms", "iterations took %ms"))
  {
    iteration.avoid(), std::cout << "currently doing iteration #" << iteration.value << '\n'; // not measured
    stz::sleep(100);
  }

  std::cout << '\n';
  stz::measure_block("average iteration took %Dms", 100)
  {
    stz::sleep(1);
  };

  stz::loop_n_times(10)
  {
    stz::break_after_n(5);
  };

  std::cout << '\n';
  for (auto iteration : stz::Measure(10, "iteration %# took %ms", "average iteration took %Dms, total took %ms"))
  {
    stz::sleep(7); // measured

    iteration.avoid(), stz::sleep(50); // not measured

    stz::sleep(1); // measured

    {
      auto guard = iteration.avoid();
      stz::sleep(10); // not measured
      stz::sleep(5);  // not measured
    }

    stz::sleep(1); // measured

    iteration.pause();
    stz::sleep(100); // not measured

    iteration.start();
    stz::sleep(2); // measured

    stz::sleep(std::chrono::microseconds(1));
  }
}
