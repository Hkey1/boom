/*
  Copyright (C) 2006 Steven L. Scott

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
#include "ConstrainedVectorParams.hpp"
#include <algorithm>

namespace BOOM{

  typedef VectorConstraint VC;
  typedef ElementConstraint EC;
  EC::ElementConstraint(uint el, double val)
    : element_(el),
      value_(val)
  {}

  bool EC::check(const Vec &v)const{ return v[element_]==value_;}
  void EC::impose(Vec &v)const{ v[element_] = value_;}
  Vec EC::expand(const Vec &v)const{
    Vec ans(v.size()+1);
    Vec::const_iterator b(v.begin()), e(v.end());
    Vec::const_iterator pos = b+element_;
    std::copy(b,pos, ans.begin());
    std::copy(pos, e, ans.begin()+element_ + 1);
    impose(ans);
    return ans;
  }

  Vec EC::reduce(const Vec &v)const{
    if(v.size()==0) return Vec(0);
    Vec ans(v.size()-1);
    Vec::const_iterator b(v.begin()), e(v.end());
    std::copy(b, b+element_, ans.begin());
    std::copy(b+element_+1, e, ans.begin()+element_);
    return ans;
  }
  //------------------------------------------------------------
  typedef SumConstraint SC;
  SC::SumConstraint(double x)
    : sum_(x)
  {}

  bool SC::check(const Vec &v)const{ return v.sum()==sum_;}

  void SC::impose(Vec &v)const{
    double tot = v.sum();
    v.back() = sum_-tot;
  }

  Vec SC::expand(const Vec &v)const{
    Vec ans(v.size()+1);
    std::copy(v.begin(), v.end(), ans.begin());
    impose(ans);
    return ans;
  }

  Vec SC::reduce(const Vec &v)const{
    Vec ans(v.begin(), v.end()-1);
    return ans;
  }

  //============================================================

  typedef ConstrainedVectorParams CVP;

  CVP::ConstrainedVectorParams(uint p, double x, Ptr<VC> vc)
    : VectorParams(p,x),
      c_(vc)
  {
    if(!vc) c_ = new NoConstraint;
  }

  CVP::ConstrainedVectorParams(const Vec &v, Ptr<VC> vc)
    : VectorParams(v),
      c_(vc)
  {
    if(!vc) c_ = new NoConstraint;
  }

  CVP::ConstrainedVectorParams(const CVP &rhs)
    : Data(rhs),
      Params(rhs),
      VectorParams(rhs),
      c_(rhs.c_)
  {
  }

  CVP * CVP::clone()const{return new CVP(*this);}

  Vec CVP::vectorize(bool minimal)const{
    if(minimal) return c_->reduce(value());
    return value();
  }

  Vec::const_iterator CVP::unvectorize(Vec::const_iterator &v, bool minimal){
    Vec tmp(vectorize(minimal));
    Vec::const_iterator e = v+tmp.size();
    tmp.assign(v,e);
    if(minimal) set(c_->expand(tmp));
    else set(tmp);
    return e;
  }

  Vec::const_iterator CVP::unvectorize(const Vec &v, bool minimal){
    Vec::const_iterator b(v.begin());
    return unvectorize(b, minimal);
  }

  //============================================================


}
