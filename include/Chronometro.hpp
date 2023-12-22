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
//---necessary standard libraries---------------------------------------------------------------------------------------
#include <chrono>       // for std::chrono::high_resolution_clock and std::chrono::nanoseconds
#include <ostream>      // for std::ostream
#include <iostream>     // for std::cout, std::clog, std::endl
#include <string>       // for std::string
#include <utility>      // for std::move
//---supplementary standard libraries-----------------------------------------------------------------------------------
#if not defined(CHRONOMETRO_CLOCK)
# include <type_traits> // for std::conditional
#endif
#if defined(CHRONOMETRO_WARNINGS)
#if defined(__STDCPP_THREADS__) and not defined(CHRONOMETRO_NOT_THREADSAFE)
# define CHRONOMETRO_THREADSAFE
# include <mutex>       // for std::mutex, std::lock_guard
#endif
# include <cstdio>      // for std::sprintf
#endif
//---Chronometro library------------------------------------------------------------------------------------------------
namespace Chronometro
{
  namespace Version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
  
  // clock used to measure time
#if defined(CHRONOMETRO_CLOCK) 
  using Clock = CHRONOMETRO_CLOCK;
#else
  using Clock = std::conditional<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock
  >::type;
# define CHRONOMETRO_CLOCK Clock
#endif

  // measures the time it takes to execute the following statement/block n times, with labels
# define CHRONOMETRO_MEASURE(...)

  // units in which Time<> can be displayed
  enum class Unit
  {
    ns,       // nanoseconds
    us,       // microseconds
    ms,       // milliseconds
    s,        // seconds
    min,      // minutes
    h,        // hours
    automatic // deduce appropriate unit automatically
  };

  // measure elapsed time
  class Stopwatch;

  // type returned by Stopwatch::split() and Stopwatch::lap()
  template<Unit U>
  struct Time;

  // measure iterations via range-based for-loop
  class Measure;

  // execute following statement/blocks only if its last execution was atleast N milliseconds prior
# define CHRONOMETRO_ONLY_EVERY_MS(N)

  // print time to ostream
  template<Unit U> inline
  std::ostream& operator<<(std::ostream& ostream, Time<U> time) noexcept;

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream wrn{std::clog.rdbuf()}; // warning ostream
  }
