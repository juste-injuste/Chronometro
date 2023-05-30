// --author------------------------------------------------------------------------------
// 
// Justin Asselin (juste-injuste)
// justin.asselin@usherbrooke.ca
// https://github.com/juste-injuste/Chronometro
// 
// --liscence----------------------------------------------------------------------------
// 
// MIT License
// 
// Copyright (c) 2023 Justin Asselin (juste-injuste)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//  
// --versions----------------------------------------------------------------------------
//
// version 1.0 initial release
//
// --description-------------------------------------------------------------------------
//
// Chronometro is a simple and lightweight C++11 (and newer) library that allows you to
// measure the execution time of functions or code blocks. See the included README.MD file
// for more information
//
// --inclusion guard---------------------------------------------------------------------
#ifndef CHRONOMETRO_HPP
#define CHRONOMETRO_HPP
// --necessary standard libraries--------------------------------------------------------
#include <chrono>
#include <iostream>
#include <iomanip>
// --Chronometro library: backend forward declaration------------------------------------
namespace Chronometro { namespace Backend {
  // measure elapsed time
  class Stopwatch;

  // time units for displaying
  enum class Unit : uint8_t {
    ns, us, ms, s, min, h, automatic
  };

  // returns the appropriate unit to display time
  Unit appropriate_unit(const std::chrono::nanoseconds::rep nanoseconds);
}}
// --Chronometro library: frontend forward declarations----------------------------------
namespace Chronometro { inline namespace Frontend {
  using Backend::Unit;
  using Backend::Stopwatch;

  // measure function execution time
  template <typename F, typename... A>
  std::chrono::high_resolution_clock::duration execution_time(const F function, const size_t repetitions, const A... arguments);

  // measure function execution time
  #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)
}}
// --Chronometro library: backend struct and class definitions---------------------------
namespace Chronometro { namespace Backend {
  class Stopwatch {
    public:
      inline explicit Stopwatch(const Unit unit = Unit::automatic) noexcept;
      // start measuring time
      inline void start(void) noexcept;
      // pause time measurement
      void pause(void) noexcept;
      // stop time measurement and display elapsed time
      std::chrono::high_resolution_clock::duration stop(void) noexcept;
      // reset measured time and start measuring time
      inline void restart(void);
    private:
      // units that will be displayed on stop
      Unit unit_;
      // used to keep track stopwatch status
      bool paused_;
      // measured time
      std::chrono::high_resolution_clock::duration duration_;
      // starting and ending time
      std::chrono::high_resolution_clock::time_point start_;
  };
}}
// --Chronometro library: backend struct and class member definitions--------------------
namespace Chronometro { namespace Backend {
  Stopwatch::Stopwatch(const Unit unit) noexcept
    : // member initialization list
    unit_(unit),
    paused_(false),
    duration_(0),
    start_(std::chrono::high_resolution_clock::now())
  {}

  void Stopwatch::start(void) noexcept
  {
    // unpause
    paused_ = false;
    // measure current time
    start_ = std::chrono::high_resolution_clock::now();
  }

  void Stopwatch::pause(void) noexcept
  {
    // measure elapsed time
    const std::chrono::high_resolution_clock::duration duration = std::chrono::high_resolution_clock::now() - start_;
    
    // add elapsed time up to now if not paused
    if (!paused_) {
      duration_ += duration;
      paused_ = true;
    }
    else std::cerr << "warning: Stopwatch: already paused\n";
  }

  std::chrono::high_resolution_clock::duration Stopwatch::stop(void) noexcept
  {
    // pause time measurement
    pause();

    // measured time in nanoseconds
    const std::chrono::nanoseconds::rep nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration_).count();
    
    // if unit_ == automatic, deduce the appropriate unit
    switch((unit_ == Unit::automatic) ? appropriate_unit(nanoseconds) : unit_) {
      case Unit::ns:
        std::cout << "elapsed time: " << nanoseconds << " ns\n";
        break;
      case Unit::us:
        std::cout << "elapsed time: " << nanoseconds / 1000 << " us\n";
        break;
      case Unit::ms:
        std::cout << "elapsed time: " << nanoseconds / 1000000 << " ms\n";
        break;
      case Unit::s:
        std::cout << "elapsed time: " << nanoseconds / 1000000000 << " s\n";
        break;
      case Unit::min:
        std::cout << "elapsed time: " << nanoseconds / 60000000000 << " min\n";
        break;
      case Unit::h:
        std::cout << "elapsed time: " << nanoseconds / 3600000000000 << " h\n";
        break;
      default: std::cerr << "error: Stopwatch: invalid time unit\n";
    }

    return duration_;
  }

  void Stopwatch::restart(void)
  {
    // reset measured duration
    duration_ = std::chrono::high_resolution_clock::duration(0);
    
    // start measuring time
    start();
  }

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
}}
// --Chronometro library: frontend definitions-------------------------------------------
namespace Chronometro { inline namespace Frontend {
  template <typename F, typename... A>
  std::chrono::high_resolution_clock::duration execution_time(const F function, const size_t repetitions, const A... arguments)
  {
    Stopwatch stopwatch(Unit::automatic);

    for (size_t iteration = 0; iteration < repetitions; ++iteration)
      function(arguments...);
    
    return stopwatch.stop();
  }

  #undef  CHRONOMETRO_EXECUTION_TIME
  #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)                     \
    [&](void) -> std::chrono::high_resolution_clock::duration {                      \
      Chronometro::Stopwatch stopwatch(Chronometro::Unit::automatic);                \
      for (size_t _iteration_ = 0; _iteration_ < size_t(repetitions); ++_iteration_) \
        function(__VA_ARGS__);                                                       \
      return stopwatch.stop();                                                       \
    }()
}}
#endif
