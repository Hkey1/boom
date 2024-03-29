/*
  Copyright (C) 2005 Steven L. Scott

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
#include "ParamHolder.hpp"
#include <Models/ParamTypes.hpp>

namespace BOOM{
  typedef ParamHolder PH;

  PH::ParamHolder(Ptr<Params> held, Vec &Wsp)
    : v(Wsp),
      prm(held)
  {}

  PH::ParamHolder(const Vec & x, Ptr<Params> held, Vec &Wsp)
    : v(Wsp),
      prm(held)
    {
      v = prm->vectorize(true);
      prm->unvectorize(x, true);
    }

  PH::~ParamHolder(){
    prm->unvectorize(v, true);
  }

  //------------------------------------------------------------

  typedef ParamVecHolder PVH;
  PVH::ParamVecHolder(const ParamVec &held, Vec &Wsp)
    : v(Wsp),
      prm(held)
  {}

  PVH::ParamVecHolder(const Vec & x, const ParamVec & held, Vec &Wsp)
    : v(Wsp),
      prm(held)
    {
      v = vectorize(prm, true);
      unvectorize(prm, x, true);
    }


  PVH::~ParamVecHolder(){
    unvectorize(prm, v, true);
  }



}
