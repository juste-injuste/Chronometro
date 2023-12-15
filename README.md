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
* `C` is the clock used to measure time.

_Constructor_:
* `unit` sets the unit used when displaying elapsed time.

_Methods_:
* `lap() -> C::duration` displays and returns the elapsed time since the last `lap()` (or construction).
* `split() -> C::duration` displays and returns the elapsed time since construction.
* `pause()` pauses time measurement.
* `reset()` resets the measured elapsed times.
* `unpause()` unpauses time measurement.

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

  stopwatch_1.unpause();
  sleep_for_ms(70);
  stopwatch_1.split(); // prints "elapsed time: 100 ms"
  stopwatch_1.pause()
  stopwatch_1.reset();
  stopwatch_1.unpause();
  sleep_for_ms(500);
  stopwatch_1.stop(); // prints "elapsed time: 500 ms"
}
```

_Members_:
* `unit` is the units used by the stopwatch when displaying elapsed times.

_Types_:
* `clock` is the clock used by the stopwatch.

_Warnings_:
* `"cannot measure lap, must not be paused"`
* `"cannot measure split, must not be paused"`
* `"is already paused"`
* `"is already unpaused"`
* `"invalid unit, automatic used instead"`

_Error_:
* `"invalid unit, invalid code path reached"`

---

### execution_time

```
execution_time<C = high_resolution_clock>(function, repetitions, ...arguments)
```
The execution_time function allows to measure the time it takes to execute a function for a specified number of repetitions. After the repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit and is returned as the `C::duration` type.

_Template_:
* `C` is the clock used to measure time.

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
* You may not use the CHRONOMETRO_EXECUTION_TIME macro with variables named `stopw_atch` or `itera_tion`; otherwise name collision occurs leading to undefined behavior.

---

### CHRONOMETRO_REPEAT

```
CHRONOMETRO_REPEAT(n_times)
```
The CHRONOMETRO_REPEAT repeats the following statement or block n times.

_Arguments_:
* `n_times` is the amount of times it will repeat the statement or block.

_Example_:
```cpp
#include "Chronometro.hpp"
extern void print(const char*);

int main()
{
  // prints "hello world\n" 3 times
  CHRONOMETRO_REPEAT(3)
    print("hello world\n");

  // prints "byebye world\n" 5 times
  CHRONOMETRO_REPEAT(5)
  {
    print("byebye ");
    print("world\n");
  }
}
```

_Limitations_:
* You may not use the CHRONOMETRO_REPEAT macro with a variable named `n_tim_es`; otherwise name collision occurs leading to undefined behavior.

---

## Streams

Chronometro outputs these following `std::ostream`s (defined in the `Chronometro::Global` namespace):
* `out` elapsed times go through here (linked to `std::cout` by default).
* `err` errors go through here (linked to `std::cerr` by default).
* `wrn` warnings go through here (linked to `std::cerr` by default).

---

## Examples

For more extensive examples, including basic stream redirection, see [main.cpp](examples/main.cpp).

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

## Compiler support

* `GCC` >= 4.7.3

---

## License

Chronometro is released under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

## Author

Justin Asselin (juste-injuste)  
Email: justin.asselin@usherbrooke.ca  
GitHub: [juste-injuste](https://github.com/juste-injuste)
