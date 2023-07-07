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
    // bring clocks and nanoseconds to frontend
    using std::chrono::system_clock;
    using std::chrono::steady_clock;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;

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
    template<Unit U = Unit::automatic, typename C = high_resolution_clock>
    class Stopwatch;

    // measure function execution time
    template<Unit U = Unit::automatic, typename C = high_resolution_clock, typename F, typename... A>
    typename C::duration execution_time(const F function, const size_t repetitions, const A... arguments);

    // measure function execution time without function calling via pointers
    #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)

    // output ostream
    std::ostream out_ostream{std::cout.rdbuf()};
    // error ostream
    std::ostream err_ostream{std::cerr.rdbuf()};
    // warning ostream
    std::ostream wrn_ostream{std::cerr.rdbuf()};
  }
// --Chronometro library : frontend struct and class definitions-----------------------------------
  inline namespace Frontend
  {
    template<Unit U, typename C>
    class Stopwatch final {
      public:
        // start measuring time
        inline explicit Stopwatch() noexcept;
        // display and return lap time
        typename C::duration lap(void) noexcept;
        // display and return split time
        typename C::duration split(void) noexcept;
        // pause time measurement
        void pause(void) noexcept;
        // reset measured times
        void reset(void) noexcept;
        // unpause time measurement
        void unpause(void) noexcept;
      public:
        // used clock
        using clock = C;
        // unit to be used when displaying elapsed time
        const Unit unit = U;
      private:
        // display elapsed time
        void display(const nanoseconds::rep nanoseconds, const std::string& asset) const noexcept;
        // issue warning
        inline void warning(const char*) const noexcept;
        // issue error
        inline void error(const char*) const noexcept;
        // used to keep track of the current status
        bool is_paused_;
        // measured elapsed time
        typename C::duration duration_;
        // measured elapsed time
        typename C::duration duration_lap_;
        // time either at construction or from last unpause
        typename C::time_point previous_;
        // time either at construction or from last unpause/lap
        typename C::time_point previous_lap_;
    };
  }
// --Chronometro library : backend forward declaration---------------------------------------------
  namespace Backend
  {
    // returns the appropriate unit to display time
    Unit appropriate_unit(const nanoseconds::rep nanoseconds);
  }
// --Chronometro library : frontend definitions----------------------------------------------------
  inline namespace Frontend
  {
    template<Unit U, typename C>
    Stopwatch<U, C>::Stopwatch() noexcept
      : // member initialization list
      //unit((unit > Unit::automatic) ? warning("invalid unit, automatic used instead"), Unit::automatic : unit),
      is_paused_(false),
      duration_(0),
      duration_lap_(0),
      previous_(C::now()),
      previous_lap_(previous_)
    {}

    template<Unit U, typename C>
    typename C::duration Stopwatch<U, C>::lap(void) noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      if (is_paused_ == false) {
        // save elapsed times
        duration_     += now - previous_;
        duration_lap_ += now - previous_lap_;

        // display lap time
        display(nanoseconds(duration_lap_).count(), "lap time: ");
        
        // reset measured time
        duration_lap_ = typename C::duration(0);
        previous_     = C::now();
        previous_lap_ = previous_;
      }
      else warning("cannot measure lap, is paused");

      return duration_lap_;
    }

    template<Unit U, typename C>
    typename C::duration Stopwatch<U, C>::split(void) noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      if (is_paused_ == false) {
        // save elapsed times
        duration_     += now - previous_;
        duration_lap_ += now - previous_lap_;

        // display split time
        display(nanoseconds(duration_).count(), "elapsed time: ");
        
        // save time point
        previous_     = C::now();
        previous_lap_ = previous_;
      }
      else warning("cannot measure split, is paused");
      
      return duration_;
    }

    template<Unit U, typename C>
    void Stopwatch<U, C>::pause(void) noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      // add elapsed time up to now if not paused
      if (is_paused_ == false) {
        // pause
        is_paused_ = true;

        // save elapsed times
        duration_     += now - previous_;
        duration_lap_ += now - previous_lap_;
      }
      else warning("cannot pause further, is already paused");
    }

    template<Unit U, typename C>
    void Stopwatch<U, C>::reset(void) noexcept
    {
      if (is_paused_ == true) {
        // reset measured time
        duration_     = typename C::duration(0);
        duration_lap_ = typename C::duration(0);
      }
      else warning("cannot reset, must be paused");
    }

    template<Unit U, typename C>
    void Stopwatch<U, C>::unpause(void) noexcept
    {
      if (is_paused_ == true) {
        // unpause
        is_paused_ = false;

        // reset measured time
        previous_     = C::now();
        previous_lap_ = previous_;
      }
      else warning("is already unpaused");
    }
    
    template<Unit U, typename C>
    void Stopwatch<U, C>::display(const nanoseconds::rep nanoseconds, const std::string& asset) const noexcept
    {
      // if unit_ == automatic, deduce the appropriate unit
      switch((unit == Unit::automatic) ? Backend::appropriate_unit(nanoseconds) : unit) {
        case Unit::ns:
          out_ostream << asset << nanoseconds << " ns" << std::endl;
          break;
        case Unit::us:
          out_ostream << asset << nanoseconds / 1000 << " us" << std::endl;
          break;
        case Unit::ms:
          out_ostream << asset << nanoseconds / 1000000 << " ms" << std::endl;
          break;
        case Unit::s:
          out_ostream << asset << nanoseconds / 1000000000 << " s" << std::endl;
          break;
        case Unit::min:
          out_ostream << asset << nanoseconds / 60000000000 << " min" << std::endl;
          break;
        case Unit::h:
          out_ostream << asset << nanoseconds / 3600000000000 << " h" << std::endl;
          break;
        default: error("invalid time unit");
      }
    }

    template<Unit U, typename C>
    void Stopwatch<U, C>::warning(const char* message) const noexcept
    {
      wrn_ostream << "warning: Stopwatch: " << message << std::endl;
    }

    template<Unit U, typename C>
    void Stopwatch<U, C>::error(const char* message) const noexcept
    {
      err_ostream << "error: Stopwatch: " << message << std::endl;
    }

    template<Unit U, typename C, typename F, typename... A>
    typename C::duration execution_time(const F function, const size_t repetitions, const A... arguments)
    {
      Stopwatch<U, C> stopwatch;

      for (size_t iteration = 0; iteration < repetitions; ++iteration)
        function(arguments...);

      return stopwatch.split();
    }

    #undef  CHRONOMETRO_EXECUTION_TIME
    #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)           \
      [&](void) {                                                            \
        Chronometro::Stopwatch<> stopw_atch;                                 \
        const size_t repet_itions = repetitions;                             \
        for (size_t itera_tion = 0; itera_tion < repet_itions; ++itera_tion) \
          function(__VA_ARGS__);                                             \
        return stopw_atch.split();                                           \
      }()
  }
// --Chronometro library : backend definitions-----------------------------------------------------
  namespace Backend
  {
    Unit appropriate_unit(const nanoseconds::rep nanoseconds)
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
