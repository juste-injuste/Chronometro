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

chz::Stopwatch;

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef _chronometro_hpp
#define _chronometro_hpp
//---necessary libraries------------------------------------------------------------------------------------------------
#include <chrono>   // for std::chrono::steady_clock, std::chrono::high_resolution_clock, std::chrono::nanoseconds
#include <ostream>  // for std::ostream
#include <iostream> // for std::cout, std::endl
#include <string>   // for std::string
#include <utility>  // for std::move
#include <cstdio>   // for std::sprintf
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if not defined(CHRONOMETRO_CLOCK)
# include <type_traits> // for std::conditional
#endif

#if defined(__STDCPP_THREADS__) and not defined(CHRONOMETRO_NOT_THREADSAFE)
# define _backend_CHZ_THREADSAFE
# include <mutex> // for std::mutex, std::lock_guard
#endif
//---Chronometro library------------------------------------------------------------------------------------------------
namespace Chronometro
{
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

  // // type returned by Stopwatch::split() and Stopwatch::lap()
  // template<Unit U, unsigned D>
  // class Time;

  // measure elapsed time
  class Stopwatch;

  // measure iterations via range-based for-loop
  class Measure;

  // execute following statement/blocks only if its last execution was atleast N milliseconds prior
# define CHRONOMETRO_ONLY_EVERY_MS(N)

  // // print time to ostream
  // template<Unit U, unsigned D>
  // std::ostream& operator<<(std::ostream& ostream, Time<U, D> time) noexcept;

  namespace _io
  {
    static std::ostream out{std::cout.rdbuf()}; // output ostream
  }

  namespace _version
  {
    constexpr long major  = 000;
    constexpr long minor  = 001;
    constexpr long patch  = 000;
    constexpr long number = (major * 1000 + minor) * 1000 + patch;
  }
//---Chronometro library: backend---------------------------------------------------------------------------------------
  namespace _backend
  {
# if defined(__clang__)
#   define _backend_CHZ_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _backend_CHZ_IGNORE(WARNING, ...)                \
      _backend_CHZ_PRAGMA(clang diagnostic push)            \
      _backend_CHZ_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                   \
      _backend_CHZ_PRAGMA(clang diagnostic pop)
#endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _backend_CHZ_HOT  _backend_CHZ_IGNORE("-Wc++20-extensions", [[likely]])
#   define _backend_CHZ_COLD _backend_CHZ_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _backend_CHZ_HOT  [[likely]]
#   define _backend_CHZ_COLD [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _backend_CHZ_HOT  [[likely]]
#   define _backend_CHZ_COLD [[unlikely]]
# else
#   define _backend_CHZ_HOT
#   define _backend_CHZ_COLD
# endif

// support from clang 3.9.0 and GCC 5.1 onward
# if defined(__clang__)
#   define _backend_CHZ_NODISCARD __attribute__((warn_unused_result))
# elif defined(__GNUC__)
#   define _backend_CHZ_NODISCARD __attribute__((warn_unused_result))
# else
#   define _backend_CHZ_NODISCARD
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _backend_CHZ_NODISCARD_REASON(REASON) _backend_CHZ_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _backend_CHZ_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _backend_CHZ_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _backend_CHZ_NODISCARD_REASON(REASON) _backend_CHZ_NODISCARD
# endif

#if defined(_backend_CHZ_THREADSAFE)
# undef  _backend_CHZ_THREADSAFE
# define _backend_CHZ_THREADLOCAL         thread_local
# define _backend_CHZ_DECLARE_MUTEX(...)  static std::mutex __VA_ARGS__
# define _backend_CHZ_DECLARE_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock(MUTEX)
#else
# define _backend_CHZ_THREADLOCAL
# define _backend_CHZ_DECLARE_MUTEX(...)
# define _backend_CHZ_DECLARE_LOCK(MUTEX) void(0)
#endif

