# Chronometro (C++11 timing library)

[![GitHub releases](https://img.shields.io/github/v/release/juste-injuste/Chronometro.svg)](https://github.com/juste-injuste/Chronometro/releases)
[![License](https://img.shields.io/github/license/juste-injuste/Chronometro.svg)](LICENSE)

Chronometro is a simple and lightweight C++11 (and newer) library that allows you to measure the execution time of functions or code blocks.

## Usage

Chronometro offers three ways to measure elapsed time:
* [Stopwatch](#Stopwatch) class
* [execution_time](#execution_time) function
* [CHRONOMETRO_EXECUTION_TIME](#CHRONOMETRO_EXECUTION_TIME) macro

Chronometro also introduces the [Unit](#Unit) enum class type.

---

### Unit

The `Unit` enum class type is used to chose the displayed time unit.
* `Unit::ns` (nanoseconds)
* `Unit::us` (microseconds)
* `Unit::ms` (milliseconds)
* `Unit::s` (seconds)
* `Unit::min` (minutes)
* `Unit::h` (hours)
* `Unit::automatic` deduces the appropriate time unit.

---

### Stopwatch

```text
Stopwatch(unit = automatic)
```
The Stopwatch class allows to measure the time it takes to execute code blocks. A stopwatch object may be started, paused stopped or restarted. When a stopwatch object is stopped, it displays the measured elasped time. A stopwatch object starts measuring time upon creation.

The constructor takes this optional parameter:
* `unit` specifies the unit used when displaying elapsed time. The default value is `Unit::automatic`.

The class has these methods:
* `start()` starts measuring time.
* `pause()` stops time measurement until the next `start()` or `restart()`.
* `stop()` stops time measurement, displays measured time and returns the elapsed time as the `std::chrono::high_resolution_clock::duration` type.
* `restart()` resets the measured time and starts measuring time.

Here is how `Stopwatch` may be used:
```cpp
#include <Chronometro.hpp>
extern void sleep_for_ms(int);

int main()
{
  Chronometro::Stopwatch stopwatch(Chronometro::Unit::ms);
  sleep_for_ms(30);

  stopwatch.pause();
  sleep_for_ms(1000);  // not measured by the stopwatch

  stopwatch.start();
  sleep_for_ms(70);

  stopwatch.stop(); // prints "elapsed time: 100 ms"

  stopwatch.restart();
  sleep_for_ms(500);

  stopwatch.stop(); // prints "elapsed time: 500 ms"
}
```

---

### execution_time

```text
execution_time(function, repetitions, ...arguments)
```
The execution_time function allows to measure the time it takes to execute a function for a specified number of repetitions. After the repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit and the elapsed time is returned as the `std::chrono::high_resolution_clock::duration` type.

The function takes these arguments:
* `function` is the function whose execution time will be measured
* `repetitions` is the amount of function execution repetitions
* `arguments` is optional, it is used to pass arguments to the function

Here is how `execution_time` may be used:
```cpp
#include <Chronometro.hpp>
extern void sleep_for_ms(int);

int main()
{
  Chronometro::execution_time(sleep_for_ms, 10, 30); // prints "elapsed time: 300 ms"
}
```

---

### CHRONOMETRO_EXECUTION_TIME

```text
CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)
```
The CHRONOMETRO_EXECUTION_TIME macro functions like the [execution_time](#execution_time) function, difference being that it is a text-replacement macro, meaning the function is not called function pointer indirection, which may be desireable. After the repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit and the elapsed time is returned as the `std::chrono::high_resolution_clock::duration` type.

_Limitation_ : when using the CHRONOMETRO_EXECUTION_TIME macro, you may not have a variable named `_iteration_` otherwise there will be name collision.

The macro takes these arguments:
* `function` is the function whose execution time will be measured
* `repetitions` is the amount of function execution repetitions
* `...` is optional, it is used to pass arguments to the function

Here is how `CHRONOMETRO_EXECUTION_TIME` may be used:
```cpp
#include <Chronometro.hpp>
extern void sleep_for_ms(int);

int main()
{
  CHRONOMETRO_EXECUTION_TIME(sleep_for_ms, 10, 30); // prints "elapsed time: 300 ms"
}
```

---

## Installation

Chronometro is a header-only library. To use it in your C++ project, simply `#include` the [Chronometro.hpp](include/Chronometro.hpp) header file.

## Limitations

Chronometro relies entirely on the accuracy of `std::chrono::high_resolution_clock`.

## Version History

Version 1.0: initial release

## License

Chronometro is released under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Author

Justin Asselin (juste-injuste)  
email: justin.asselin@usherbrooke.ca  
GitHub: [juste-injuste](https://github.com/juste-injuste)