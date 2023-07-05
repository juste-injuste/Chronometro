/*---author----------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Chronometro

-----licence---------------------------------------------------------------------------------------
 
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
 
-----versions--------------------------------------------------------------------------------------

Version 0.1.0 - Initial release

-----description-----------------------------------------------------------------------------------

Chronometro is a simple and lightweight C++11 (and newer) library that allows you to measure the
execution time of functions or code blocks. See the included README.MD file for more information.

-----inclusion guard-----------------------------------------------------------------------------*/
#ifndef CHRONOMETRO_HPP
#define CHRONOMETRO_HPP
// --necessary standard libraries------------------------------------------------------------------
#include <chrono>   // for clocks and time representations
#include <iostream> // for std::cout, std::cerr, std::endl
#include <cstddef>  // for size_t
#include <ostream>  // for std::ostream
#include <string>   // for std::string
// --Chronometro library---------------------------------------------------------------------------
namespace Chronometro
{
  namespace Version
  {
    constexpr long NUMBER = 000001000;
    constexpr long MAJOR  = 000      ;
    constexpr long MINOR  =    001   ;
    constexpr long PATCH  =       000;
  }
// --Chronometro library : frontend forward declarations-------------------------------------------
  inline namespace Frontend
  {
    // bring clocks to frontend
    using std::chrono::system_clock;
    using std::chrono::steady_clock;
    using std::chrono::high_resolution_clock;

    // time units for displaying
    enum class Unit : uint8_t {
      ns,       // nanoseconds
      us,       // microseconds
      ms,       // milliseconds
      s,        // seconds
      min,      // minutes
      h,        // hours
      automatic // deduce automatically the appropriate unit
    };

    // measure elapsed time
    template<typename C = high_resolution_clock>
    class Stopwatch;

    // measure function execution time
    template<typename C = high_resolution_clock, typename F, typename... A>
    typename C::duration execution_time(const F function, const size_t repetitions, const A... arguments);

    // measure function execution time without function calling via pointers
    #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)
  }
// --Chronometro library : frontend struct and class definitions-----------------------------------
  inline namespace Frontend
  {
    template<typename C>
    class Stopwatch final {
      public:
        inline explicit Stopwatch(const Unit unit = Unit::automatic) noexcept;
        // start measuring time
        void start(void) noexcept;
        // pause time measurement
        typename C::duration pause(void) noexcept;
        // stop time measurement and display elapsed time
        typename C::duration stop(void) noexcept;
        // reset measured time and start measuring time
        inline void restart(void) noexcept;
        // set unit
        inline void set(const Unit unit) noexcept;
      private:
        // unit to be used when displaying elapsed time
        Unit unit_;
        // used to keep track stopwatch status
        bool paused_;
        // time either at construction or from last start/restart 
        typename C::time_point start_;
        // measured elapsed time
        typename C::duration duration_;
      public:
        // Stopwatch internals
        struct Internals final {
          // elapsed time prefix
          static std::string label;
          // warning message prefix
          static std::string wrn_label;
          // error message prefix
          static std::string err_label;
          // output ostream
          static std::ostream out_ostream;
          // error ostream
          static std::ostream err_ostream;
          // warning ostream
          static std::ostream wrn_ostream;
          // display or not elapsed time
          static bool display;
          // not meant to be instantiated
          Internals(void) = delete;
        };
    };
  }
// --Chronometro library : backend forward declaration---------------------------------------------
  namespace Backend
  {
    // returns the appropriate unit to display time
    Unit appropriate_unit(const std::chrono::nanoseconds::rep nanoseconds);
  }
// --Chronometro library : frontend definitions----------------------------------------------------
  inline namespace Frontend
  {
    template<typename C>
    Stopwatch<C>::Stopwatch(const Unit unit) noexcept
      : // member initialization list
      unit_(unit),
      paused_(false),
      start_(C::now()),
      duration_(0)
    {}

    template<typename C>
    void Stopwatch<C>::start(void) noexcept
    {
      if (paused_) {
        // unpause
        paused_ = false;

        // measure current time
        start_ = C::now();
      }
      else Internals::wrn_ostream << Internals::wrn_label << "already started" << std::endl;
    }

    template<typename C>
    typename C::duration Stopwatch<C>::pause(void) noexcept
    {
      // measure elapsed time
      const typename C::duration duration = C::now() - start_;

      // add elapsed time up to now if not paused
      if (paused_) {
        Internals::wrn_ostream << Internals::wrn_label << "already paused" << std::endl;
      }
      else {
        duration_ += duration;
        paused_ = true;
      }

      return duration_;
    }

    template<typename C>
    typename C::duration Stopwatch<C>::stop(void) noexcept
    {
      // pause time measurement
      pause();

      // measured time in nanoseconds
      const std::chrono::nanoseconds::rep nanoseconds = std::chrono::nanoseconds(duration_).count();

      // conditionally display elapsed time
      if (Internals::display) {
        // if unit_ == automatic, deduce the appropriate unit
        switch((unit_ == Unit::automatic) ? Backend::appropriate_unit(nanoseconds) : unit_) {
          case Unit::ns:
            Internals::out_ostream << Internals::label << nanoseconds << " ns" << std::endl;
            break;
          case Unit::us:
            Internals::out_ostream << Internals::label << nanoseconds / 1000 << " us" << std::endl;
            break;
          case Unit::ms:
            Internals::out_ostream << Internals::label << nanoseconds / 1000000 << " ms" << std::endl;
            break;
          case Unit::s:
            Internals::out_ostream << Internals::label << nanoseconds / 1000000000 << " s" << std::endl;
            break;
          case Unit::min:
            Internals::out_ostream << Internals::label << nanoseconds / 60000000000 << " min" << std::endl;
            break;
          case Unit::h:
            Internals::out_ostream << Internals::label << nanoseconds / 3600000000000 << " h" << std::endl;
            break;
          default: Internals::err_ostream << Internals::err_label << "invalid time unit" << std::endl;
        }
      }

      return duration_;
    }

    template<typename C>
    void Stopwatch<C>::restart(void) noexcept
    {
      // reset measured duration
      duration_ = typename C::duration(0);

      // unpause
      paused_ = false;

      // measure current time
      start_ = C::now();
    }

    template<typename C>
    void Stopwatch<C>:: set(const Unit unit) noexcept
    {
      // validate unit
      if (unit > Unit::automatic) {
        Internals::wrn_ostream << Internals::wrn_label << "invalid unit, automatic used instead" << std::endl;
        unit_ = Unit::automatic;
      }
      else unit_ = unit;
    }

    template<typename C>
    std::ostream Stopwatch<C>::Internals::out_ostream{std::cout.rdbuf()};

    template<typename C>
    std::ostream Stopwatch<C>::Internals::err_ostream(std::cerr.rdbuf());

    template<typename C>
    std::ostream Stopwatch<C>::Internals::wrn_ostream(std::cerr.rdbuf());

    template<typename C>
    std::string Stopwatch<C>::Internals::label = "elapsed time: ";

    template<typename C>
    std::string Stopwatch<C>::Internals::wrn_label = "warning: Stopwatch: ";

    template<typename C>
    std::string Stopwatch<C>::Internals::err_label = "error: Stopwatch: ";

    template<typename C>
    bool Stopwatch<C>::Internals::display = true;

    template<typename C, typename F, typename... A>
    typename C::duration execution_time(const F function, const size_t repetitions, const A... arguments)
    {
      Stopwatch<C> stopwatch(Unit::automatic);

      for (size_t iteration = 0; iteration < repetitions; ++iteration)
        function(arguments...);

      return stopwatch.stop();
    }

    #undef  CHRONOMETRO_EXECUTION_TIME
    #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)                     \
      [&](void) {                                                                      \
        Chronometro::Stopwatch<> _stopwatch_;                                          \
        for (size_t _iteration_ = 0; _iteration_ < size_t(repetitions); ++_iteration_) \
          function(__VA_ARGS__);                                                       \
        return _stopwatch_.stop();                                                     \
      }()
  }
// --Chronometro library : backend definitions-----------------------------------------------------
  namespace Backend
  {
    Unit appropriate_unit(const std::chrono::nanoseconds::rep nanoseconds)
    {
      // 10 h < duration
      if (nanoseconds > 36000000000000)
        return Unit::h;

      // 10 min < duration <= 10 h
      if (nanoseconds > 600000000000)
        return Unit::min;

      // 10 s < duration <= 10 m
      if (nanoseconds > 10000000000)
        return Unit::s;

      // 10 ms < duration <= 10 s
      if (nanoseconds > 10000000)
        return Unit::ms;

      // 10 us < duration <= 10 ms
      if (nanoseconds > 10000)
        return Unit::us;

      // duration <= 10 us
      return Unit::ns;
    }
  }
}
#endif