  // clock used to measure time
#if defined(CHRONOMETRO_CLOCK)
  using _clock = CHRONOMETRO_CLOCK;
#else
  using _clock = std::conditional<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock
  >::type;
#endif

    template<Unit U, unsigned D>
    class Time
    {
    public:
      std::chrono::nanoseconds nanoseconds;

      template<Unit U_, unsigned D_ = D>
      inline // change format
      auto format() noexcept -> Time<U_, D_>;

      template<unsigned D_, Unit U_ = U>
      inline // change format
      auto format() noexcept -> Time<U_, D_>;
    };

    _backend_CHZ_DECLARE_MUTEX(_out_mtx);

    template<Unit>
    struct _unit_helper;

#   define _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION(UNIT, LABEL, FACTOR) \
      template<>                                                     \
      struct _unit_helper<UNIT>                                      \
      {                                                              \
        static constexpr const char* label  = LABEL;                 \
        static constexpr double      factor = FACTOR;                \
      }
    
    _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ns,  "ns",  1);
    _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::us,  "us",  1000);
    _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ms,  "ms",  1000000);
    _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::s,   "s",   1000000000);
    _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::min, "min", 60000000000);
    _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::h,   "h",   3600000000000);
#   undef _backend_CHZ_MAKE_UNIT_HELPER_SPECIALIZATION

    template<Unit U, unsigned D>
    const char* _time_as_cstring(Time<U, D> time)
    {
      static_assert(D <= 3, "_backend::_time_as_string: too many decimals requested");

      static _backend_CHZ_THREADLOCAL char buffer[32];

      double ajusted_time = static_cast<double>(time.nanoseconds.count())/_unit_helper<U>::factor;

      std::sprintf(buffer,
        D == 0 ? "%.0f %s"
      : D == 1 ? "%.1f %s"
      : D == 2 ? "%.2f %s"
      :          "%.3f %s",
      ajusted_time, _unit_helper<U>::label);

      return buffer;
    }

    template<unsigned D>
    const char* _time_as_cstring(Time<Unit::automatic, D> time)
    {
      // 10 h < duration
      if (time.nanoseconds.count() > 36000000000000) _backend_CHZ_COLD
      {
        return _time_as_cstring(Time<Unit::h, D>{time.nanoseconds});
      }

      // 10 min < duration <= 10 h
      if (time.nanoseconds.count() > 600000000000) _backend_CHZ_COLD
      {
        return _time_as_cstring(Time<Unit::min, D>{time.nanoseconds});
      }

      // 10 s < duration <= 10 m
      if (time.nanoseconds.count() > 10000000000)
      {
        return _time_as_cstring(Time<Unit::s, D>{time.nanoseconds});
      }

      // 10 ms < duration <= 10 s
      if (time.nanoseconds.count() > 10000000)
      {
        return _time_as_cstring(Time<Unit::ms, D>{time.nanoseconds});
      }

      // 10 us < duration <= 10 ms
      if (time.nanoseconds.count() > 10000)
      {
        return _time_as_cstring(Time<Unit::us, D>{time.nanoseconds});
      }

      // duration <= 10 us
      return _time_as_cstring(Time<Unit::ns, D>{time.nanoseconds});
    }

    template<Unit U, unsigned D>
    std::string _format_time(Time<U, D> time, std::string&& format) noexcept
    {
      static const std::string unit_specifiers[] = {"%ns", "%us", "%ms", "%s", "%min", "%h"};

      for (unsigned k = 0; k < 6; ++k)
      {
        const auto& unit_specifier = unit_specifiers[k];
        auto  position             = format.rfind(unit_specifier);
        while (position != std::string::npos)
        {
          format.replace(position, unit_specifier.length(), _time_as_cstring(time));
          position = format.find(unit_specifier);
        }
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

      if (iterations == 0) _backend_CHZ_COLD
      {
        iterations = 1;
      }

      return _format_time(Time<U, 3>{time.nanoseconds/iterations}, std::move(format));
    }
    
    template<Unit U, unsigned D>
    std::ostream& operator<<(std::ostream& ostream, Time<U, D> time) noexcept
    {
      return ostream << "elapsed time: " << _backend::_time_as_cstring(time) << std::endl;
    }

    struct _measure_backdoor;
  }
