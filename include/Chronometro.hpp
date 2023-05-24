// --author-----------------------------------------------------------------------
// 
// Justin Asselin (juste-injuste)
// justin.asselin@usherbrooke.ca
// https://github.com/juste-injuste/Chronometro
// 
// --liscence---------------------------------------------------------------------
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
// --versions---------------------------------------------------------------------
// version 1.0.0 initial release
// --inclusion guard--------------------------------------------------------------
#ifndef CHRONOMETRO_HPP
#define CHRONOMETRO_HPP
// --necessary standard libraries-------------------------------------------------
#include <chrono>
#include <iostream>
// --Chronometro library: backend forward declaration-----------------------------
namespace Chronometro { namespace Backend {
  // measure elapsed time
  class Stopwatch;
  // displayed time units
  enum class Unit : unsigned char {
    ns = 0, us, ms, s, min, h, keep, automatic
  };
  // returns the appropriate unit to display time
  Unit appropriate_unit(std::chrono::nanoseconds time);
}}
// --Chronometro library: frontend forward declarations---------------------------
namespace Chronometro { inline namespace Frontend {
  using Backend::Unit;
  using Backend::Stopwatch;
  #define CHRONOMETRO_EXECUTION_SPEED(function, repetitions, arguments...)
}}
// --Chronometro library: backend struct and class definitions--------------------
namespace Chronometro { namespace Backend {
  class Stopwatch {
    public:
      Stopwatch(const bool display_on_destruction = false, const Unit unit = Unit::automatic) noexcept;
      ~Stopwatch() noexcept;
      // restart stopwatch
      void start(const Unit unit = Unit::keep) noexcept;
      // stop stopwatch and display elapsed time
      void stop(const Unit unit = Unit::keep) noexcept;
    private:
      // if true, elapsed time will be displayed on destruction
      const bool display_on_destruction_;
      // units that will be displayed on stop
      Unit unit_;
      // starting time
      std::chrono::system_clock::time_point start_;
  };
}}
// --Chronometro library: backend struct and class member definitions-------------
namespace Chronometro { namespace Backend {
  Stopwatch::Stopwatch(const bool display_on_destruction, const Unit unit) noexcept
    : // member initialization list
    display_on_destruction_(display_on_destruction),
    unit_(unit),
    start_(std::chrono::high_resolution_clock::now())
  {
    if (unit == Unit::keep) {
      unit_ = Unit::ms;
      std::clog << "warning: stopwatch: invalid unit, ms used instead\n";
    }
  }

  Stopwatch::~Stopwatch() noexcept
  {
    if (display_on_destruction_)
      stop();
  }

  void Stopwatch::start(const Unit unit) noexcept
  {
    // if unit == keep, use previously set unit
    unit_ = (unit == Unit::keep) ? unit_ : unit;

    // measure current time
    start_ = std::chrono::high_resolution_clock::now();
  }

  void Stopwatch::stop(const Unit unit) noexcept
  {
    // measure current time
    auto stop = std::chrono::high_resolution_clock::now();
    
    // if unit == keep, use previously set unit
    unit_ = (unit == Unit::keep) ? unit_ : unit;

    // display elapsed time in unit_ units
    std::cout << "time elapsed: ";

    // if unit_ == automatic, deduce the appropriate unit
    switch((unit_ == Unit::automatic) ? appropriate_unit(stop - start_) : unit_) {
      case Unit::ns:
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start_).count() << "ns\n";
        return;
      case Unit::us:
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stop - start_).count() << "us\n";
        return;
      case Unit::ms:
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start_).count() << "ms\n";
        return;
      case Unit::s:
        std::cout << std::chrono::duration_cast<std::chrono::seconds>(stop - start_).count() << "s\n";
        return;
      case Unit::min:
        std::cout << std::chrono::duration_cast<std::chrono::minutes>(stop - start_).count() << "min\n";
        return;
      case Unit::h:
        std::cout << std::chrono::duration_cast<std::chrono::hours>(stop - start_).count() << "h\n";
        return;
      default:
        std::cerr << "error: chronometro: invalid unit\n";
    }
  }
  
  Unit appropriate_unit(std::chrono::nanoseconds time)
  {
    auto count = time.count();

    // display in minute: 10 h < time
    if (count > 3600e9 * 10)
      return Unit::h;

    // display in minutes: 10 min < time <= 10 h
    if (count > 60e9 * 10)
      return Unit::min;

    // display in seconds: 10 s < time <= 10 m
    if (count > 1e9 * 10)
      return Unit::s;

    // display in milliseconds: 10 ms < time <= 10 s
    if (count > 1e6 * 10)
      return Unit::ms;

    // display in microseconds: 10 us < time <= 10 ms
    if (count > 1e3 * 10)
      return Unit::us;
      
    // display in minute: time <= 10 us
    return Unit::ns;
  }
}}
// --Chronometro library: frontend definitions------------------------------------
namespace Chronometro { inline namespace Frontend {
  #undef  CHRONOMETRO_EXECUTION_SPEED
  #define CHRONOMETRO_EXECUTION_SPEED(function, repetitions, arguments...)   \
    {                                                                        \
    Stopwatch stopwatch(true);                                               \
    for (size_t iteration = 0; iteration < size_t(repetitions); ++iteration) \
      function(arguments);                                                   \
    }
}}
#endif