/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Chronometro

-----licence------------------------------------------------------------------------------------------------------------
 
MIT License

Copyright (c) 2023 Justin Asselin (juste-injuste)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 
-----versions-----------------------------------------------------------------------------------------------------------

Version 0.1.0 - Initial release

-----description--------------------------------------------------------------------------------------------------------

Chronometro is a simple and lightweight C++11 (and newer) library that allows you to measure the
execution time of functions or code blocks. See the included README.MD file for more information.

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef CHRONOMETRO_HPP
#define CHRONOMETRO_HPP
// --necessary standard libraries---------------------------------------------------------------------------------------
#include <chrono>   // for std::chrono::high_resolution_clock and std::chrono::nanoseconds
#include <ostream>  // for std::ostream
#include <iostream> // for std::cout, std::cerr, std::endl
#include <cstddef>  // for size_t
#include <string>   // for std::string
// --Chronometro library------------------------------------------------------------------------------------------------
namespace Chronometro
{
  namespace Version
  {
    constexpr long MAJOR = 000;
    constexpr long MINOR = 001;
    constexpr long PATCH = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
  
  namespace stdc = std::chrono;

  // measures the time it takes to execute the following statement/block n times
# define CHRONOMETRO_MEASURE(...)

  struct Time
  {
    stdc::nanoseconds::rep nanoseconds;
  };

  // measure elapsed time
  class Stopwatch final
  {
  public:
    // display and return lap time
    inline Time lap() noexcept;
    // display and return split time
    inline Time split() noexcept;
    // pause time measurement
    inline void pause() noexcept;
    // reset measured times
    inline void reset() noexcept;
    // unpause time measurement
    inline void unpause() noexcept;
  private:
    bool                                    is_paused    = false;
    stdc::high_resolution_clock::duration   duration     = {};
    stdc::high_resolution_clock::duration   duration_lap = {};
    stdc::high_resolution_clock::time_point previous     = stdc::high_resolution_clock::now();
    stdc::high_resolution_clock::time_point previous_lap = previous;
  };

  class Measure final
  {
  public:
    inline Measure() noexcept = default;
    inline Measure(size_t n) noexcept;
    inline Measure(size_t n, const char* lap_format) noexcept;
    inline Measure(size_t n, const char* lap_format, const char* total_format) noexcept;
    inline size_t operator*() noexcept;
    inline void operator++() noexcept;
    inline bool operator!=(const Measure&) noexcept;
    inline operator bool() noexcept;
    inline Measure& begin() noexcept;
    inline Measure end() noexcept;
  private:
    Stopwatch    stopwatch;
    const size_t iterations      = 1;
    size_t       iterations_left = iterations;
    const char*  lap_format      = nullptr;
    const char*  total_format    = "total elapsed time: %ms";
  };

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream wrn{std::cerr.rdbuf()}; // warning ostream
  }

  std::ostream& operator << (std::ostream& ostream, const Time time) noexcept;
// --Chronometro library: backend forward declaration-------------------------------------------------------------------
  namespace Backend
  {
    std::string format_string(const Time time, std::string format, const size_t iteration = 0) noexcept;

# if defined(CHRONOMETRO_NO_WARNINGS)
#   define CHRONOMETRO_SW_WARNING(message) {} /* warnings are disabled do not #define CHRONOMETRO_NO_WARNINGS to enable them */
# else
#   define CHRONOMETRO_SW_WARNING(message) Global::wrn << "warning: Stopwatch::" << __func__ << "(): " << message << std::endl
# endif
  }
// --Chronometro library: frontend definitions--------------------------------------------------------------------------
# undef  CHRONOMETRO_MEASURE
# define CHRONOMETRO_MEASURE(...) for (Chronometro::Measure measurement{__VA_ARGS__}; measurement; ++measurement)

