# Chronometro (C++11 timing library)

[![GitHub releases](https://img.shields.io/github/v/release/juste-injuste/Chronometro.svg)](https://github.com/juste-injuste/Chronometro/releases)
[![License](https://img.shields.io/github/license/juste-injuste/Chronometro.svg)](LICENSE)

Chronometro is a simple and lightweight C++11 (and newer) library that allows you to measure the execution time of functions or code blocks.

---

## Usage

Chronometro offers three ways to measure elapsed time:
* [Stopwatch](#Stopwatch) class
* [execution_time](#execution_time) function
* [CHRONOMETRO_EXECUTION_TIME](#CHRONOMETRO_EXECUTION_TIME) macro

Chronometro also introduces the [Unit](#Unit) enum class type.

---

### Unit

The `Unit` enum class type is used to chose the displayed time unit. These are the enumerators:
* `Unit::ns` (nanoseconds)
* `Unit::us` (microseconds)
* `Unit::ms` (milliseconds)
* `Unit::s` (seconds)
* `Unit::min` (minutes)
* `Unit::h` (hours)
* `Unit::automatic` deduces the appropriate unit using the following:
  - If elapsed time is greater than 10 hours, use `Unit::h`
  - If elapsed time is greater than 10 minutes, use `Unit::min`
  - If elapsed time is greater than 10 seconds, use `Unit::s`
  - If elapsed time is greater than 10 milliseconds, use `Unit::ms`
  - If elapsed time is greater than 10 microseconds, use `Unit::us`
  - Otherwise, use `Unit::ns`

---

### Stopwatch

```
Stopwatch<C = high_resolution_clock>(unit = automatic)
```
The Stopwatch class allows to measure the time it takes to execute code blocks. A stopwatch may be started, paused stopped, restarted or have its unit set. When a stopwatch is stopped, it displays the elapsed time. Pausing or stopping a stopwatch returns the elapsed time as the `C::duration` type. A stopwatch starts measuring time upon creation.

_Template_:
* `C` is the clock used to measure time. The default is `std::chrono::high_resolution_clock`.

_Constructor_:
* `unit` sets the unit used when displaying elapsed time. The default is `Unit::automatic`.

_Methods_:
* `start()` starts measuring time.
* `pause()` stops time measurement until the next `start()` or `restart()`, and returns the elapsed time as the `C::duration` type.
* `stop()` stops time measurement, displays elapsed time and returns it as the `C::duration` type.
* `restart()` resets and starts measuring elapsed time.
* `set(unit)` sets the unit used when displaying the elapsed time.

_Error_:
* `stop()` will issue `"error: Stopwatch: invalid time unit"` if the unit is invalid.

_Warnings_:
* `start()` will issue `"warning: Stopwatch: already started"` if the stopwatch was already measuring time.
* `stop()` and `pause()` will issue `"warning: Stopwatch: already paused"` if the stopwatch was already paused.
* `set(unit)` will issue `"warning: Stopwatch: invalid unit; automatic used instead"` if the unit is invalid.

_Examples_:
```cpp
#include "Chronometro.hpp"
extern void sleep_for_ms(int);

int main()
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
}
```

---

### execution_time

```
execution_time<C = high_resolution_clock>(function, repetitions, ...arguments)
```
The execution_time function allows to measure the time it takes to execute a function for a specified number of repetitions. After the repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit and is returned as the `C::duration` type.

_Template_:
* `C` is the clock used to measure time. The default is `std::chrono::high_resolution_clock`.

_Arguments_:
* `function` is the function whose execution time will be measured.
* `repetitions` is the amount of function execution repetitions.
* `arguments` is optional, it is used to pass arguments to the function.

_Examples_:
```cpp
#include "Chronometro.hpp"
extern void sleep_for_ms(int);

int main()
{
  using namespace Chronometro;

  // using default clock (high_resolution_clock)
  execution_time(sleep_for_ms, 10, 30); // prints "elapsed time: 300 ms"

  // using steady clock
  execution_time<steady_clock>(sleep_for_ms, 10, 20); // prints "elapsed time: 200 ms"
}
```

---

### CHRONOMETRO_EXECUTION_TIME

```
CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)
```
The CHRONOMETRO_EXECUTION_TIME macro functions like the [execution_time](#execution_time) function, difference being that it is a text-replacement macro, meaning the function is not called via pointer indirection, which may be desireable in some scenarios. After the repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit and is returned as the `std::chrono::high_resolution_clock::duration` type.

_Arguments_:
* `function` is the function whose execution time will be measured.
* `repetitions` is the amount of function execution repetitions.
* `...` is optional, it is used to pass arguments to the function.

_Example_:
```cpp
#include "Chronometro.hpp"
extern void sleep_for_ms(int);

int main()
{
  CHRONOMETRO_EXECUTION_TIME(sleep_for_ms, 10, 30); // prints "elapsed time: 300 ms"
}
```

_Limitations_:
* The CHRONOMETRO_EXECUTION_TIME does not allow to specify a clock; `std::chrono::high_resolution_clock` is used.
* You may not use the CHRONOMETRO_EXECUTION_TIME macro with variables named `stopw_atch`, `itera_tion` or `repet_itions`; otherwise name collision occurs leading to undefined behavior.

---

## Streams

Chronometro defines these following `std::ostream`s, which may be [redirected](https://github.com/juste-injuste/Katagrafeas):
* `out_stream` elapsed times go through here (linked to `std::cout` by default).
* `err_stream` errors go through here (linked to `std::cerr` by default).
* `wrn_stream` warnings go through here (linked to `std::cerr` by default).

---

## Examples

For more extensive examples, including basic stream redirection, see [examples.cpp](examples.cpp).

---

## Version

Chronometro defines the following macros (which correspond to the current version):
```cpp
#define CHRONOMETRO_VERSION       001000000L
#define CHRONOMETRO_VERSION_MAJOR 1
#define CHRONOMETRO_VERSION_MINOR 0
#define CHRONOMETRO_VERSION_PATCH 0
```

---

## Details

Chronometro brings into scope `std::chrono::system_clock`, `std::chrono::steady_clock` and `std::chrono::high_resolution_clock` to simplify scope resolution.

---

## Disclosure

Chronometro relies entirely on the accuracy of the clock that is used.

---

## History

Version 1.0.0 - Initial release

---

## Installation

Chronometro is a header-only library. To use it in your C++ project, simply `#include` the [Chronometro.hpp](include/Chronometro.hpp) header file.

---

## License

Chronometro is released under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

## Author

Justin Asselin (juste-injuste)  
Email: justin.asselin@usherbrooke.ca  
GitHub: [juste-injuste](https://github.com/juste-injuste)
