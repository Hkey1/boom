/*
  Copyright (C) 2005-2010 Steven L. Scott

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

#include <stats/ChiSquareTest.hpp>
#include <distributions.hpp>
#include <cpputil/math_utils.hpp>

namespace BOOM{
OneWayChiSquareTest::OneWayChiSquareTest(const Vec & observed,
                             const Vec & distribution)
    : observed_(observed),
      expected_(distribution * sum(observed_)),
      chi_square_(0),
      df_(observed.size()-1)
{
  for(int i = 0; i < observed_.size(); ++i) {
    double Oi = observed_[i];
    double Ei = expected_[i];
    if(Ei > 0){
      chi_square_ += pow(Oi - Ei, 2) / Ei;
    }else{
      --df_;
      if(Oi>0) chi_square_ = BOOM::infinity();
      p_value_ = 0.0;
    }
  }
  if(chi_square_ < BOOM::infinity()) {
    if(df_ > 0){
      // usual case...
      p_value_ = 1 - pchisq(chi_square_, df_);
    } else {
      // if df_ <= 0 then all but one element of distribution is zero
      p_value_ = 1;
    }
  }
}

bool OneWayChiSquareTest::is_valid()const{
  for(int i = 0; i < expected_.size(); ++i){
    if(expected_[i] < 5.0) return false;
  }
  return true;
}
double OneWayChiSquareTest::p_value()const{return p_value_;}
double OneWayChiSquareTest::degrees_of_freedom()const{return df_;}
double OneWayChiSquareTest::chi_square()const{return chi_square_;}
std::ostream & OneWayChiSquareTest::print(std::ostream &out)const{
  out << "chi_square = " << chi_square_
      << " df = " << df_
      << " p-value = " << p_value_;
  return out;
}


TwoWayChiSquareTest::TwoWayChiSquareTest(const Mat &observed)
    : chi_square_(0.0),
      df_((nrow(observed) - 1) * (ncol(observed) - 1)),
      assumptions_are_met_(true)
{
  Vec row_margin = observed * Vec(ncol(observed), 1.0);
  Vec col_margin = Vec(nrow(observed), 1.0) * observed;
  double total = sum(observed);
  for(int i = 0; i < nrow(observed); ++i) {
    for(int j =  0; j < ncol(observed); ++j) {
      double expected = row_margin[i] * col_margin[j] / total;
      chi_square_ += pow(observed(i,j) - expected, 2) / expected;
      if(expected < 5.0) assumptions_are_met_ = false;
    }
  }
  p_value_ = 1 - pchisq(chi_square_, df_);
}
double TwoWayChiSquareTest::p_value()const{return p_value_;}
double TwoWayChiSquareTest::degrees_of_freedom()const{return df_;}
double TwoWayChiSquareTest::chi_square()const{return chi_square_;}
bool TwoWayChiSquareTest::is_valid()const{return assumptions_are_met_;}
std::ostream & TwoWayChiSquareTest::print(std::ostream &out)const{
  out << "chi_square = " << chi_square_
      << " df = " << df_
      << " p-value = " << p_value_;
  return out;
}

}
