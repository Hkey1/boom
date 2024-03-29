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
#include "CompositeParamPolicy.hpp"
#include <algorithm>

namespace BOOM{
  typedef CompositeParamPolicy CPP;

  CPP::CompositeParamPolicy()
  {}

  CPP::CompositeParamPolicy(const CPP &rhs)
    : Model(rhs)
  { }


  CPP & CPP::operator=(const CompositeParamPolicy &)
  {return *this;}

  void CPP::add_params(Ptr<Params> p){ t_.push_back(p); }

  void CPP::add_model(Ptr<Model> m){
    if(have_model(m)) return;
    models_.push_back(m);
    ParamVec tmp(m->t());
    std::copy(tmp.begin(), tmp.end(), back_inserter(t_));
  }
  void CPP::drop_model(Ptr<Model> m){
    if(!have_model(m)) return;
    std::remove(models_.begin(), models_.end(), m);  //???
    ParamVec tmp(m->t());
    for(uint i = 0; i<tmp.size(); ++i){
      std::remove(t_.begin(), t_.end(), tmp[i]);
    }
  }
  void CPP::clear(){
    models_.clear();
    t_.clear();
  }

  ParamVec CPP::t(){return t_;}
  const ParamVec CPP::t()const{return t_;}

  bool CPP::have_model(Ptr<Model> m)const{
    return std::find(models_.begin(),models_.end(), m)
      != models_.end();}


}
