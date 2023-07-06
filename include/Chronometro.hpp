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

    // output ostream
    static std::ostream out_ostream{std::cout.rdbuf()};
    // error ostream
    static std::ostream err_ostream{std::cerr.rdbuf()};
    // warning ostream
    static std::ostream wrn_ostream{std::cerr.rdbuf()};
  }
// --Chronometro library : frontend struct and class definitions-----------------------------------
  inline namespace Frontend
  {
    template<typename C>
    class Stopwatch final {
      public:
        // start measuring time
        inline explicit Stopwatch(const Unit unit = Unit::automatic) noexcept;
        // display and return lap time
        typename C::duration lap(void) noexcept;
        // display and return split time
        typename C::duration split(void) noexcept;
        // pause measuring time
        typename C::duration pause(void) noexcept;
        // reset measured time
        void reset(void) noexcept;
        // unpause stopwatch
        void unpause(void) noexcept;
      public:
        // used clock
        using clock = C;
        // unit to be used when displaying elapsed time
        const Unit unit;
      private:
        // display elapsed time
        void display(const std::chrono::nanoseconds::rep nanoseconds, const std::string& asset) noexcept;
        // used to keep track of the current status
        bool is_paused_;
        // time either at construction or from last unpause 
        typename C::time_point previous_;
        // time either at construction or from last unpause/lap
        typename C::time_point previous_lap_;
        // measured elapsed time
        typename C::duration duration_;
        // measured elapsed time
        typename C::duration duration_lap_;
      public:
        // Stopwatch settings
        struct Settings final {
          // display or not elapsed time
          static bool display;
          // not meant to be instantiated
          Settings(void) = delete;
        };
        // Stopwatch assets
        struct Assets final {
          // lap time prefix
          static std::string lap_time;
          // split time prefix
          static std::string split_time;
          // elapsed time prefix
          static std::string elapsed_time;
          // warning message prefix
          static std::string wrn_label;
          // error message prefix
          static std::string err_label;
          // not meant to be instantiated
          Assets(void) = delete;
        };
      private:
        inline void warning(const char*) noexcept;
        inline void error(const char*) noexcept;
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
      unit((unit > Unit::automatic) ? warning("invalid unit, automatic used instead"), Unit::automatic : unit),
      is_paused_(false),
      previous_(C::now()),
      previous_lap_(previous_),
      duration_(0),
      duration_lap_(0)
    {}

    template<typename C>
    typename C::duration Stopwatch<C>::lap(void) noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      if (is_paused_ == false) {
        // save elapsed times
        duration_     += now - previous_;
        duration_lap_ += now - previous_lap_;

        // display lap time
        display(std::chrono::nanoseconds(duration_lap_).count(), Assets::lap_time);
        
        // reset measured time
        previous_     = C::now();
        previous_lap_ = previous_;
        duration_lap_ = typename C::duration(0);
      }
      else warning("cannot measure lap, is paused");

      return duration_lap_;
    }

    template<typename C>
    typename C::duration Stopwatch<C>::split(void) noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      if (is_paused_ == false) {
        // save elapsed times
        duration_     += now - previous_;
        duration_lap_ += now - previous_lap_;

        // display split time
        display(std::chrono::nanoseconds(duration_).count(), Assets::split_time);
        
        // save time point
        previous_     = C::now();
        previous_lap_ = previous_;
      }
      else warning("cannot measure split, is paused");
      
      return duration_;
    }

    template<typename C>
    typename C::duration Stopwatch<C>::pause(void) noexcept
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

      return duration_;
    }

    template<typename C>
    void Stopwatch<C>::reset(void) noexcept
    {
      if (is_paused_ == true) {
        // reset measured time
        duration_     = typename C::duration(0);
        duration_lap_ = typename C::duration(0);
      }
      else warning("cannot reset, must be paused");
    }

    template<typename C>
    void Stopwatch<C>::unpause(void) noexcept
    {
      if (is_paused_ == true) {
        // unpause
        is_paused_ = false;

        // reset measured time
        previous_     = C::now();
        previous_lap_ = previous_;
      }
      else warning("cannot unpause further, is already unpaused");
    }
    
    template<typename C>
    void Stopwatch<C>::display(const std::chrono::nanoseconds::rep nanoseconds, const std::string& asset) noexcept
    {
      // conditionally display elapsed time
      if (Settings::display) {
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
    }

    template<typename C>
    void Stopwatch<C>::warning(const char* message) noexcept
    {
      wrn_ostream << Assets::wrn_label << message << std::endl;
    }

    template<typename C>
    void Stopwatch<C>::error(const char* message) noexcept
    {
      err_ostream << Assets::err_label << message << std::endl;
    }

    template<typename C>
    bool Stopwatch<C>::Settings::display = true;

    template<typename C>
    std::string Stopwatch<C>::Assets::lap_time = "lap time: ";

    template<typename C>
    std::string Stopwatch<C>::Assets::split_time = "split time: ";

    template<typename C>
    std::string Stopwatch<C>::Assets::elapsed_time = "elapsed time: ";

    template<typename C>
    std::string Stopwatch<C>::Assets::wrn_label = "warning: Stopwatch: ";

    template<typename C>
    std::string Stopwatch<C>::Assets::err_label = "error: Stopwatch: ";

    template<typename C, typename F, typename... A>
    typename C::duration execution_time(const F function, const size_t repetitions, const A... arguments)
    {
      Stopwatch<C> stopwatch(Unit::automatic);

      for (size_t iteration = 0; iteration < repetitions; ++iteration)
        function(arguments...);

      return stopwatch.stop();
    }

    #undef  CHRONOMETRO_EXECUTION_TIME
    #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)           \
      [&](void) {                                                            \
        Chronometro::Stopwatch<> stopw_atch;                                 \
        const size_t repet_itions = repetitions;                             \
        for (size_t itera_tion = 0; itera_tion < repet_itions; ++itera_tion) \
          function(__VA_ARGS__);                                             \
        return stopw_atch.split();                                            \
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
