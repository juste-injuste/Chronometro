# Chronometro

Chronometro is a lightweight C++ library that allows you to measure the execution time of functions or code blocks. It provides a simple and convenient way to measure elapsed time with high precision.

## Usage

The execution_time function takes the following parameters:

function: The function or code block to measure the execution time of.
repetitions: The number of times to repeat the measurement.
Displayed Time Units
By default, Chronometro automatically determines the appropriate time unit to display based on the measured execution time. However, you can specify a specific time unit if desired. The available time units are:

`ns` (nanoseconds)<br>
`us` (microseconds)<br>
`ms` (milliseconds)<br>
`s` (seconds)<br>
`min` (minutes)<br>
`h` (hours)<br>
To specify a time unit, you can modify the execution_time function call as follows:

Chronometro::execution_time(myFunction, 1000, Chronometro::Unit::ms); // Measure the execution time in milliseconds


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