//----------------------------------------------------------------------------------------------------------------------
# undef  CHRONOMETRO_MEASURE
# define CHRONOMETRO_MEASURE(...)             _backend_CHZ_MEASURE_PROX(__LINE__, __VA_ARGS__)
# define _backend_CHZ_MEASURE_PROX(line, ...) _backend_CHZ_MEASURE_IMPL(line,     __VA_ARGS__)
# define _backend_CHZ_MEASURE_IMPL(line, ...)                             \
    for (Chronometro::Measure _measurement##line{__VA_ARGS__};            \
      Chronometro::_backend::_measure_backdoor::good(_measurement##line); \
      Chronometro::_backend::_measure_backdoor::next(_measurement##line))

  class Stopwatch
  {
    class _guard;
  public:
    _backend_CHZ_NODISCARD_REASON("lap: not using the return value makes no sens")
    inline // display and return lap time
    auto lap() noexcept -> _backend::Time<Unit::automatic, 0>;

    _backend_CHZ_NODISCARD_REASON("split: not using the return value makes no sens")
    inline // display and return split time
    auto split() noexcept -> _backend::Time<Unit::automatic, 0>;

    inline // reset measured times
    void reset() noexcept;

    inline // pause time measurement
    void pause() noexcept;

    inline // unpause time measurement
    void unpause() noexcept;

    inline // RAII-style scoped pause/unpause
    auto guard() noexcept -> _guard;
    
  private:
    bool                         _is_paused    = false;
    std::chrono::nanoseconds     _duration_tot = {};
    std::chrono::nanoseconds     _duration_lap = {};
    _backend::_clock::time_point _previous     = _backend::_clock::now();
  public: // extra overloads to make time formatting easier
    template<Unit U, unsigned D = 0>
    _backend_CHZ_NODISCARD_REASON("lap: not using the return value makes no sens")
    inline // display and return lap time with custom format
    auto lap() noexcept -> _backend::Time<U, D>;

    template<unsigned D, Unit U = Unit::automatic>
    _backend_CHZ_NODISCARD_REASON("lap: not using the return value makes no sens")
    inline // display and return lap time with custom format
    auto lap() noexcept -> _backend::Time<U, D>;

    template<Unit U, unsigned D = 0>
    _backend_CHZ_NODISCARD_REASON("split: not using the return value makes no sens")
    inline // display and return split time with custom format
    auto split() noexcept -> _backend::Time<U, D>;

    template<unsigned D, Unit U = Unit::automatic>
    _backend_CHZ_NODISCARD_REASON("split: not using the return value makes no sens")
    inline // display and return split time
    auto split() noexcept -> _backend::Time<U, D>;
  };

  class Measure
  {
  public:
    // measure one iteration
    Measure() noexcept = default;

    inline // measure iterations
    Measure(unsigned iterations) noexcept;

    inline // measure iterations with custom iteration message
    Measure(unsigned iterations, const char* iteration_format) noexcept;

    inline // measure iterations with custom iteration/total message
    Measure(unsigned iterations, const char* iteration_format, const char* total_format) noexcept;

    inline // pause measurement
    void pause() noexcept;

    inline // unpause measurement
    void unpause() noexcept;

    inline // scoped pause/unpause of measurement
    auto guard() noexcept -> decltype(Stopwatch().guard());

  private:
    const unsigned _iterations  = 1;
    unsigned       _iters_left  = _iterations;
    const char*    _iter_format = nullptr;
    const char*    _tot_format  = "total elapsed time: %ms";
    Stopwatch      _stopwatch;
    class _iter;
  public:
    inline _iter begin()     noexcept;
    inline _iter end() const noexcept;
  private:
    class View;
    inline View  view()      noexcept;
    inline void  next()      noexcept;
    inline bool  good()      noexcept;
  friend _backend::_measure_backdoor;
  };

  class Measure::View final
  {
    friend Measure;
  public:
    // current measurement iteration
    const unsigned iteration;

    inline // pause measurements
    void pause() noexcept;

    inline // unpause measurement
    void unpause() noexcept;

    inline // scoped pause/unpause of measurement
    auto guard() noexcept -> decltype(Stopwatch().guard());
  private:
    inline View(unsigned current_iteration, Measure* measurement) noexcept;
    Measure* const _measurement;
  };

# undef  CHRONOMETRO_ONLY_EVERY_MS
# define CHRONOMETRO_ONLY_EVERY_MS(N)                                                               \
    if ([]{                                                                                         \
      static_assert((N) > 0, "CHRONOMETRO_ONLY_EVERY_MS: 'N' must be a non-zero positive number."); \
      static Chronometro::_backend::_clock::time_point _previous = {};                              \
      auto _target = std::chrono::nanoseconds{(N)*1000000};                                         \
      if ((Chronometro::_backend::_clock::now() - _previous) > _target)                             \
      {                                                                                             \
        _previous = Chronometro::_backend::_clock::now();                                           \
        return true;                                                                                \
      }                                                                                             \
      return false;                                                                                 \
    }())
//----------------------------------------------------------------------------------------------------------------------
  namespace _backend
  {
    struct _measure_backdoor
    {
      static
      bool good(Measure& measure_) noexcept
      {
        return measure_.good();
      }

      static
      void next(Measure& measure_) noexcept
      {
        measure_.next();
      }
    };
  }
//----------------------------------------------------------------------------------------------------------------------
  class Stopwatch::_guard final
  {
    friend Stopwatch;
  private:
    Stopwatch* const _stopwatch;

    _guard(Stopwatch* stopwatch) noexcept :
      _stopwatch(stopwatch)
    {
      _stopwatch->pause();
    }

  public:
    ~_guard() noexcept
    {
      _stopwatch->unpause();
    }
  };
//----------------------------------------------------------------------------------------------------------------------
  class Measure::_iter final
  {
  public:
    _iter() noexcept = default;

    _iter(Measure* const measure_) :
      _measure(measure_)
    {}

    void operator++() const noexcept
    {
      _measure->next();
    }

    bool operator!=(_iter&) const noexcept
    {
      return _measure->good();
    }

    auto operator*() const noexcept -> Measure::View
    {
      return _measure->view();
    }
  private:
    Measure* const _measure = nullptr;
  };
//----------------------------------------------------------------------------------------------------------------------
  template<Unit U, unsigned D>
  template<Unit U_, unsigned D_>
  auto _backend::Time<U, D>::format() noexcept -> _backend::Time<U_, D_>
  {
    return reinterpret_cast<Time<U_, D_>&>(*this);
  }

  template<Unit U, unsigned D>
  template<unsigned D_, Unit U_>
  auto _backend::Time<U, D>::format() noexcept -> _backend::Time<U_, D_>
  {
    return reinterpret_cast<Time<U_, D_>&>(*this);
  }
//----------------------------------------------------------------------------------------------------------------------
  auto Stopwatch::lap() noexcept -> _backend::Time<Unit::automatic, 0>
  {
    return lap<Unit::automatic, 0>();
  }
  
  template<Unit U, unsigned D>
  auto Stopwatch::lap() noexcept -> _backend::Time<U, D>
  {
    auto now = _backend::_clock::now();

    std::chrono::nanoseconds lap_duration = _duration_lap;
    _duration_lap = {};

    if (not _is_paused) _backend_CHZ_HOT
    {
      _duration_tot += now - _previous;
      lap_duration  += now - _previous;

      _previous = _backend::_clock::now(); // start measurement from here
    }

    return _backend::Time<U, D>{lap_duration};
  }

  template<unsigned D, Unit U>
  auto Stopwatch::lap() noexcept -> _backend::Time<U, D>
  {
    return lap<U, D>();
  }

  auto Stopwatch::split() noexcept -> _backend::Time<Unit::automatic, 0>
  {
    return split<Unit::automatic, 0>();
  }

  template<Unit U, unsigned D>
  auto Stopwatch::split() noexcept -> _backend::Time<U, D>
  {
    auto now = _backend::_clock::now();

    std::chrono::nanoseconds tot_duration = _duration_tot;

    if (not _is_paused) _backend_CHZ_HOT
    {
      tot_duration += now - _previous;

      _duration_lap = {};
      _duration_tot = {};
    }

    return _backend::Time<U, D>{tot_duration};
  }

  template<unsigned D, Unit U>
  auto Stopwatch::split() noexcept -> _backend::Time<U, D>
  {
    return split<U, D>();
  }

  void Stopwatch::reset() noexcept
  {
    _duration_tot = {};
    _duration_lap = {};

    // hot reset if unpaused
    if (not _is_paused)
    {
      _previous = _backend::_clock::now(); // start measurement from here
    }
  }

  void Stopwatch::pause() noexcept
  {
    auto now = _backend::_clock::now();

    if (not _is_paused) _backend_CHZ_HOT
    {
      _is_paused = true;

      _duration_tot += now - _previous;
      _duration_lap += now - _previous;
    }
  }

  void Stopwatch::unpause() noexcept
  {
    if (_is_paused) _backend_CHZ_HOT
    {
      _is_paused = false;

      _previous  = _backend::_clock::now(); // start measurement from here
    }
  }

  auto Stopwatch::guard() noexcept -> Stopwatch::_guard
  {
    return _guard(this);
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

  Measure::View Measure::view() noexcept
  {
    return View(_iterations - _iters_left, this);
  }

  void Measure::next() noexcept
  {
    _stopwatch.pause();
    auto iter_duration = _stopwatch.lap();

    if (_iter_format)
    {
      _backend_CHZ_DECLARE_LOCK(_backend::_out_mtx);
      _io::out << _backend::_format_lap(iter_duration, _iter_format, _iterations - _iters_left) << std::endl;
    }

    --_iters_left;
    _stopwatch.unpause();
  }
  
  bool Measure::good() noexcept
  {
    _stopwatch.pause();
    if (_iters_left) _backend_CHZ_HOT
    {
      _stopwatch.unpause();
      return true;
    }

    auto duration = _stopwatch.split();

    if (_tot_format) _backend_CHZ_HOT
    {
      _backend_CHZ_DECLARE_LOCK(_backend::_out_mtx);
      _io::out << _backend::_format_tot(duration, _tot_format, _iterations) << std::endl;
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

  auto Measure::guard() noexcept -> decltype(Stopwatch().guard())
  {
    return _stopwatch.guard();
  }

  auto Measure::begin() noexcept -> _iter
  {
    _iters_left = _iterations;

    _stopwatch.unpause();
    _stopwatch.reset();

    return _iter(this);
  }

  auto Measure::end() const noexcept -> _iter
  {
    return _iter();
  }

  auto Measure::View::guard() noexcept -> decltype(Stopwatch().guard())
  {
    return _measurement->guard();
  }
}
//----------------------------------------------------------------------------------------------------------------------
# undef _backend_CHZ_PRAGMA
# undef _backend_CHZ_IGNORE
# undef _backend_CHZ_HOT
# undef _backend_CHZ_COLD
# undef _backend_CHZ_NODISCARD
# undef _backend_CHZ_NODISCARD_REASON
# undef _backend_CHZ_THREADLOCAL
# undef _backend_CHZ_DECLARE_MUTEX
# undef _backend_CHZ_DECLARE_LOCK
#endif
