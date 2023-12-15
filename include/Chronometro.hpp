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
#include <chrono>      // for std::chrono::high_resolution_clock and std::chrono::nanoseconds
#include <ostream>     // for std::ostream
#include <iostream>    // for std::cout, std::cerr, std::endl
#include <string>      // for std::string
#include <type_traits> // for std::conditional
// --Chronometro library------------------------------------------------------------------------------------------------
namespace Chronometro
{
  namespace Version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
  
#if defined(CHRONOMETRO_CLOCK) 
  using Clock = CHRONOMETRO_CLOCK;
#else
  using Clock = std::conditional<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock
  >::type;
#endif

  // measures the time it takes to execute the following statement/block n times
# define CHRONOMETRO_MEASURE(...)

  struct Time
  {
    std::chrono::nanoseconds::rep nanoseconds;
  };

  // measure elapsed time
  class Stopwatch final
  {
  public:
    inline // display and return lap time
    Time lap()     noexcept;
    
    inline // display and return split time
    Time split()   noexcept;

    inline // pause time measurement
    void pause()   noexcept;

    inline // reset measured times
    void reset()   noexcept;

    inline // unpause time measurement
    void unpause() noexcept;
  private:
    bool              is_paused    = false;
    Clock::duration   duration     = {};
    Clock::duration   duration_lap = {};
    Clock::time_point previous     = Clock::now();
    Clock::time_point previous_lap = previous;
  };

  class Measure final
  {
  public:
    Measure() noexcept = default;
    
    inline
    Measure(unsigned n) noexcept;

    inline
    Measure(unsigned n, const char* lap_format) noexcept;

    inline
    Measure(unsigned n, const char* lap_format, const char* total_format) noexcept;

    inline
    unsigned operator*() noexcept;
    
    inline
    void operator++() noexcept;
    
    inline
    bool operator!=(const Measure&) noexcept;
    
    inline
    operator bool() noexcept;
    
    inline
    Measure& begin() noexcept;
    
    inline
    Measure end() noexcept;
  private:
    Stopwatch      stopwatch;
    const unsigned iterations      = 1;
    unsigned       iterations_left = iterations;
    const char*    lap_format      = nullptr;
    const char*    total_format    = "total elapsed time: %ms";
  };

  // execute following statement/block only if its last execution was atleast N milliseconds prior
# define CHRONOMETRO_ONLY_EVERY_MS(N)

  std::ostream& operator<<(std::ostream& ostream, const Time time) noexcept;

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream wrn{std::cerr.rdbuf()}; // warning ostream
  }
// --Chronometro library: backend forward declaration-------------------------------------------------------------------
  namespace _backend
  {
    std::string _format_string(const Time time, std::string format) noexcept
    {
      auto time_position = format.rfind("%ms");
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

    std::string _format_string(const Time time, std::string format, const unsigned iteration) noexcept
    {
      auto iteration_position = format.find("%#");
      if (iteration_position != std::string::npos)
      {
        format.replace(iteration_position, 2, std::to_string(iteration));
      }
      
      return _format_string(time, format);
    }

# if defined(CHRONOMETRO_NO_WARNINGS)
#   define CHRONOMETRO_SW_WARNING(message) void(0) /* warnings are disabled do not #define CHRONOMETRO_NO_WARNINGS to enable them */
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
    const auto now = Clock::now();

    std::chrono::nanoseconds::rep ns = duration_lap.count();

    if (is_paused == false)
    {
      // save elapsed times
      duration += now - previous;
      ns       += (now - previous_lap).count();
      
      // reset measured time
      duration_lap = Clock::duration{};
      previous     = Clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_SW_WARNING("cannot measure lap, must not be paused");

    return Time{ns};
  }

  Time Stopwatch::split() noexcept
  {
    // measure current time
    const auto now = Clock::now();

    std::chrono::nanoseconds::rep ns = duration.count();

    if (is_paused == false)
    {
      // save elapsed times
      duration     += now - previous;
      duration_lap += now - previous_lap;
      
      ns = duration.count();

      // save time point
      previous     = Clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_SW_WARNING("cannot measure split, must not be paused");
    
    return Time{ns};
  }

  void Stopwatch::pause() noexcept
  {
    // measure current time
    const auto now = Clock::now();

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
    duration     = Clock::duration{};
    duration_lap = Clock::duration{};

    // hot reset if unpaused
    if (is_paused == false)
    {
      previous     = Clock::now();
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
      previous     = Clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_SW_WARNING("is already unpaused");
  }
  
  Measure::Measure(unsigned n) noexcept :
    iterations(n)
  {}
  
  Measure::Measure(unsigned n, const char* lap_format) noexcept :
    iterations(n),
    lap_format{lap_format}
  {}
  
  Measure::Measure(unsigned n, const char* lap_format, const char* total_format) noexcept :
    iterations(n),
    lap_format{lap_format},
    total_format{total_format}
  {}
  
  unsigned Measure::operator*() noexcept
  {
    return iterations - iterations_left;
  }

  void Measure::operator++() noexcept
  {
    Time lap_time = stopwatch.lap();
    
    stopwatch.pause();

    if (lap_format)
    {
      Global::out << _backend::_format_string(lap_time, lap_format, iterations - iterations_left) << std::endl;
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
      Global::out << _backend::_format_string(time, total_format) << std::endl;
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

# undef  CHRONOMETRO_ONLY_EVERY_MS
# define CHRONOMETRO_ONLY_EVERY_MS(N)                               \
    if ([]{                                                         \
      static_assert(n > 0, "n must be a non-zero positive number"); \
      static Clock::time_point previous = {};                       \
      auto target = std::chrono::nanoseconds{n*1000000};            \
      if ((Clock::now() - previous) > target)                       \
      {                                                             \
        previous = Clock::now();                                    \
        return true;                                                \
      }                                                             \
      return false;                                                 \
    }())

  std::ostream& operator<<(std::ostream& ostream, const Time time) noexcept
  {
    // 10 h < duration
    if (time.nanoseconds > 36000000000000)
    {
      return ostream << _backend::_format_string(time, "elapsed time: %h") << std::endl;
    }

    // 10 min < duration <= 10 h
    if (time.nanoseconds > 600000000000)
    {
      return ostream << _backend::_format_string(time, "elapsed time: %min") << std::endl;
    }

    // 10 s < duration <= 10 m
    if (time.nanoseconds > 10000000000)
    {
      return ostream << _backend::_format_string(time, "elapsed time: %s") << std::endl;
    }

    // 10 ms < duration <= 10 s
    if (time.nanoseconds > 10000000)
    {
      return ostream << _backend::_format_string(time, "elapsed time: %ms") << std::endl;
    }

    // 10 us < duration <= 10 ms
    if (time.nanoseconds > 10000)
    {
      return ostream << _backend::_format_string(time, "elapsed time: %us") << std::endl;
    }

    // duration <= 10 us
    return ostream << _backend::_format_string(time, "elapsed time: %ns") << std::endl;
  }
}
#endif
