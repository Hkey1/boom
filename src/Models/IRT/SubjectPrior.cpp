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
#include "SubjectPrior.hpp"
#include <Models/MvnModel.hpp>

namespace BOOM{
  namespace IRT{

    typedef MvnSubjectPrior MSP;
    typedef SubjectPrior SP;

    //------------------------------------------------------------

    MSP::MvnSubjectPrior(Ptr<MvnModel> Mvn)
      : mvn(Mvn)
    {
      ParamPolicy::add_model(mvn);
    }

    MSP::MvnSubjectPrior(const MSP &rhs)
      : Model(rhs),
	SubjectPrior(rhs),
	ParamPolicy(rhs),
	DataPolicy(rhs),
	PriorPolicy(rhs),
	mvn(rhs.mvn->clone())
    {
      ParamPolicy::add_model(mvn);
    }

    MSP * MSP::clone()const{return new MSP(*this);}

    double MSP::pdf(Ptr<Data>dp, bool logsc)const{
      return pdf(DAT(dp), logsc);}

    double MSP::pdf(Ptr<Subject> s, bool logsc)const{
      return mvn->pdf(s->Theta(), logsc); }

    void MSP::initialize_params(){mvn->initialize_params();}

    void MSP::clear_data(){
      mvn->clear_data();
      DataPolicy::clear_data();
    }

    void MSP::add_data(Ptr<Subject> s){
      Ptr<VectorData> dp = s->Theta_prm();
      mvn->add_data(dp);
      DataPolicy::add_data(s); }

    void MSP::add_data(Ptr<Data> d){
      Ptr<Subject> s = DAT(d);
      add_data(s);
    }

    Vec MSP::mean(Ptr<Subject>)const{
      return mvn->mu();}

    Spd MSP::siginv()const{
      return mvn->siginv();}

    void MSP::set_param_filenames(const string &mu, const string &Sig){
      mvn->set_param_filenames(mu, Sig); }
    //------------------------------------------------------------

    typedef MvRegSubjectPrior RSP;


  }  // namespace IRT
} // namespace BOOM
