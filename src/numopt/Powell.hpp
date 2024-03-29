/*
  Copyright (C) 2005-2012 Steven L. Scott

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef BOOM_NUMOPT_POWELL_HPP_
#define BOOM_NUMOPT_POWELL_HPP_

#include <numopt.hpp>

namespace BOOM{
  // A derivative free minimization routine based on Powell's NEWUOA
  // algorithm.  The README file from NEWUOA can be found as a comment
  // in the corresponding cpp file.
  class PowellMinimizer {
   public:
    PowellMinimizer(Target f);
    void minimize(const Vector &initial_value);

    void set_evaluation_limit(long number_of_evaluations);
    void set_precision(double precision = 1e-6);
    void set_initial_stepsize(double stepsize);

    const Vector & minimizing_value()const{return minimizing_x_;}
    double minimum()const{return minimum_;}

    int number_of_function_evaluations()const{
      return number_of_function_evaluations_;}

   private:
    Target f_;
    double minimum_;
    Vector minimizing_x_;

    double initial_stepsize_;    // rho_begin
    double desired_precision_;   // rho_end
    long number_of_interpolating_points_;

    int number_of_function_evaluations_;
    long max_number_of_function_evaulations_;
  };
}

#endif //  BOOM_NUMOPT_POWELL_HPP_
