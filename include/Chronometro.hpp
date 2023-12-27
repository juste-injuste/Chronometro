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
# include <cstdio>      // for std::sprintf
//---supplementary libraries--------------------------------------------------------------------------------------------
#if not defined(CHRONOMETRO_CLOCK)
# include <type_traits> // for std::conditional
#endif

#if defined(__STDCPP_THREADS__) and not defined(CHRONOMETRO_NOT_THREADSAFE)
# define CHRONOMETRO_THREADSAFE
# include <mutex>       // for std::mutex, std::lock_guard
# define CHRONOMETRO_THREADLOCAL thread_local
# define CHRONOMETRO_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock(MUTEX)
#else
# define CHRONOMETRO_THREADLOCAL
# define CHRONOMETRO_LOCK(MUTEX)
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
  template<Unit U, unsigned D>
  class Time;

  // measure iterations via range-based for-loop
  class Measure;

  // execute following statement/blocks only if its last execution was atleast N milliseconds prior
# define CHRONOMETRO_ONLY_EVERY_MS(N)

  // print time to ostream
  template<Unit U, unsigned D>
  std::ostream& operator<<(std::ostream& ostream, Time<U, D> time) noexcept;

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
# if defined(__clang__) and (__clang_major__ >= 12)
#   define CHRONOMETRO_HOT  [[likely]]
#   define CHRONOMETRO_COLD [[unlikely]]
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define CHRONOMETRO_HOT  [[likely]]
#   define CHRONOMETRO_COLD [[unlikely]]
# else
#   define CHRONOMETRO_HOT
#   define CHRONOMETRO_COLD
# endif

# if defined(__clang__) and ((__clang_major__ > 3) or ((__clang_major__ == 3) and (__clang_minor__ >= 9)))
#   define CHRONOMETRO_NODISCARD [[nodiscard]]
# elif defined(__GNUC__) and (__GNUC__ >= 7)
#   define CHRONOMETRO_NODISCARD [[nodiscard]]
# else
#   define CHRONOMETRO_NODISCARD
# endif

# if defined(__clang__) and (__clang_major__ >= 10)
#   define CHRONOMETRO_NODISCARD_REASON(reason) [[nodiscard(reason)]]
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define CHRONOMETRO_NODISCARD_REASON(reason) [[nodiscard(reason)]]
# else
#   define CHRONOMETRO_NODISCARD_REASON(reason)
# endif

    CHRONOMETRO_THREADLOCAL char _out_buffer[256];
# if defined(CHRONOMETRO_THREADSAFE)
    std::mutex _out_mtx;
# endif

# if defined(CHRONOMETRO_WARNINGS)
    CHRONOMETRO_THREADLOCAL char _wrn_buffer[256];
# if defined(CHRONOMETRO_THREADSAFE)
    std::mutex _wrn_mtx;
# endif

#   define CHRONOMETRO_WARNING(...)                                          \
      [&](const char* caller){                                               \
        sprintf(_backend::_wrn_buffer, __VA_ARGS__);                         \
        CHRONOMETRO_LOCK(_backend::_wrn_mtx);                                \
        Global::wrn << caller << ": " << _backend::_wrn_buffer << std::endl; \
      }(__func__)