//---Chronometro library: backend---------------------------------------------------------------------------------------
  namespace _backend
  {
# if defined(__GNUC__) and (__GNUC__ >= 7)
#   define CHRONOMETRO_NODISCARD [[nodiscard]]
# elif defined(__clang__) and ((__clang_major__ > 3) or ((__clang_major__ == 3) and (__clang_minor__ >= 9)))
#   define CHRONOMETRO_NODISCARD [[nodiscard]]
# else
#   define CHRONOMETRO_NODISCARD
# endif

#if defined(__GNUC__) && (__GNUC__ >= 10)
#   define CHRONOMETRO_NODISCARD_REASON(reason) [[nodiscard(reason)]]
#elif defined(__clang__) && (__clang_major__ >= 10)
#   define CHRONOMETRO_NODISCARD_REASON(reason) [[nodiscard(reason)]]
#else
#   define CHRONOMETRO_NODISCARD_REASON(reason)
#endif

# if defined(__GNUC__) and (__GNUC__ >= 10)
#   define CHRONOMETRO_HOT  [[likely]]
#   define CHRONOMETRO_COLD [[unlikely]]
# elif defined(__clang__) and (__clang_major__ >= 12)
#   define CHRONOMETRO_HOT  [[likely]]
#   define CHRONOMETRO_COLD [[unlikely]]
# else
#   define CHRONOMETRO_HOT
#   define CHRONOMETRO_COLD
# endif

# if defined(CHRONOMETRO_WARNINGS)
# if defined (CHRONOMETRO_THREADSAFE)
    thread_local char _wrn_buffer[256];
    std::mutex _wrn_mtx;
#   define CHRONOMETRO_WRN_LOCK std::lock_guard<std::mutex> lock{_backend::_wrn_mtx}
# else
    char _log_buffer[256];
#   define CHRONOMETRO_WRN_LOCK
# endif
    
#   define CHRONOMETRO_WARNING(...)                  \
      [&](const char* caller){                       \
        sprintf(_backend::_wrn_buffer, __VA_ARGS__); \
        CHRONOMETRO_WRN_LOCK;                        \
        Global::wrn << caller << ": "                \
        << _backend::_wrn_buffer << std::endl;       \
      }(__func__)
# else
#   define CHRONOMETRO_WARNING(...) void(0)
# endif

    template<Unit U>
    std::string _format(Time<U> time, std::string&& format) noexcept
    {
      auto time_position = format.rfind("%ms");
      if (time_position != std::string::npos) CHRONOMETRO_HOT
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000000) + ' ');
      }

      time_position = format.rfind("%us");
      if (time_position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000) + ' ');
      }

      time_position = format.rfind("%ns");
      if (time_position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds) + ' ');
      }

      time_position = format.rfind("%s");
      if (time_position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000000000) + ' ');
      }

      time_position = format.rfind("%min");
      if (time_position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/60000000000) + ' ');
      }

      time_position = format.rfind("%h");
      if (time_position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/3600000000000) + ' ');
      }
      
      return std::move(format);
    }

    template<Unit U>
    std::string _format(Time<U> time, std::string&& format, const unsigned iteration) noexcept
    {
      auto iteration_position = format.find("%#");
      if (iteration_position != std::string::npos)
      {
        format.replace(iteration_position, 2, std::to_string(iteration));
      }
      
      return _format(time, std::move(format));
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  class Stopwatch final
  {
  public:
    template<Unit U = Unit::automatic>
    CHRONOMETRO_NODISCARD_REASON("lap: not using the return value makes no sens")
    inline // display and return lap time
    auto lap()     noexcept -> Time<U>;
    
    template<Unit U = Unit::automatic>
    CHRONOMETRO_NODISCARD_REASON("split: not using the return value makes no sens")
    inline // display and return split time
    auto split()   noexcept -> Time<U>;

    inline // pause time measurement
    void pause()   noexcept;

    inline // reset measured times
    void reset()   noexcept;

    inline // unpause time measurement
    void unpause() noexcept;
  private:
    bool                     is_paused    = false;
    std::chrono::nanoseconds duration     = {};
    std::chrono::nanoseconds duration_lap = {};
    Clock::time_point        previous     = Clock::now();
    Clock::time_point        previous_lap = previous;
  };
  
  template<Unit U>
  struct Time
  {
    std::chrono::nanoseconds::rep nanoseconds;
  };

  class Measure final
  {
  public:
    // measure 1 iteration
    Measure() noexcept = default;
    
    inline // measure N iterations
    Measure(unsigned N) noexcept;

    inline // measure N iterations with iteration message
    Measure(unsigned N, const char* lap_format) noexcept;

    inline // measure N iterations with iteration message and custom total message
    Measure(unsigned N, const char* lap_format, const char* total_format) noexcept;
  private:
    Stopwatch      _stopwatch;
    const unsigned _iterations      = 1;
    unsigned       _iterations_left = _iterations;
    const char*    _lap_format      = nullptr;
    const char*    _total_format    = "total elapsed time: %ms";
  public: // iterator stuff
    inline auto begin()                    noexcept -> Measure&;
    inline auto end()                const noexcept -> Measure;
    inline auto operator*()          const noexcept -> unsigned;
    inline void operator++()               noexcept;
    inline bool operator!=(const Measure&) noexcept;
    inline      operator bool()            noexcept;
  };

# undef  CHRONOMETRO_MEASURE
# define CHRONOMETRO_MEASURE(...) for (Chronometro::Measure measurement{__VA_ARGS__}; measurement; ++measurement)

  template<Unit U>
  auto Stopwatch::lap() noexcept -> Time<U>
  {
    // measure current time
    const auto now = Clock::now();

    std::chrono::nanoseconds::rep nanoseconds = duration_lap.count();

    if (is_paused == false) CHRONOMETRO_HOT
    {
      // save elapsed times
      duration    += now - previous;
      nanoseconds += (now - previous_lap).count();
      
      // reset measured time
      duration_lap = Clock::duration{};
      previous     = Clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_WARNING("cannot be measured, must not be paused");

    return Time<U>{nanoseconds};
  }

  template<Unit U>
  auto Stopwatch::split() noexcept -> Time<U>
  {
    // measure current time
    const auto now = Clock::now();

    std::chrono::nanoseconds::rep nanoseconds = duration.count();

    if (is_paused == false) CHRONOMETRO_HOT
    {
      // save elapsed times
      duration     += now - previous;
      duration_lap += now - previous_lap;
      
      nanoseconds   = duration.count();

      // save time point
      previous     = Clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_WARNING("cannot be measured, must not be paused");
    
    return Time<U>{nanoseconds};
  }

  void Stopwatch::pause() noexcept
  {
    // measure current time
    const auto now = Clock::now();

    // add elapsed time up to now if not paused
    if (is_paused == false) CHRONOMETRO_HOT
    {
      is_paused = true;

      // save elapsed times
      duration     += now - previous;
      duration_lap += now - previous_lap;
    }
    else CHRONOMETRO_WARNING("cannot be paused further, is already paused");
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
    if (is_paused == true) CHRONOMETRO_HOT
    {
      // unpause
      is_paused = false;

      // reset measured time
      previous     = Clock::now();
      previous_lap = previous;
    }
    else CHRONOMETRO_WARNING("is already unpaused");
  }

  Measure& Measure::begin() noexcept
  {
    _iterations_left = _iterations;
    _stopwatch.reset();
    return *this;
  }

  Measure Measure::end() const noexcept
  {
    return Measure(0);
  }
  
  Measure::Measure(unsigned N) noexcept :
    _iterations(N)
  {}
  
  Measure::Measure(unsigned N, const char* lap_format) noexcept :
    _iterations(N),
    _lap_format(lap_format)
  {}
  
  Measure::Measure(unsigned N, const char* lap_format, const char* total_format) noexcept :
    _iterations(N),
    _lap_format(lap_format),
    _total_format(total_format)
  {}
  
  unsigned Measure::operator*() const noexcept
  {
    return _iterations - _iterations_left;
  }

  void Measure::operator++() noexcept
  {
    auto lap_time = _stopwatch.lap();
    
    _stopwatch.pause();

    if ((_lap_format != nullptr) && (_lap_format[0] != '\0'))
    {
      Global::out << _backend::_format(lap_time, _lap_format, _iterations - _iterations_left) << std::endl;
    }

    --_iterations_left;

    _stopwatch.unpause();
  }

  bool Measure::operator!=(const Measure&) noexcept
  {
    return operator bool();
  }

  Measure::operator bool() noexcept
  {     
    if (_iterations_left) CHRONOMETRO_HOT
    {
      return true;
    }

    auto time = _stopwatch.split();
    if (_total_format) CHRONOMETRO_HOT
    {
      Global::out << _backend::_format(time, _total_format) << std::endl;
    }

    return false;
  }

# undef  CHRONOMETRO_ONLY_EVERY_MS
# define CHRONOMETRO_ONLY_EVERY_MS(N)                               \
    if ([]{                                                         \
      static_assert(N > 0, "N must be a non-zero positive number"); \
      static Chronometro::Clock::time_point previous = {};          \
      auto target = std::chrono::nanoseconds{N*1000000};            \
      if ((Chronometro::Clock::now() - previous) > target)          \
      {                                                             \
        previous = Chronometro::Clock::now();                       \
        return true;                                                \
      }                                                             \
      return false;                                                 \
    }())

  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::ns> time) noexcept
  {
    return ostream << "elapsed time: " << time.nanoseconds << " ns" << std::endl;
  }
  
  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::us> time) noexcept
  {
    return ostream << "elapsed time: " << time.nanoseconds/1000 << " us" << std::endl;
  }
  
  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::ms> time) noexcept
  {
    return ostream << "elapsed time: " << time.nanoseconds/1000000 << " ms" << std::endl;
  }
  
  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::s> time) noexcept
  {
    return ostream << "elapsed time: " << time.nanoseconds/1000000000 << " s" << std::endl;
  }
  
  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::min> time) noexcept
  {
    return ostream << "elapsed time: " << time.nanoseconds/60000000000 << " min" << std::endl;
  }
  
  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::h> time) noexcept
  {
    return ostream << "elapsed time: " << time.nanoseconds/3600000000000 << " h" << std::endl;
  }

  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::automatic> time) noexcept
  {
    // 10 h < duration
    if (time.nanoseconds > 36000000000000) CHRONOMETRO_COLD
    {
      return ostream << Time<Unit::h>{time.nanoseconds};
    }

    // 10 min < duration <= 10 h
    if (time.nanoseconds > 600000000000) CHRONOMETRO_COLD
    {
      return ostream << Time<Unit::min>{time.nanoseconds};
    }

    // 10 s < duration <= 10 m
    if (time.nanoseconds > 10000000000)
    {
      return ostream << Time<Unit::s>{time.nanoseconds};
    }

    // 10 ms < duration <= 10 s
    if (time.nanoseconds > 10000000)
    {
      return ostream << Time<Unit::ms>{time.nanoseconds};
    }

    // 10 us < duration <= 10 ms
    if (time.nanoseconds > 10000)
    {
      return ostream << Time<Unit::us>{time.nanoseconds};
    }

    // duration <= 10 us
    return ostream << Time<Unit::ns>{time.nanoseconds};
  }

// cleanup of internal macros
# undef CHRONOMETRO_THREADSAFE
# undef CHRONOMETRO_NODISCARD
# undef CHRONOMETRO_NODISCARD_REASON
# undef CHRONOMETRO_HOT
# undef CHRONOMETRO_COLD
# undef CHRONOMETRO_WRN_LOCK
# undef CHRONOMETRO_WARNING
}
#endif
