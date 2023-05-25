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
    ns = 0, us, ms, s, min, h, automatic
  };
  // returns the appropriate unit to display time
  Unit appropriate_unit(std::chrono::high_resolution_clock::duration duration);
}}
// --Chronometro library: frontend forward declarations---------------------------
namespace Chronometro { inline namespace Frontend {
  using Backend::Unit;
  using Backend::Stopwatch;

  template <typename F, typename... A>
  void execution_time(const F function, const size_t repetitions, const A... arguments);

  #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)
}}
// --Chronometro library: backend struct and class definitions--------------------
namespace Chronometro { namespace Backend {
  class Stopwatch {
    public:
      Stopwatch(const Unit unit = Unit::automatic, const bool display_on_destruction = false) noexcept;
      ~Stopwatch() noexcept;
      // restart stopwatch
      void start(void) noexcept;
      // stop stopwatch and display elapsed time
      void stop(void) noexcept;
    private:
      // if true, elapsed time will be displayed on destruction
      const bool display_on_destruction_;
      // units that will be displayed on stop
      Unit unit_;
      // starting and ending time
      std::chrono::high_resolution_clock::time_point start_;
  };
}}
// --Chronometro library: backend struct and class member definitions-------------
namespace Chronometro { namespace Backend {
  Stopwatch::Stopwatch(const Unit unit, const bool display_on_destruction) noexcept
    : // member initialization list
    display_on_destruction_(display_on_destruction),
    unit_(unit),
    start_(std::chrono::high_resolution_clock::now())
  {}

  Stopwatch::~Stopwatch() noexcept
  {
    if (display_on_destruction_)
      stop();
  }

  void Stopwatch::start(void) noexcept
  {
    // measure current time
    start_ = std::chrono::high_resolution_clock::now();
  }

  void Stopwatch::stop(void) noexcept
  {
    // measure duration
    std::chrono::high_resolution_clock::duration duration = std::chrono::high_resolution_clock::now() - start_;

    // display elapsed time in unit_ units
    std::cout << "time elapsed: ";

    // if unit_ == automatic, deduce the appropriate unit
    switch((unit_ == Unit::automatic) ? appropriate_unit(duration) : unit_) {
      case Unit::ns:
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() << "ns\n";
        return;
      case Unit::us:
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << "us\n";
        return;
      case Unit::ms:
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "ms\n";
        return;
      case Unit::s:
        std::cout << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << "s\n";
        return;
      case Unit::min:
        std::cout << std::chrono::duration_cast<std::chrono::minutes>(duration).count() << "min\n";
        return;
      case Unit::h:
        std::cout << std::chrono::duration_cast<std::chrono::hours>(duration).count() << "h\n";
        return;
      default:
        std::cerr << "error: chronometro: invalid unit\n";
    }
  }

  Unit appropriate_unit(std::chrono::high_resolution_clock::duration duration)
  {
    std::chrono::nanoseconds::rep nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

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
// --Chronometro library: frontend definitions------------------------------------
namespace Chronometro { inline namespace Frontend {
  template <typename F, typename... A>
  void execution_time(const F function, const size_t repetitions, const A... arguments)
  {
    Stopwatch stopwatch(Unit::automatic, true);
    for (size_t iteration = 0; iteration < repetitions; ++iteration)
      function(arguments...);
  }

  #undef  CHRONOMETRO_EXECUTION_TIME
  #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)             \
    {                                                                        \
    Chronometro::Stopwatch stopwatch(Chronometro::Unit::automatic, true);    \
    for (size_t iteration = 0; iteration < size_t(repetitions); ++iteration) \
      function(__VA_ARGS__);                                                 \
    }
}}
#endif