  Time Stopwatch::lap() noexcept
  {
    // measure current time
    const auto now = stdc::high_resolution_clock::now();

    stdc::nanoseconds::rep ns = duration_lap.count();

    if (is_paused == false)
    {
      // save elapsed times
      duration += now - previous;
      ns       += (now - previous_lap).count();
      
      // reset measured time
      duration_lap = stdc::high_resolution_clock::duration{};
      previous     = stdc::high_resolution_clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_SW_WARNING("cannot measure lap, must not be paused");

    return Time{ns};
  }

  Time Stopwatch::split() noexcept
  {
    // measure current time
    const auto now = stdc::high_resolution_clock::now();

    stdc::nanoseconds::rep ns = duration.count();

    if (is_paused == false)
    {
      // save elapsed times
      duration     += now - previous;
      duration_lap += now - previous_lap;
      
      ns = duration.count();

      // save time point
      previous     = stdc::high_resolution_clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_SW_WARNING("cannot measure split, must not be paused");
    
    return Time{ns};
  }

  void Stopwatch::pause() noexcept
  {
    // measure current time
    const auto now = stdc::high_resolution_clock::now();

    // add elapsed time up to now if not paused
    if (is_paused == false)
    {
      is_paused = true;

      // save elapsed times
      duration     += now - previous;
      duration_lap += now - previous_lap;
    }
    else CHRONOMETRO_SW_WARNING("cannot pause further, is already paused");
  }

  void Stopwatch::reset() noexcept
  {
    // reset measured time
    duration     = stdc::high_resolution_clock::duration{};
    duration_lap = stdc::high_resolution_clock::duration{};

    // hot reset if unpaused
    if (is_paused == false)
    {
      previous     = stdc::high_resolution_clock::now();
      previous_lap = previous;
    } 
  }

  void Stopwatch::unpause() noexcept
  {
    if (is_paused == true)
    {
      // unpause
      is_paused = false;

      // reset measured time
      previous     = stdc::high_resolution_clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_SW_WARNING("is already unpaused");
  }
  
  Measure::Measure(size_t n) noexcept :
    iterations(n)
  {}
  
  Measure::Measure(size_t n, const char* lap_format) noexcept :
    iterations(n),
    lap_format{lap_format}
  {}
  
  Measure::Measure(size_t n, const char* lap_format, const char* total_format) noexcept :
    iterations(n),
    lap_format{lap_format},
    total_format{total_format}
  {}
  
  size_t Measure::operator*() noexcept
  {
    return iterations - iterations_left;
  }

  void Measure::operator++() noexcept
  {
    Time lap_time = stopwatch.lap();
    
    stopwatch.pause();

    if (lap_format)
    {
      Global::out << Backend::format_string(lap_time, lap_format, iterations - iterations_left) << std::endl;
    }

    --iterations_left;

    stopwatch.unpause();
  }

  bool Measure::operator!=(const Measure&) noexcept
  {
    return operator bool();
  }

  Measure::operator bool() noexcept
  {     
    if (iterations_left)
    {
      return true;
    }

    Time time = stopwatch.split();
    if (total_format)
    {
      Global::out << Backend::format_string(time, total_format) << std::endl;
    }

    return false;
  }

  Measure& Measure::begin() noexcept
  {
    return *this;
  }

  Measure Measure::end() noexcept
  {
    return Measure{0};
  }

  std::ostream& operator << (std::ostream& ostream, const Time time) noexcept
  {
    // 10 h < duration
    if (time.nanoseconds > 36000000000000)
    {
      return ostream << Backend::format_string(time, "elapsed time: %h") << std::endl;
    }

    // 10 min < duration <= 10 h
    if (time.nanoseconds > 600000000000)
    {
      return ostream << Backend::format_string(time, "elapsed time: %min") << std::endl;
    }

    // 10 s < duration <= 10 m
    if (time.nanoseconds > 10000000000)
    {
      return ostream << Backend::format_string(time, "elapsed time: %s") << std::endl;
    }

    // 10 ms < duration <= 10 s
    if (time.nanoseconds > 10000000)
    {
      return ostream << Backend::format_string(time, "elapsed time: %ms") << std::endl;
    }

    // 10 us < duration <= 10 ms
    if (time.nanoseconds > 10000)
    {
      return ostream << Backend::format_string(time, "elapsed time: %us") << std::endl;
    }

    // duration <= 10 us
    return ostream << Backend::format_string(time, "elapsed time: %ns") << std::endl;
  }
// --Chronometro library: backend definitions---------------------------------------------------------------------------
  namespace Backend
  {
    std::string format_string(const Time time, std::string format, const size_t iteration) noexcept
    {
      size_t iteration_position = format.find("%#");
      if (iteration_position != std::string::npos)
      {
        format.replace(iteration_position, 2, std::to_string(iteration));
      }

      size_t time_position = format.rfind("%ms");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000000) + ' ');
      }

      time_position = format.rfind("%us");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000) + ' ');
      }

      time_position = format.rfind("%ns");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds) + ' ');
      }

      time_position = format.rfind("%s");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000000000) + ' ');
      }

      time_position = format.rfind("%min");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/60000000) + ' ');
      }

      time_position = format.rfind("%h");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/3600000000) + ' ');
      }
      
      return format;
    }

#   define CHRONOMETRO_ONLY_EVERY_MS(n)                               \
      if ([]{                                                         \
        static_assert(n > 0, "n must be a non-zero positive number"); \
        using clock = std::chrono::high_resolution_clock;             \
        static clock::time_point previous = {};                       \
        auto target = std::chrono::nanoseconds{n*1000000};            \
        if ((clock::now() - previous) > target)                       \
        {                                                             \
          previous = clock::now();                                    \
          return true;                                                \
        }                                                             \
        return false;                                                 \
      }())
  }
}
#endif
