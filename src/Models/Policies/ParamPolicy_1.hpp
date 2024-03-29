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
#ifndef BOOM_PARAM_POLICY_1_HPP
#define BOOM_PARAM_POLICY_1_HPP

#include<cpputil/Ptr.hpp>
#include <Models/ModelTypes.hpp>

namespace BOOM{

  template <class P>
  class ParamPolicy_1 : virtual public Model{
  public:
    typedef P ParamType;
    typedef ParamPolicy_1<P> ParamPolicy;

    ParamPolicy_1();
    ParamPolicy_1(Ptr<P> pPrm);
    ParamPolicy_1(const ParamPolicy_1 &rhs);
    ParamPolicy_1<P> * clone()const=0;
    ParamPolicy_1<P> & operator=(const ParamPolicy_1 &rhs);

    void set_prm(Ptr<P> p){prm_ = p; set_t();}
    Ptr<P> prm(){return prm_;}
    const Ptr<P> prm()const{return prm_;}
    P & prm_ref(){return *prm_;}
    const P & prm_ref()const{return *prm_;}

    virtual void set_param_filename(const string &);
    // over-rides for abstract base Model
    ParamVec t();
    const ParamVec t()const;

  private:
    Ptr<P> prm_;
    ParamVec t_;
    void set_t();
  };
  //------------------------------------------------------------

  template<class P>
  void ParamPolicy_1<P>::set_t(){
    t_ = ParamVec(1,prm_);
  }

  template<class P>
  ParamPolicy_1<P>::ParamPolicy_1()
    : prm_()
    {
      set_t();
    }
  template<class P>
  ParamPolicy_1<P>::ParamPolicy_1(Ptr<P> pPrm)
      : prm_(pPrm)
    {
      set_t();
    }

  template<class P>
  ParamPolicy_1<P>::ParamPolicy_1(const ParamPolicy_1 &rhs)
    : Model(rhs),
      prm_(rhs.prm_->clone())
    {
      set_t();
    }

  template<class P>
  ParamPolicy_1<P> * ParamPolicy_1<P>::clone()const{
    return new ParamPolicy_1<P>(*this);}

  template<class P>
  ParamPolicy_1<P> & ParamPolicy_1<P>::operator=
  (const ParamPolicy_1 &rhs){
    if(&rhs != this){
      prm_ = rhs.prm_->clone();
      set_t();
    }
    return *this;
  }

  template<class P>
  void ParamPolicy_1<P>::set_param_filename(const string &fn){
    prm_->set_fname(fn); }

  template<class P>
  ParamVec ParamPolicy_1<P>::t(){return t_;}

  template<class P>
  const ParamVec ParamPolicy_1<P>::t()const{return t_;}


}
#endif // BOOM_PARAM_POLICY_1_HPP
