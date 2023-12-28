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

  // measures the time it takes to execute the following statement/block n times, with labels
# define CHRONOMETRO_MEASURE(...)

  // type returned by Stopwatch::split() and Stopwatch::lap()
  template<Unit U, unsigned D>
  class Time;

  // measure elapsed time
  class Stopwatch;

  // measure iterations via range-based for-loop
  class Measure;

  // execute following statement/blocks only if its last execution was atleast N milliseconds prior
# define CHRONOMETRO_ONLY_EVERY_MS(N)

  // print time to ostream
  template<Unit U, unsigned D> inline
  std::ostream& operator<<(std::ostream& ostream, Time<U, D> time) noexcept;

  namespace Global
  {
    static std::ostream out{std::cout.rdbuf()}; // output ostream
    static std::ostream wrn{std::clog.rdbuf()}; // warning ostream
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
#   define CHRONOMETRO_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define CHRONOMETRO_CLANG_IGNORE(WARNING, ...)          \
      CHRONOMETRO_PRAGMA(clang diagnostic push)            \
      CHRONOMETRO_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                          \
      CHRONOMETRO_PRAGMA(clang diagnostic pop)

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define CHRONOMETRO_HOT  CHRONOMETRO_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define CHRONOMETRO_COLD CHRONOMETRO_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define CHRONOMETRO_HOT  [[likely]]
#   define CHRONOMETRO_COLD [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define CHRONOMETRO_HOT  [[likely]]
#   define CHRONOMETRO_COLD [[unlikely]]
# else
#   define CHRONOMETRO_HOT
#   define CHRONOMETRO_COLD
# endif

// support from clang 3.9.0 and GCC 7.1 onward
# if defined(__clang__) and ((__clang_major__ > 3) or ((__clang_major__ == 3) and (__clang_minor__ >= 9)))
# if __cplusplus < 201703L
#   define CHRONOMETRO_NODISCARD    CHRONOMETRO_CLANG_IGNORE("-Wc++1z-extensions", [[nodiscard]])                               \
# else
#   define CHRONOMETRO_NODISCARD    [[nodiscard]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 7)
#   define CHRONOMETRO_NODISCARD    [[nodiscard]]
# else
#   define CHRONOMETRO_NODISCARD
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define CHRONOMETRO_NODISCARD_REASON(REASON) CHRONOMETRO_CLANG_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define CHRONOMETRO_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define CHRONOMETRO_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define CHRONOMETRO_NODISCARD_REASON(REASON) CHRONOMETRO_NODISCARD
# endif

#if defined(CHRONOMETRO_THREADSAFE)
# define CHRONOMETRO_THREADLOCAL     thread_local
# define CHRONOMETRO_MAKE_MUTEX(...) static std::mutex __VA_ARGS__
# define CHRONOMETRO_LOCK(MUTEX)     std::lock_guard<decltype(MUTEX)> _lock(MUTEX)
#else
# define CHRONOMETRO_THREADLOCAL
# define CHRONOMETRO_MAKE_MUTEX(...)
# define CHRONOMETRO_LOCK(MUTEX)      void(0)
#endif

    CHRONOMETRO_MAKE_MUTEX(_out_mtx);

    template<Unit>
    struct _unit_helper;

    template<>
    struct _unit_helper<Unit::ns>
    {
      static constexpr const char* label = "ns";
      static constexpr double      factor = 1.0;
    };

    template<>
    struct _unit_helper<Unit::us>
    {
      static constexpr const char* label = "us";
      static constexpr double      factor = 1000.0;
    };

    template<>
    struct _unit_helper<Unit::ms>
    {
      static constexpr const char* label = "ms";
      static constexpr double      factor = 1000000.0;
    };

    template<>
    struct _unit_helper<Unit::s>
    {
      static constexpr const char* label = "s";
      static constexpr double      factor = 1000000000.0;
    };

    template<>
    struct _unit_helper<Unit::min>
    {
      static constexpr const char* label = "min";
      static constexpr double      factor = 60000000000.0;
    };

    template<>
    struct _unit_helper<Unit::h>
    {
      static constexpr const char* label = "h";
      static constexpr double      factor = 3600000000000.0;
    };

    template<Unit U, unsigned D> inline
    std::string _time_as_string(Time<U, D> time)
    {
      static_assert(D <= 3, "too many decimals requested");
      static const char* format[] = {"%.0f %s", "%.1f %s", "%.2f %s", "%.3f %s"};

      static CHRONOMETRO_THREADLOCAL char buffer[32];

      double ajusted_time = static_cast<double>(time.nanoseconds.count())/_unit_helper<U>::factor;
      std::sprintf(buffer, format[D], ajusted_time, _unit_helper<U>::label);

      return buffer;
    }

    template<unsigned D> inline
    std::string _time_as_string(Time<Unit::automatic, D> time)
    {
      // 10 h < duration
      if (time.nanoseconds.count() > 36000000000000) CHRONOMETRO_COLD
      {
        return _time_as_string(Time<Unit::h, D>{time.nanoseconds});
      }

      // 10 min < duration <= 10 h
      if (time.nanoseconds.count() > 600000000000) CHRONOMETRO_COLD
      {
        return _time_as_string(Time<Unit::min, D>{time.nanoseconds});
      }

      // 10 s < duration <= 10 m
      if (time.nanoseconds.count() > 10000000000)
      {
        return _time_as_string(Time<Unit::s, D>{time.nanoseconds});
      }

      // 10 ms < duration <= 10 s
      if (time.nanoseconds.count() > 10000000)
      {
        return _time_as_string(Time<Unit::ms, D>{time.nanoseconds});
      }

      // 10 us < duration <= 10 ms
      if (time.nanoseconds.count() > 10000)
      {
        return _time_as_string(Time<Unit::us, D>{time.nanoseconds});
      }

      // duration <= 10 us
      return _time_as_string(Time<Unit::ns, D>{time.nanoseconds});
    }

    template<Unit U, unsigned D> inline
    std::string _format_time(Time<U, D> time, std::string&& format) noexcept
    {
      static const std::string unit_specifiers[] = {"%ns", "%us", "%ms", "%s", "%min", "%h"};

      for (unsigned k = 0; k < 6; ++k)
      {
        const auto& unit_specifier = unit_specifiers[k];
        auto  position             = format.rfind(unit_specifier);
        while (position != std::string::npos)
        {
          format.replace(position, unit_specifier.length(), _time_as_string(time));
          position = format.find(unit_specifier);
        }
      }

      return std::move(format);
    }

    template<Unit U, unsigned D> inline
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

    template<Unit U, unsigned D> inline
    std::string _format_tot(Time<U, D> time, std::string&& format, unsigned iterations) noexcept
    {
      format = _format_time(time, std::move(format));

      auto position = format.rfind("%D");
      while (position != std::string::npos)
      {
        format.erase(position + 1, 1);
        position = format.find("%D");
      }

      if (iterations == 0) CHRONOMETRO_COLD
      {
        iterations = 1;
      }

      return _format_time(Time<U, 3>{time.nanoseconds/iterations}, std::move(format));
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  template<Unit U = Unit::ms, unsigned D = 0>
  class Time final
  {
  public:
    std::chrono::nanoseconds nanoseconds;

    template<Unit U_>
    Time<U_, D> unit()     noexcept { return reinterpret_cast<Time<U_, D>&>(*this); }

    template<unsigned D_>
    Time<U, D_> decimals() noexcept { return reinterpret_cast<Time<U, D_>&>(*this); }
  };
  
  class Stopwatch final
  {
  public:
    // scoped pause/unpause
    class Guard;

    CHRONOMETRO_NODISCARD_REASON("lap: not using the return value makes no sens")
    inline // display and return lap time
    auto lap()     noexcept -> Time<>;

    CHRONOMETRO_NODISCARD_REASON("split: not using the return value makes no sens")
    inline // display and return split time
    auto split()   noexcept -> Time<>;

    inline // reset measured times
    void reset()   noexcept;

    inline // pause time measurement
    void pause()   noexcept;

    inline // unpause time measurement
    void unpause() noexcept;

    inline // scoped pause/unpause
    auto guard()   noexcept -> Guard;
  private:
    bool                     _is_paused    = false;
    std::chrono::nanoseconds _duration_tot = {};
    std::chrono::nanoseconds _duration_lap = {};
    Clock::time_point        _previous     = Clock::now();
  };

  class Measure final
  {
  public:
    // measure one iteration
    Measure() noexcept = default;

    inline // measure iterations
    Measure(unsigned iterations) noexcept;

    inline // measure iterations with iteration message
    Measure(unsigned iterations, const char* iteration_format) noexcept;

    inline // measure iterations with iteration message and custom total message
    Measure(unsigned iterations, const char* iteration_format, const char* total_format) noexcept;

    inline
    void pause()   noexcept;

    inline
    void unpause() noexcept;

    inline
    auto guard()   noexcept -> Stopwatch::Guard;
  private:
    class View;
    const unsigned _iterations  = 1;
    unsigned       _iters_left  = _iterations;
    const char*    _iter_format = nullptr;
    const char*    _tot_format  = "total elapsed time: %ms";
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

    inline
    void pause()   noexcept;

    inline
    void unpause() noexcept;

    inline
    auto guard()   noexcept -> Stopwatch::Guard;
  private:
    inline View(unsigned current_iteration, Measure* measurement) noexcept;
    Measure* _measurement;
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
  class Stopwatch::Guard final
  {
  private:
    inline Guard(Stopwatch* stopwatch) noexcept;
    Stopwatch* _stopwatch;
  public:
    inline ~Guard() noexcept;
  friend class Stopwatch;
  };
  
  Stopwatch::Guard::Guard(Stopwatch* stopwatch) noexcept :
    _stopwatch(stopwatch)
  {
    _stopwatch->pause();
  }

  Stopwatch::Guard::~Guard() noexcept
  {
    _stopwatch->unpause();
  }
//----------------------------------------------------------------------------------------------------------------------
  auto Stopwatch::lap() noexcept -> Time<>
  {
    auto now = Clock::now();

    std::chrono::nanoseconds lap_duration = _duration_lap;
    _duration_lap = {};

    if (not _is_paused) CHRONOMETRO_HOT
    {
      _duration_tot += now - _previous;
      lap_duration  += now - _previous;

      _previous = Clock::now(); // start measurement from here
    }

    return Time<>{lap_duration};
  }

  auto Stopwatch::split() noexcept -> Time<>
  {
    auto now = Clock::now();

    std::chrono::nanoseconds tot_duration = _duration_tot;

    if (not _is_paused) CHRONOMETRO_HOT
    {
      tot_duration += now - _previous;

      _duration_lap = {};
      _duration_tot = {};
    }

    return Time<>{tot_duration};
  }

  void Stopwatch::reset() noexcept
  {
    _duration_tot = {};
    _duration_lap = {};

    // hot reset if unpaused
    if (not _is_paused)
    {
      _previous = Clock::now(); // start measurement from here
    }
  }

  void Stopwatch::pause() noexcept
  {
    auto now = Clock::now();

    if (not _is_paused) CHRONOMETRO_HOT
    {
      _is_paused = true;

      _duration_tot += now - _previous;
      _duration_lap += now - _previous;
    }
  }

  void Stopwatch::unpause() noexcept
  {
    if (_is_paused) CHRONOMETRO_HOT
    {
      _is_paused = false;

      _previous  = Clock::now(); // start measurement from here
    }
  }

  auto Stopwatch::guard() noexcept -> Stopwatch::Guard
  {
    return Guard(this);
  }
//----------------------------------------------------------------------------------------------------------------------
  Measure::Measure(unsigned iterations) noexcept :
    _iterations(iterations),
    _tot_format((iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(unsigned iterations, const char* iteration_format) noexcept :
    _iterations(iterations),
    _iter_format((iteration_format[0] == '\0') ? nullptr : iteration_format),
    _tot_format((iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(unsigned iterations, const char* iteration_format, const char* total_format) noexcept :
    _iterations(iterations),
    _iter_format((iteration_format[0] == '\0') ? nullptr : iteration_format),
    _tot_format((total_format[0] == '\0') ? nullptr : total_format)
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

    _stopwatch.unpause();
    _stopwatch.reset();

    return *this;
  }

  Measure Measure::end() const noexcept
  {
    return Measure(0);
  }

  Measure::View Measure::operator*() noexcept
  {
    return View(_iterations - _iters_left, this);
  }

  void Measure::operator++() noexcept
  {
    _stopwatch.pause();
    auto iter_duration = _stopwatch.lap();

    if (_iter_format)
    {
      CHRONOMETRO_LOCK(_backend::_out_mtx);
      Global::out << _backend::_format_lap(iter_duration, _iter_format, _iterations - _iters_left) << std::endl;
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
    _stopwatch.pause();
    if (_iters_left) CHRONOMETRO_HOT
    {
      _stopwatch.unpause();
      return true;
    }

    auto duration = _stopwatch.split();

    if (_tot_format) CHRONOMETRO_HOT
    {
      CHRONOMETRO_LOCK(_backend::_out_mtx);
      Global::out << _backend::_format_tot(duration, _tot_format, _iterations) << std::endl;
    }

    return false;
  }
//----------------------------------------------------------------------------------------------------------------------
  Measure::View::View(unsigned current_iteration, Measure* measurement) noexcept :
    iteration(current_iteration),
    _measurement(measurement)
  {}

  void Measure::View::pause() noexcept
  {
    _measurement->pause();
  }

  void Measure::View::unpause() noexcept
  {
    _measurement->unpause();
  }

  auto Measure::guard() noexcept -> Stopwatch::Guard
  {
    return _stopwatch.guard();
  }

  auto Measure::View::guard() noexcept -> Stopwatch::Guard
  {
    return _measurement->guard();
  }
//----------------------------------------------------------------------------------------------------------------------
  template<Unit U, unsigned D>
  std::ostream& operator<<(std::ostream& ostream, Time<U, D> time) noexcept
  {
    return ostream << "elapsed time: " << _backend::_time_as_string(time) << std::endl;
  }
//----------------------------------------------------------------------------------------------------------------------
# undef CHRONOMETRO_PRAGMA
# undef CHRONOMETRO_CLANG_IGNORE
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
