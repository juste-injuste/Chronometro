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
#ifndef _chronometro_hpp
#define _chronometro_hpp
//---necessary standard libraries---------------------------------------------------------------------------------------
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
# define _chz_impl_THREADSAFE
# include <mutex> // for std::mutex, std::lock_guard
#endif
//---Chronometro library------------------------------------------------------------------------------------------------
namespace Chronometro
{
  // measures the time it takes to execute the following statement/block N times, with labels
# define CHRONOMETRO_MEASURE(...)

  // measure elapsed time
  class Stopwatch;

  // measure iterations via range-based for-loop
  class Measure;

  // units in which time obtained from Stopwatch can be displayed
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

  // execute following statement/blocks only if its last execution was atleast N milliseconds prior
# define CHRONOMETRO_ONLY_EVERY_MS(N)

  namespace _io
  {
    static std::ostream out(std::cout.rdbuf()); // output ostream
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
#   define _chz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _chz_impl_IGNORE(WARNING, ...)                \
      _chz_impl_PRAGMA(clang diagnostic push)            \
      _chz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                   \
      _chz_impl_PRAGMA(clang diagnostic pop)
#endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _chz_impl_HOT  _chz_impl_IGNORE("-Wc++20-extensions", [[likely]])
#   define _chz_impl_COLD _chz_impl_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _chz_impl_HOT  [[likely]]
#   define _chz_impl_COLD [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _chz_impl_HOT  [[likely]]
#   define _chz_impl_COLD [[unlikely]]
# else
#   define _chz_impl_HOT
#   define _chz_impl_COLD
# endif

// support from clang 3.9.0 and GCC 5.1 onward
# if defined(__clang__)
#   define _chz_impl_NODISCARD __attribute__((warn_unused_result))
# elif defined(__GNUC__)
#   define _chz_impl_NODISCARD __attribute__((warn_unused_result))
# else
#   define _chz_impl_NODISCARD
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _chz_impl_NODISCARD_REASON(REASON) _chz_impl_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _chz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _chz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _chz_impl_NODISCARD_REASON(REASON) _chz_impl_NODISCARD
# endif

#if defined(_chz_impl_THREADSAFE)
# undef  _chz_impl_THREADSAFE
# define _chz_impl_THREADLOCAL         thread_local
# define _chz_impl_DECLARE_MUTEX(...)  static std::mutex __VA_ARGS__
# define _chz_impl_DECLARE_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock(MUTEX)
#else
# define _chz_impl_THREADLOCAL
# define _chz_impl_DECLARE_MUTEX(...)
# define _chz_impl_DECLARE_LOCK(MUTEX) void(0)
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

    template<Unit unit, unsigned n_decimals>
    class _time
    {
    public:
      std::chrono::nanoseconds nanoseconds;

      template<Unit unit_, unsigned n_decimals_ = n_decimals>
      auto format() noexcept -> _time<unit_, n_decimals_>
      {
        return reinterpret_cast<_time<unit_, n_decimals_>&>(*this);
      }

      template<unsigned n_decimals_, Unit unit_ = unit>
      auto format() noexcept -> _time<unit_, n_decimals_>
      {
        return reinterpret_cast<_time<unit_, n_decimals_>&>(*this);
      }
    };

    _chz_impl_DECLARE_MUTEX(_out_mtx);

    template<Unit>
    struct _unit_helper;

#   define _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(UNIT, LABEL, FACTOR) \
      template<>                                                     \
      struct _unit_helper<UNIT>                                      \
      {                                                              \
        static constexpr const char* label  = LABEL;                 \
        static constexpr double      factor = FACTOR;                \
      }
    
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ns,  "ns",  1);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::us,  "us",  1000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::ms,  "ms",  1000000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::s,   "s",   1000000000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::min, "min", 60000000000);
    _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION(Unit::h,   "h",   3600000000000);

