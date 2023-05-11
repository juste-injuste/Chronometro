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
#include <functional>
// --Chronometro library: backend forward declaration-----------------------------
namespace Chronometro { namespace Backend {
  // measure elapsed time
  class Stopwatch;
  // displayed time units
  enum Unit : char {
    ns = 0, us, ms, s, min, h
  };
  // benchmark function execution
  void execution_speed(std::function<void(void)> function, size_t N = 1000, Unit unit = Unit(-1));
}}
// --Chronometro library: frontend forward declarations---------------------------
namespace Chronometro { inline namespace Frontend {
  using Backend::Unit;
  using Backend::Stopwatch;
  using Backend::execution_speed;
}}
// --Chronometro library: backend struct and class definitions--------------------
namespace Chronometro { namespace Backend {
  class Stopwatch {
    public:
      Stopwatch(Unit unit = ms);
      ~Stopwatch();
      // restart stopwatch
      void start(Unit unit = Unit(-1));
      // stop stopwatch and display elapsed time
      void stop(Unit unit = Unit(-1));
    private:
      // units that will be displayed on stop
      Unit unit_;
      // starting time
      std::chrono::system_clock::time_point start_;
  };
}}
// --Chronometro library: backend struct and class member definitions-------------
namespace Chronometro { namespace Backend {
  Stopwatch::Stopwatch(Unit unit)
    : // member initialization list
    unit_(unit),
    start_(std::chrono::high_resolution_clock::now())
  {}

  Stopwatch::~Stopwatch()
  {
    stop();
  }

  void Stopwatch::start(Unit unit)
  {
    // if unit == -1, use previously set unit
    unit_ = (unit == -1) ? unit_ : unit;
    // measure current time
    start_ = std::chrono::high_resolution_clock::now();
  }

  void Stopwatch::stop(Unit unit)
  {
    // measure current time
    auto stop = std::chrono::high_resolution_clock::now();
    // if unit == -1, use previously set unit
    unit_ = (unit == -1) ? unit_ : unit;
    // display elapsed time in unit_ units
    std::cout << "time elapsed: ";
    switch(unit_) {
      case ns:
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start_).count() << "ns";
        return;
      case us:
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stop - start_).count() << "us";
        return;
      case ms:
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start_).count() << "ms";
        return;
      case s:
        std::cout << std::chrono::duration_cast<std::chrono::seconds>(stop - start_).count() << 's';
        return;
      case min:
        std::cout << std::chrono::duration_cast<std::chrono::minutes>(stop - start_).count() << "min";
        return;
      case h:
        std::cout << std::chrono::duration_cast<std::chrono::hours>(stop - start_).count() << 'h';
        return;
    }
  }

  void execution_speed(std::function<void(void)> function, size_t N, Unit unit)
  {
    // start stopwatch
    Stopwatch stopwatch(unit);
    // execute function N times
    for (size_t iteration = 0; iteration < N; ++iteration)
      function();
    // stopwatch stops and displays elapsed time at the end of the function
  }
}}
#endif