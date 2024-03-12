#define CHZ_NOT_THREADSAFE
#include "Chronometro.hpp"
#include <iostream>

int main()
{
  chz::Stopwatch stopwatch;
  chz::sleep(30);
  stopwatch.pause();
  chz::sleep(50); // not measured by the stopwatch
  stopwatch.start();
  chz::sleep(70);

  // these print operations are avoided to avoid measuring them
  stopwatch.avoid(), std::cout << stopwatch.split().style<3, chz::Unit::us>(); // prints ~"elapsed time: 100000.000 us"
  chz::sleep(80);
  stopwatch.avoid(), std::cout << stopwatch.split(); // prints ~"elapsed time: 80 ms"
  stopwatch.avoid(), std::cout << stopwatch.total(); // prints ~"elapsed time: 180 ms"
  stopwatch.reset();
  chz::sleep(250);

  std::cout << stopwatch.total().style<chz::Unit::us>(); // prints ~"elapsed time: 250000 ns"
  stopwatch.pause();
  std::cout << stopwatch.total().style<2>(); // warning issued, prints ~"elapsed time: 250.00 ms"

  std::cout << '\n';
  CHZ_MEASURE()
  std::cout << "once\n";

  std::cout << '\n';
  CHZ_MEASURE(2)
  std::cout << "twice\n";

  std::cout << '\n';
  CHZ_MEASURE(3, "iteration %# took %us")
  std::cout << "thrice\n";

  std::cout << '\n';
  CHZ_MEASURE(4, "iteration %# took %us",  "took %us overall")
  std::cout << "four times\n";

  std::cout << '\n';
  CHZ_MEASURE(5, "",  "took %us overall")
  std::cout << "five times\n";

  std::cout << '\n';
  CHZ_MEASURE(1, "", "should take ~800 ms, took %ms")
  {
    unsigned inner_loops = 0, outer_loops = 0;
    while (inner_loops < 5)
    {
      ++outer_loops;
      CHZ_ONLY_EVERY(200) // first execution does not wait 200 ms
      {
        ++inner_loops;
        std::cout << "executing inner loop...\n"; // measured
      }
    }
    
    std::cout << "inner loop executions: " << inner_loops << '\n'; // measured
    std::cout << "outer loop executions: " << outer_loops << '\n'; // measured
  }

  std::cout << '\n';
  for (auto measurement : chz::Measure(4, "iteration %# took %ms", "iterations took %ms"))
  {
    measurement.avoid(), std::cout << "currently doing iteration #" << measurement.iteration << '\n';

    chz::sleep(100);
  }

  std::cout << '\n';
  CHZ_MEASURE(100, "", "average iteration took %Dms")
  {
    chz::sleep(1);
  }

  CHZ_LOOP_FOR(10)
  {
    CHZ_BREAK_AFTER(5);
  }

  std::cout << '\n';
  for (auto measurement : chz::Measure(10, "iteration %# took %ms", "average iteration took %Dms, total took %ms"))
  {
    chz::sleep(7); // measured

    measurement.avoid(), chz::sleep(50); // not measured

    chz::sleep(1); // measured

    {
      auto guard = measurement.avoid();
      chz::sleep(10); // not measured
      chz::sleep(5);  // not measured
    }

    chz::sleep(1); // measured

    measurement.pause();
    chz::sleep(100); // not measured

    measurement.start();
    chz::sleep(2); // measured
  }
}