#   undef _chz_impl_MAKE_UNIT_HELPER_SPECIALIZATION

    template<Unit unit, unsigned n_decimals>
    const char* _time_as_cstring(const _time<unit, n_decimals> time_) noexcept
    {
      static_assert(n_decimals <= 3, "_backend::_time_as_string: too many decimals requested");

      static _chz_impl_THREADLOCAL char buffer[32];

      const auto ajusted_time = static_cast<double>(time_.nanoseconds.count())/_unit_helper<unit>::factor;

      std::sprintf(buffer,
        n_decimals == 0 ? "%.0f %s"
      : n_decimals == 1 ? "%.1f %s"
      : n_decimals == 2 ? "%.2f %s"
      :                   "%.3f %s",
      ajusted_time, _unit_helper<unit>::label);

      return buffer;
    }

    template<unsigned n_decimals>
    const char* _time_as_cstring(const _time<Unit::automatic, n_decimals> time_) noexcept
    {
      // 10 h < duration
      if (time_.nanoseconds.count() > 36000000000000) _chz_impl_COLD
      {
        return _time_as_cstring(_time<Unit::h, n_decimals>{time_.nanoseconds});
      }

      // 10 min < duration <= 10 h
      if (time_.nanoseconds.count() > 600000000000) _chz_impl_COLD
      {
        return _time_as_cstring(_time<Unit::min, n_decimals>{time_.nanoseconds});
      }

      // 10 s < duration <= 10 m
      if (time_.nanoseconds.count() > 10000000000)
      {
        return _time_as_cstring(_time<Unit::s, n_decimals>{time_.nanoseconds});
      }

      // 10 ms < duration <= 10 s
      if (time_.nanoseconds.count() > 10000000)
      {
        return _time_as_cstring(_time<Unit::ms, n_decimals>{time_.nanoseconds});
      }

      // 10 us < duration <= 10 ms
      if (time_.nanoseconds.count() > 10000)
      {
        return _time_as_cstring(_time<Unit::us, n_decimals>{time_.nanoseconds});
      }

      // duration <= 10 us
      return _time_as_cstring(_time<Unit::ns, n_decimals>{time_.nanoseconds});
    }

    template<Unit unit, unsigned n_decimals>
    std::string _format_time(const _time<unit, n_decimals> time_, std::string&& fmt_) noexcept
    {
      static const std::string unit_specifiers[] = {"%ns", "%us", "%ms", "%s", "%min", "%h"};

      for (const auto& unit_specifier : unit_specifiers)
      {
        auto position = fmt_.rfind(unit_specifier);
        while (position != std::string::npos)
        {
          fmt_.replace(position, unit_specifier.length(), _time_as_cstring(time_));
          position = fmt_.find(unit_specifier);
        }
      }

      return std::move(fmt_);
    }

    template<Unit unit, unsigned n_decimals>
    std::string _format_lap(const _time<unit, n_decimals> time_, std::string&& fmt_, const unsigned iter_) noexcept
    {
      auto position = fmt_.find("%#");
      while (position != std::string::npos)
      {
        fmt_.replace(position, 2, std::to_string(iter_));
        position = fmt_.rfind("%#");
      }

      return _format_time(time_, std::move(fmt_));
    }

    template<Unit unit, unsigned n_decimals>
    std::string _format_tot(const _time<unit, n_decimals> time_, std::string&& fmt_, unsigned n_iters_) noexcept
    {
      fmt_ = _format_time(time_, std::move(fmt_));

      auto position = fmt_.rfind("%D");
      while (position != std::string::npos)
      {
        fmt_.erase(position + 1, 1);
        position = fmt_.find("%D");
      }

      if (n_iters_ == 0) _chz_impl_COLD
      {
        n_iters_ = 1;
      }

      return _format_time(_time<unit, 3>{time_.nanoseconds/n_iters_}, std::move(fmt_));
    }
    
    template<Unit unit, unsigned n_decimals>
    std::ostream& operator<<(std::ostream& ostream_, const _time<unit, n_decimals> time_) noexcept
    {
      return ostream_ << "elapsed time: " << _backend::_time_as_cstring(time_) << std::endl;
    }

    struct _measure_backdoor;
  }
