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
execution time of code blocks and more. See the included README.MD file for more information.

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef CHRONOMETRO_HPP
#define CHRONOMETRO_HPP
//---necessary libraries------------------------------------------------------------------------------------------------
#include <chrono>       // for std::chrono::steady_clock, std::chrono::high_resolution_clock, std::chrono::nanoseconds
#include <ostream>      // for std::ostream
#include <iostream>     // for std::cout, std::clog, std::endl
#include <string>       // for std::string
#include <utility>      // for std::move
//---supplementary libraries--------------------------------------------------------------------------------------------
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
  // clock used to measure time
#if defined(CHRONOMETRO_CLOCK)
  using Clock = CHRONOMETRO_CLOCK;
#else
  using Clock = std::conditional<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock
  >::type;
#endif

  // measures the time it takes to execute the following statement/block n times, with labels
# define CHRONOMETRO_MEASURE(...)

  // measure elapsed time
  class Stopwatch;

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

  // type returned by Stopwatch::split() and Stopwatch::lap()
  template<Unit U>
  class Time;

  // measure iterations via range-based for-loop
  class Measure;

  // execute following statement/blocks only if its last execution was atleast N milliseconds prior
# define CHRONOMETRO_ONLY_EVERY_MS(N)

  // print time to ostream
  template<Unit U>
  std::ostream& operator<<(std::ostream& ostream, Time<U> time) noexcept;

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream wrn{std::clog.rdbuf()}; // warning ostream
  }

  namespace Version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//---Chronometro library: backend---------------------------------------------------------------------------------------
  namespace _backend
  {
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

# if defined(__GNUC__) and (__GNUC__ >= 7)
#   define CHRONOMETRO_NODISCARD [[nodiscard]]
# elif defined(__clang__) and ((__clang_major__ > 3) or ((__clang_major__ == 3) and (__clang_minor__ >= 9)))
#   define CHRONOMETRO_NODISCARD [[nodiscard]]
# else
#   define CHRONOMETRO_NODISCARD
# endif

#if defined(__GNUC__) and (__GNUC__ >= 10)
#   define CHRONOMETRO_NODISCARD_REASON(reason) [[nodiscard(reason)]]
#elif defined(__clang__) and (__clang_major__ >= 10)
#   define CHRONOMETRO_NODISCARD_REASON(reason) [[nodiscard(reason)]]
#else
#   define CHRONOMETRO_NODISCARD_REASON(reason)
#endif

# if defined(CHRONOMETRO_THREADSAFE)
    thread_local char _out_buffer[256];
    std::mutex _out_mtx;
#   define CHRONOMETRO_OUT_LOCK std::lock_guard<std::mutex> _out_lock{_backend::_out_mtx}
# else
    char _log_buffer[256];
#   define CHRONOMETRO_OUT_LOCK
# endif

# if defined(CHRONOMETRO_WARNINGS)
# if defined(CHRONOMETRO_THREADSAFE)
    thread_local char _wrn_buffer[256];
    std::mutex _wrn_mtx;
#   define CHRONOMETRO_WRN_LOCK std::lock_guard<std::mutex> lock{_backend::_wrn_mtx}
# else
    char _log_buffer[256];
#   define CHRONOMETRO_WRN_LOCK
# endif

#   define CHRONOMETRO_WARNING(...)                                          \
      [&](const char* caller){                                               \
        sprintf(_backend::_wrn_buffer, __VA_ARGS__);                         \
        CHRONOMETRO_WRN_LOCK;                                                \
        Global::wrn << caller << ": " << _backend::_wrn_buffer << std::endl; \
      }(__func__)
# else
#   define CHRONOMETRO_WARNING(...) void(0)
# endif

    std::string _format(std::chrono::nanoseconds time, std::string&& format) noexcept
    {
      auto position = format.rfind("%ms");
      if (position != std::string::npos) CHRONOMETRO_HOT
      {
        format.replace(position, 1, std::to_string((time/1000000).count()) + ' ');
      }

      position = format.rfind("%us");
      if (position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(position, 1, std::to_string((time/1000).count()) + ' ');
      }

      position = format.rfind("%ns");
      if (position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(position, 1, std::to_string(time.count()) + ' ');
      }

      position = format.rfind("%s");
      if (position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(position, 1, std::to_string((time/1000000000).count()) + ' ');
      }

      position = format.rfind("%min");
      if (position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(position, 1, std::to_string((time/60000000000).count()) + ' ');
      }

      position = format.rfind("%h");
      if (position != std::string::npos) CHRONOMETRO_COLD
      {
        format.replace(position, 1, std::to_string((time/3600000000000).count()) + ' ');
      }
      
      return std::move(format);
    }

    std::string _format_lap(std::chrono::nanoseconds time, std::string&& format, unsigned iteration) noexcept
    {
      auto position = format.find("%#");
      if (position != std::string::npos) CHRONOMETRO_HOT
      {
        format.replace(position, 2, std::to_string(iteration));
      }
      
      return _format(time, std::move(format));
    }

    std::string _format_total(std::chrono::nanoseconds time, std::string&& format, unsigned iterations) noexcept
    {
      format = _format(time, std::move(format));

      auto position = format.rfind("%D");
      if (position != std::string::npos) CHRONOMETRO_HOT
      {
        format.erase(position + 1, 1);
      }

      return _format(time/iterations, std::move(format));
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
    bool                     _is_paused    = false;
    std::chrono::nanoseconds _duration     = {};
    std::chrono::nanoseconds _duration_lap = {};
    Clock::time_point        _previous     = Clock::now();
    Clock::time_point        _previous_lap = _previous;
  };

  template<Unit U = Unit::ms>
  class Time final
  {
  public:
    std::chrono::nanoseconds nanoseconds;
  };

  class Measure final
  {
  public:
    inline // measure 1 iteration
    Measure() noexcept;

    inline // measure N iterations
    Measure(unsigned N) noexcept;

    inline // measure N iterations with iteration message
    Measure(unsigned N, const char* lap_format) noexcept;

    inline // measure N iterations with iteration message and custom total message
    Measure(unsigned N, const char* lap_format, const char* total_format) noexcept;
  private:
    std::chrono::nanoseconds _iter_duration   = {};
    Clock::time_point        _iter_start_time = Clock::now();
    Clock::time_point        _start_time      = _iter_start_time;
    const unsigned           _iterations      = 1;
    unsigned                 _iters_left      = _iterations;
    const char*              _lap_format      = nullptr;
    const char*              _total_format    = "total elapsed time: %ms [avg = %Dus]";
  public: // iterator stuff
    inline auto begin()                    noexcept -> Measure&;
    inline auto end()                const noexcept -> Measure;
    inline auto operator*()          const noexcept -> unsigned;
    inline void operator++()               noexcept;
    inline bool operator!=(const Measure&) noexcept;
    inline      operator bool()            noexcept;
  };

# undef  CHRONOMETRO_MEASURE
# define CHRONOMETRO_MEASURE(...)                                                      \
    for (Chronometro::Measure _measurement{__VA_ARGS__}; _measurement; ++_measurement)

# undef  CHRONOMETRO_ONLY_EVERY_MS
# define CHRONOMETRO_ONLY_EVERY_MS(N)                               \
    if ([]{                                                         \
      static_assert(N > 0, "N must be a non-zero positive number"); \
      static Chronometro::Clock::time_point _previous = {};         \
      auto _target = std::chrono::nanoseconds{N*1000000};           \
      if ((Chronometro::Clock::now() - _previous) > _target)        \
      {                                                             \
        _previous = Chronometro::Clock::now();                      \
        return true;                                                \
      }                                                             \
      return false;                                                 \
    }())
//----------------------------------------------------------------------------------------------------------------------
  template<Unit U>
  auto Stopwatch::lap() noexcept -> Time<U>
  {
    // measure current time
    const auto now = Clock::now();

    std::chrono::nanoseconds nanoseconds = _duration_lap;

    if (_is_paused == false) CHRONOMETRO_HOT
    {
      // save elapsed times
      _duration   += now - _previous;
      nanoseconds += now - _previous_lap;

      // reset measured time
      _duration_lap = Clock::duration{};
      _previous     = Clock::now();
      _previous_lap = _previous;
    }
    else CHRONOMETRO_WARNING("cannot be measured, must not be paused");

    return Time<U>{nanoseconds};
  }

  template<Unit U>
  auto Stopwatch::split() noexcept -> Time<U>
  {
    // measure current time
    const auto now = Clock::now();

    std::chrono::nanoseconds nanoseconds = _duration;

    if (_is_paused == false) CHRONOMETRO_HOT
    {
      // save elapsed times
      _duration     += now - _previous;
      _duration_lap += now - _previous_lap;

      nanoseconds   = _duration;

      // save time point
      _previous     = Clock::now();
      _previous_lap = _previous;
    }
    else CHRONOMETRO_WARNING("cannot be measured, must not be paused");

    return Time<U>{nanoseconds};
  }

  void Stopwatch::pause() noexcept
  {
    // measure current time
    const auto now = Clock::now();

    // add elapsed time up to now if not paused
    if (_is_paused == false) CHRONOMETRO_HOT
    {
      _is_paused = true;

      // save elapsed times
      _duration     += now - _previous;
      _duration_lap += now - _previous_lap;
    }
    else CHRONOMETRO_WARNING("cannot be paused further, is already paused");
  }

  void Stopwatch::reset() noexcept
  {
    // reset measured time
    _duration     = Clock::duration{};
    _duration_lap = Clock::duration{};

    // hot reset if unpaused
    if (_is_paused == false)
    {
      _previous     = Clock::now();
      _previous_lap = _previous;
    }
  }

  void Stopwatch::unpause() noexcept
  {
    if (_is_paused == true) CHRONOMETRO_HOT
    {
      _is_paused = false;

      // reset measured time
      _previous     = Clock::now();
      _previous_lap = _previous;
    }
    else CHRONOMETRO_WARNING("is already unpaused");
  }

  Measure::Measure() noexcept :
    _total_format("total elapsed time: %ms")
  {}

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

  Measure& Measure::begin() noexcept
  {
    _iters_left      = _iterations;
    
    _iter_duration   = {};
    _iter_start_time = Clock::now();
    _start_time      = _iter_start_time;

    return *this;
  }

  Measure Measure::end() const noexcept
  {
    return Measure(0);
  }

  unsigned Measure::operator*() const noexcept
  {
    return _iterations - _iters_left;
  }

  void Measure::operator++() noexcept
  {
    _iter_duration += Clock::now() - _iter_start_time;

    if ((_lap_format != nullptr) and (_lap_format[0] != '\0'))
    {
      auto time      = Time<>{_iter_duration};
      auto iteration = _iterations - _iters_left;
      CHRONOMETRO_OUT_LOCK;
      Global::out << _backend::_format_lap(time.nanoseconds, _lap_format, iteration) << std::endl;
    }

    --_iters_left;

    _iter_duration   = {};
    _iter_start_time = Clock::now();
  }

  bool Measure::operator!=(const Measure&) noexcept
  {
    return operator bool();
  }

  Measure::operator bool() noexcept
  {
    auto now = Clock::now();

    if (_iters_left) CHRONOMETRO_HOT
    {
      return true;
    }

    if (_total_format) CHRONOMETRO_HOT
    {
      auto time = Time<>{std::chrono::nanoseconds(now - _start_time)};
      CHRONOMETRO_OUT_LOCK;
      Global::out << _backend::_format_total(time.nanoseconds, _total_format, _iterations) << std::endl;
    }

    return false;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::ns> time) noexcept
  {
    return ostream << "elapsed time: " << time.nanoseconds.count() << " ns" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::us> time) noexcept
  {
    return ostream << "elapsed time: " << (time.nanoseconds/1000).count() << " us" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::ms> time) noexcept
  {
    return ostream << "elapsed time: " << (time.nanoseconds/1000000).count() << " ms" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::s> time) noexcept
  {
    return ostream << "elapsed time: " << (time.nanoseconds/1000000000).count() << " s" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::min> time) noexcept
  {
    return ostream << "elapsed time: " << (time.nanoseconds/60000000000).count() << " min" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::h> time) noexcept
  {
    return ostream << "elapsed time: " << (time.nanoseconds/3600000000000).count() << " h" << std::endl;
  }

  template<>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::automatic> time) noexcept
  {
    // 10 h < duration
    if (time.nanoseconds.count() > 36000000000000) CHRONOMETRO_COLD
    {
      return ostream << Time<Unit::h>{time.nanoseconds};
    }

    // 10 min < duration <= 10 h
    if (time.nanoseconds.count() > 600000000000) CHRONOMETRO_COLD
    {
      return ostream << Time<Unit::min>{time.nanoseconds};
    }

    // 10 s < duration <= 10 m
    if (time.nanoseconds.count() > 10000000000)
    {
      return ostream << Time<Unit::s>{time.nanoseconds};
    }

    // 10 ms < duration <= 10 s
    if (time.nanoseconds.count() > 10000000)
    {
      return ostream << Time<Unit::ms>{time.nanoseconds};
    }

    // 10 us < duration <= 10 ms
    if (time.nanoseconds.count() > 10000)
    {
      return ostream << Time<Unit::us>{time.nanoseconds};
    }

    // duration <= 10 us
    return ostream << Time<Unit::ns>{time.nanoseconds};
  }
//----------------------------------------------------------------------------------------------------------------------
# undef CHRONOMETRO_THREADSAFE
# undef CHRONOMETRO_HOT
# undef CHRONOMETRO_COLD
# undef CHRONOMETRO_NODISCARD
# undef CHRONOMETRO_NODISCARD_REASON
# undef CHRONOMETRO_OUT_LOCK
# undef CHRONOMETRO_WRN_LOCK
# undef CHRONOMETRO_WARNING
}
#endif
