# Chronometro
Chronometro is a lightweight C++ library that allows you to measure the execution time of functions or code blocks. It provides a simple and convenient way to measure elapsed time with high precision.

## Usage
Chronometro offers 3 ways to measure elapsed time:
* [Stopwatch](#Stopwatch) instance
* [execution_time](#execution_time) function
* [CHRONOMETRO_EXECUTION_TIME](#CHRONOMETRO_EXECUTION_TIME) macro

Chronometro also introduces the [Unit](#Unit) enum class type.

### Unit
The `Unit` enum class type is used to chose the displayed time unit. <br>
* `automatic` deduces the appropriate time unit<br>
* `ns` (nanoseconds)<br>
* `us` (microseconds)<br>
* `ms` (milliseconds)<br>
* `s` (seconds)<br>
* `min` (minutes)<br>
* `h` (hours)<br>

### Stopwatch
The stopwatch class allows to create a stopwatch in your program. A stopwatch may be stopped or started. When a stopwatch is stopped, it displays the elasped time between the previous start.

The constructor takes in these optional parameters:<br>
* [Unit](#Unit) `unit` specifies the units to be used by the stopwatch.
  * the default is `Unit::automatic`.
* bool `display_on_destruction` specifies if it should display elapsed time on destruction.
  * the default is `false`.
  
### execution_time
The execution_time function allows to measure the time it takes to execute a function for a specified number of repetitions.

The function takes in these parameters:<br>
* F `function` is the function to measure
* size_t `repetitions` is the amount of function execution repetitions
* A... `arguments` is used to optionally pass arguments to the function

After all repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit.

### CHRONOMETRO_EXECUTION_TIME
The CHRONOMETRO_EXECUTION_TIME macro functions like the [execution_time](#execution_time) function. The difference being that it is a text-replacement macro, meaning the function is not called function pointer indirection, which may be desireable.

The macro takes in these parameters:<br>
* `function` is the function to measure
* `repetitions` is the amount of function execution repetitions
* `...` is used to optionally pass arguments to the function

After all repetitions are done, the elapsed time is displayed using the `Unit::automatic` time unit.

## Example
You can measure the execution time of a function or code block using the `execution_time` function provided by the Chronometro library. Here's an example:

```cpp
#include <iostream>
#include "Chronometro.hpp"

void foo()
{
  // function body
}

int main()
{
  // measure the elapsed time of a 1000 'foo' executions
  Chronometro::execution_time(foo, 1000); 

  return 0;
}
```

## Installation
Chronometro is a header-only library. To use it in your C++ project, simply include the [Chronometro.hpp](include/Chronometro.hpp) header file.

## Version History
Version 1.0: initial release

## License
Chronometro is released under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Author
Justin Asselin (juste-injuste)  
email: justin.asselin@usherbrooke.ca  
GitHub: [juste-injuste](https://github.com/juste-injuste)