//----------------------------------------------------------------------------------------------------------------------
# undef  CHRONOMETRO_MEASURE
# define CHRONOMETRO_MEASURE(...)                 _chz_impl_MEASURE_PROX(__LINE__,    __VA_ARGS__)
# define _chz_impl_MEASURE_PROX(line_number, ...) _chz_impl_MEASURE_IMPL(line_number, __VA_ARGS__)
# define _chz_impl_MEASURE_IMPL(line_number, ...)                                \
    for (Chronometro::Measure _measurement##line_number{__VA_ARGS__};            \
      Chronometro::_backend::_measure_backdoor::good(_measurement##line_number); \
      Chronometro::_backend::_measure_backdoor::next(_measurement##line_number))

  class Stopwatch
  {
    class _guard;
  public:    
    _chz_impl_NODISCARD_REASON("lap: not using the return value makes no sens")
    inline // display and return lap time
    auto lap() noexcept -> _backend::_time<Unit::automatic, 0>;

    _chz_impl_NODISCARD_REASON("split: not using the return value makes no sens")
    inline // display and return split time
    auto split() noexcept -> _backend::_time<Unit::automatic, 0>;

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
    template<Unit unit, unsigned n_decimals = 0>
    _chz_impl_NODISCARD_REASON("lap: not using the return value makes no sens.")
    inline // display and return lap time with custom format
    auto lap() noexcept -> _backend::_time<unit, n_decimals>;

    template<unsigned n_decimals, Unit unit = Unit::automatic>
    _chz_impl_NODISCARD_REASON("lap: not using the return value makes no sens.")
    inline // display and return lap time with custom format
    auto lap() noexcept -> _backend::_time<unit, n_decimals>;

    template<Unit unit, unsigned n_decimals = 0>
    _chz_impl_NODISCARD_REASON("split: not using the return value makes no sens.")
    inline // display and return split time with custom format
    auto split() noexcept -> _backend::_time<unit, n_decimals>;

    template<unsigned n_decimals, Unit unit = Unit::automatic>
    _chz_impl_NODISCARD_REASON("split: not using the return value makes no sens.")
    inline // display and return split time
    auto split() noexcept -> _backend::_time<unit, n_decimals>;
  };

  class Measure
  {
    class View;
  public:
    constexpr // measure one iteration
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
    inline View view() noexcept;
    inline void next() noexcept;
    inline bool good() noexcept;
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
      const auto _target = std::chrono::nanoseconds{(N)*1000000};                                   \
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

    _guard(
      Stopwatch* const stopwatch_
    ) noexcept :
      _stopwatch(stopwatch_)
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
    constexpr _iter() noexcept = default;

    _iter(Measure* const measure_) noexcept :
      _measure(measure_)
    {}

    void operator++() const noexcept
    {
      _measure->next();
    }

    bool operator!=(const _iter&) const noexcept
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
  auto Stopwatch::lap() noexcept -> _backend::_time<Unit::automatic, 0>
  {
    return lap<Unit::automatic, 0>();
  }
  
  template<Unit unit, unsigned n_decimals>
  auto Stopwatch::lap() noexcept -> _backend::_time<unit, n_decimals>
  {
    const auto now = _backend::_clock::now();

    std::chrono::nanoseconds lap_duration = _duration_lap;
    _duration_lap = {};

    if (not _is_paused) _chz_impl_HOT
    {
      _duration_tot += now - _previous;
      lap_duration  += now - _previous;

      _previous = _backend::_clock::now(); // start measurement from here
    }

    return _backend::_time<unit, n_decimals>{lap_duration};
  }

  template<unsigned n_decimals, Unit unit>
  auto Stopwatch::lap() noexcept -> _backend::_time<unit, n_decimals>
  {
    return lap<unit, n_decimals>();
  }

  auto Stopwatch::split() noexcept -> _backend::_time<Unit::automatic, 0>
  {
    return split<Unit::automatic, 0>();
  }

  template<Unit unit, unsigned n_decimals>
  auto Stopwatch::split() noexcept -> _backend::_time<unit, n_decimals>
  {
    const auto now = _backend::_clock::now();

    std::chrono::nanoseconds tot_duration = _duration_tot;

    if (not _is_paused) _chz_impl_HOT
    {
      tot_duration += now - _previous;

      _duration_lap = {};
      _duration_tot = {};
    }

    return _backend::_time<unit, n_decimals>{tot_duration};
  }

  template<unsigned n_decimals, Unit unit>
  auto Stopwatch::split() noexcept -> _backend::_time<unit, n_decimals>
  {
    return split<unit, n_decimals>();
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
    const auto now = _backend::_clock::now();

    if (not _is_paused) _chz_impl_HOT
    {
      _is_paused = true;

      _duration_tot += now - _previous;
      _duration_lap += now - _previous;
    }
  }

  void Stopwatch::unpause() noexcept
  {
    if (_is_paused) _chz_impl_HOT
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
  Measure::Measure(
    const unsigned iterations_
  ) noexcept :
    _iterations(iterations_),
    _tot_format((_iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(
    const unsigned    iterations_,
    const char* const iteration_format_
  ) noexcept :
    _iterations(iterations_),
    _iter_format((iteration_format_[0] == '\0') ? nullptr : iteration_format_),
    _tot_format((_iterations > 1) ? "total elapsed time: %ms [avg = %Dus]" : "total elapsed time: %ms")
  {}

  Measure::Measure(
    const unsigned    iterations_,
    const char* const iteration_format_,
    const char* const total_format_
  ) noexcept :
    _iterations(iterations_),
    _iter_format((iteration_format_[0] == '\0') ? nullptr : iteration_format_),
    _tot_format((total_format_[0] == '\0') ? nullptr : total_format_)
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
    const auto iter_duration = _stopwatch.lap();

    if (_iter_format)
    {
      _chz_impl_DECLARE_LOCK(_backend::_out_mtx);
      _io::out << _backend::_format_lap(iter_duration, _iter_format, _iterations - _iters_left) << std::endl;
    }

    --_iters_left;
    _stopwatch.unpause();
  }
  
  bool Measure::good() noexcept
  {
    _stopwatch.pause();
    if (_iters_left) _chz_impl_HOT
    {
      _stopwatch.unpause();
      return true;
    }

    const auto duration = _stopwatch.split();

    if (_tot_format) _chz_impl_HOT
    {
      _chz_impl_DECLARE_LOCK(_backend::_out_mtx);
      _io::out << _backend::_format_tot(duration, _tot_format, _iterations) << std::endl;
    }

    return false;
  }
//----------------------------------------------------------------------------------------------------------------------
  Measure::View::View(
    const unsigned current_iteration_,
    Measure* const measurement_
  ) noexcept :
    iteration(current_iteration_),
    _measurement(measurement_)
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
# undef _chz_impl_PRAGMA
# undef _chz_impl_IGNORE
# undef _chz_impl_HOT
# undef _chz_impl_COLD
# undef _chz_impl_NODISCARD
# undef _chz_impl_NODISCARD_REASON
# undef _chz_impl_THREADLOCAL
# undef _chz_impl_DECLARE_MUTEX
# undef _chz_impl_DECLARE_LOCK
#endif