# else
#   define CHRONOMETRO_WARNING(...) void(0)
# endif

    template<Unit U, unsigned D>
    std::string _time_as_string(Time<U, D> time, double factor)
    {
      static_assert(D <= 3, "too many decimals requested");
      static const char* format[] = {"%.0f", "%.1f", "%.2f", "%.3f"};

      static CHRONOMETRO_THREADLOCAL char buffer[32];

      std::sprintf(buffer, format[D], static_cast<double>(time.nanoseconds.count())/factor);

      return buffer;
    }

    template<Unit U, unsigned D>
    std::string _format_time(Time<U, D> time, std::string&& format) noexcept
    {
      auto position = format.rfind("%ms");
      while (position != std::string::npos)
      {
        format.replace(position, 1, _time_as_string(time, 1000000.0) + ' ');
        position = format.find("%ms");
      }

      position = format.rfind("%us");
      while (position != std::string::npos)
      {
        format.replace(position, 1, _time_as_string(time, 1000.0) + ' ');
        position = format.find("%us");
      }

      position = format.rfind("%ns");
      while (position != std::string::npos)
      {
        format.replace(position, 1, _time_as_string(time, 1.0) + ' ');
        position = format.find("%ns");
      }

      position = format.rfind("%s");
      while (position != std::string::npos)
      {
        format.replace(position, 1, _time_as_string(time, 1000000000.0) + ' ');
        position = format.find("%s");
      }

      position = format.rfind("%min");
      while (position != std::string::npos)
      {
        format.replace(position, 1, _time_as_string(time, 60000000000.0) + ' ');
        position = format.find("%min");
      }

      position = format.rfind("%h");
      while (position != std::string::npos)
      {
        format.replace(position, 1, _time_as_string(time, 3600000000000.0) + ' ');
        position = format.find("%h");
      }

      return std::move(format);
    }

    template<Unit U, unsigned D>
    std::string _format_lap(Time<U, D> time, std::string&& format, unsigned iteration) noexcept
    {
      auto position = format.find("%#");
      while (position != std::string::npos)
      {
        format.replace(position, 2, std::to_string(iteration));
        position = format.rfind("%#");
      }

      return _format_time(time, std::move(format));
    }

    template<Unit U, unsigned D>
    std::string _format_tot(Time<U, D> time, std::string&& format, unsigned iterations) noexcept
    {
      format = _format_time(time, std::move(format));

      auto position = format.rfind("%D");
      while (position != std::string::npos)
      {
        format.erase(position + 1, 1);
        position = format.find("%D");
      }

      return _format_time(Time<U, 3>{time.nanoseconds/iterations}, std::move(format));
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  class Stopwatch final
  {
  public:
    template<Unit U = Unit::automatic, unsigned D = 0>
    CHRONOMETRO_NODISCARD_REASON("lap: not using the return value makes no sens")
    inline // display and return lap time
    auto lap()     noexcept -> Time<U, D>;

    template<Unit U = Unit::automatic, unsigned D = 0>
    CHRONOMETRO_NODISCARD_REASON("split: not using the return value makes no sens")
    inline // display and return split time
    auto split()   noexcept -> Time<U, D>;

    inline // pause time measurement
    void pause()   noexcept;

    inline // reset measured times
    void reset()   noexcept;

    inline // unpause time measurement
    void unpause() noexcept;
  private:
    bool                     _is_paused    = false;
    std::chrono::nanoseconds _duration_tot = {};
    std::chrono::nanoseconds _duration_lap = {};
    Clock::time_point        _previous     = Clock::now();
  };

  template<Unit U = Unit::ms, unsigned D = 0>
  class Time final
  {
  public:
    std::chrono::nanoseconds nanoseconds;
  };

  class Measure final
  {
  public:
    // measure one iteration
    Measure() noexcept = default;

    inline // measure iterations
    Measure(unsigned iterations) noexcept;

    inline // measure iterations with iteration message
    Measure(unsigned iterations, const char* lap_format) noexcept;

    inline // measure iterations with iteration message and custom total message
    Measure(unsigned iterations, const char* lap_format, const char* total_format) noexcept;

    inline
    void pause() noexcept;

    inline
    void unpause() noexcept;
  private:
    class View;
    const unsigned _iterations   = 1;
    unsigned       _iters_left   = _iterations;
    const char*    _lap_format   = nullptr;
    const char*    _total_format = "total elapsed time: %ms";
    Stopwatch      _stopwatch;
  public: // iterator stuff
    inline auto begin()                    noexcept -> Measure&;
    inline auto end()                const noexcept -> Measure;
    inline View operator*()                noexcept;
    inline void operator++()               noexcept;
    inline bool operator!=(const Measure&) noexcept;
    inline      operator bool()            noexcept;
  };

  class Measure::View
  {
  public:
    const unsigned iteration;

    void pause()   noexcept { _measurement.pause(); }

    void unpause() noexcept { _measurement.unpause(); }
  private:
    View(unsigned current_iteration, Measure& measurement) noexcept;
    Measure& _measurement;
  friend class Measure;
  };

# undef  CHRONOMETRO_MEASURE
# define CHRONOMETRO_MEASURE(...)                                                      \
    for (Chronometro::Measure _measurement{__VA_ARGS__}; _measurement; ++_measurement)

# undef  CHRONOMETRO_ONLY_EVERY_MS
# define CHRONOMETRO_ONLY_EVERY_MS(N)                                 \
    if ([]{                                                           \
      static_assert((N) > 0, "N must be a non-zero positive number"); \
      static Chronometro::Clock::time_point _previous = {};           \
      auto _target = std::chrono::nanoseconds{(N)*1000000};           \
      if ((Chronometro::Clock::now() - _previous) > _target)          \
      {                                                               \
        _previous = Chronometro::Clock::now();                        \
        return true;                                                  \
      }                                                               \
      return false;                                                   \
    }())
//----------------------------------------------------------------------------------------------------------------------
  template<Unit U, unsigned D>
  auto Stopwatch::lap() noexcept -> Time<U, D>
  {
    auto now = Clock::now();

    std::chrono::nanoseconds lap_duration = _duration_lap;
    _duration_lap = {};

    if (_is_paused == false) CHRONOMETRO_HOT
    {
      _duration_tot += now - _previous;
      lap_duration  += now - _previous;

      _previous = Clock::now(); // start measurement from here
    }

    return Time<U>{lap_duration};
  }

  template<Unit U, unsigned D>
  auto Stopwatch::split() noexcept -> Time<U, D>
  {
    auto now = Clock::now();

    std::chrono::nanoseconds tot_duration = _duration_tot;

    if (_is_paused == false) CHRONOMETRO_HOT
    {
      tot_duration += now - _previous;
    }

    return Time<U>{tot_duration};
  }

  void Stopwatch::reset() noexcept
  {
    // reset measured time
    _duration_tot = {};
    _duration_lap = {};

    // hot reset if unpaused
    if (_is_paused == false)
    {
      _previous = Clock::now(); // start measurement from here
    }
  }

  void Stopwatch::pause() noexcept
  {
    auto now = Clock::now();

    if (_is_paused == false) CHRONOMETRO_HOT
    {
      _is_paused = true;

      _duration_tot += now - _previous;
      _duration_lap += now - _previous;
    }
  }

  void Stopwatch::unpause() noexcept
  {
    if (_is_paused == true) CHRONOMETRO_HOT
    {
      _is_paused = false;

      _previous  = Clock::now(); // start measurement from here
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  Measure::Measure(unsigned iterations) noexcept :
    _iterations(iterations),
    _total_format((iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(unsigned iterations, const char* lap_format) noexcept :
    _iterations(iterations),
    _lap_format(lap_format),
    _total_format((iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(unsigned iterations, const char* lap_format, const char* total_format) noexcept :
    _iterations(iterations),
    _lap_format(lap_format),
    _total_format(total_format)
  {}

  void Measure::pause() noexcept
  {
    _stopwatch.pause();
  }

  void Measure::unpause() noexcept
  {
    _stopwatch.unpause();
  }

  Measure& Measure::begin() noexcept
  {
    _iters_left = _iterations;

    _stopwatch.reset();

    return *this;
  }

  Measure Measure::end() const noexcept
  {
    return Measure(0);
  }

  Measure::View Measure::operator*() noexcept
  {
    return View(_iterations - _iters_left, *this);
  }

  void Measure::operator++() noexcept
  {
    auto iter_duration = _stopwatch.lap();
    _stopwatch.pause();

    if ((_lap_format != nullptr) and (_lap_format[0] != '\0'))
    {
      CHRONOMETRO_LOCK(_backend::_out_mtx);
      Global::out << _backend::_format_lap(iter_duration, _lap_format, _iterations - _iters_left) << std::endl;
    }

    --_iters_left;
    _stopwatch.unpause();
  }

  bool Measure::operator!=(const Measure&) noexcept
  {
    return operator bool();
  }

  Measure::operator bool() noexcept
  {
    auto duration = _stopwatch.split();

    if (_iters_left) CHRONOMETRO_HOT
    {
      return true;
    }

    if (_total_format) CHRONOMETRO_HOT
    {
      CHRONOMETRO_LOCK(_backend::_out_mtx);
      Global::out << _backend::_format_tot(duration, _total_format, _iterations ? _iterations : 1) << std::endl;
    }

    return false;
  }

  Measure::View::View(unsigned current_iteration, Measure& measurement) noexcept :
    iteration(current_iteration),
    _measurement(measurement)
  {}
//----------------------------------------------------------------------------------------------------------------------
  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::ns> time) noexcept
  {
    return ostream << "elapsed time: " << _backend::_time_as_string(time, 1.0) << " ns" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::us> time) noexcept
  {
    return ostream << "elapsed time: " << _backend::_time_as_string(time, 1000.0) << " us" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::ms> time) noexcept
  {
    return ostream << "elapsed time: " << _backend::_time_as_string(time, 1000000.0) << " ms" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::s> time) noexcept
  {
    return ostream << "elapsed time: " << _backend::_time_as_string(time, 1000000000.0) << " s" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::min> time) noexcept
  {
    return ostream << "elapsed time: " << _backend::_time_as_string(time, 60000000000.0) << " min" << std::endl;
  }

  template<> inline
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::h> time) noexcept
  {
    return ostream << "elapsed time: " << _backend::_time_as_string(time, 3600000000000.0) << " h" << std::endl;
  }

  template<unsigned D>
  std::ostream& operator<<(std::ostream& ostream, Time<Unit::automatic, D> time) noexcept
  {
    // 10 h < duration
    if (time.nanoseconds.count() > 36000000000000) CHRONOMETRO_COLD
    {
      return ostream << Time<Unit::h, D>{time.nanoseconds};
    }

    // 10 min < duration <= 10 h
    if (time.nanoseconds.count() > 600000000000) CHRONOMETRO_COLD
    {
      return ostream << Time<Unit::min, D>{time.nanoseconds};
    }

    // 10 s < duration <= 10 m
    if (time.nanoseconds.count() > 10000000000)
    {
      return ostream << Time<Unit::s, D>{time.nanoseconds};
    }

    // 10 ms < duration <= 10 s
    if (time.nanoseconds.count() > 10000000)
    {
      return ostream << Time<Unit::ms, D>{time.nanoseconds};
    }

    // 10 us < duration <= 10 ms
    if (time.nanoseconds.count() > 10000)
    {
      return ostream << Time<Unit::us, D>{time.nanoseconds};
    }

    // duration <= 10 us
    return ostream << Time<Unit::ns, D>{time.nanoseconds};
  }
//----------------------------------------------------------------------------------------------------------------------
# undef CHRONOMETRO_THREADSAFE
# undef CHRONOMETRO_THREADLOCAL
# undef CHRONOMETRO_LOCK
# undef CHRONOMETRO_HOT
# undef CHRONOMETRO_COLD
# undef CHRONOMETRO_NODISCARD
# undef CHRONOMETRO_NODISCARD_REASON
# undef CHRONOMETRO_WARNING
}
#endif
