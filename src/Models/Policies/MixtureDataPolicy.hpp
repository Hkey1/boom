/*
  Copyright (C) 2007 Steven L. Scott

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
#ifndef BOOM_MIXTURE_DATA_POLICIES_HPP
#define BOOM_MIXTURE_DATA_POLICIES_HPP
#include <Models/ModelTypes.hpp>
#include <Models/Policies/DataInfoPolicy.hpp>
#include <Models/CategoricalData.hpp>
#include <Models/DataTypes.hpp>
#include <fstream>
#include <list>

namespace BOOM{
 //======================================================================

  class MixtureDataPolicy : public DefaultDataInfoPolicy<Data>{

  public:
    typedef Data DataType;
    typedef MixtureDataPolicy DataPolicy;
    typedef std::vector<Ptr<DataType> > DatasetType;
    typedef Ptr<DatasetType, false> dsetPtr;
  protected:
  public:
    MixtureDataPolicy(uint S);
    MixtureDataPolicy(const MixtureDataPolicy &);
    MixtureDataPolicy * clone()const=0;
    MixtureDataPolicy & operator=(const MixtureDataPolicy &);

    virtual void clear_data();

    DatasetType & dat(){return *dat_;}
    const DatasetType & dat()const{return *dat_;}

    std::vector<Ptr<CategoricalData> >  & latent_data();
    const std::vector<Ptr<CategoricalData> >  & latent_data()const;

    virtual void set_data(const dsetPtr d);
    virtual void set_data(const DatasetType &d);

    template <class FwdIt>
    void set_data(FwdIt Beg, FwdIt End);
    virtual void add_data(Ptr<Data> dp);
    virtual void combine_data(const Model &, bool just_suf=true);

    void add_data_with_known_source(Ptr<Data>, int source);

    // Sets the source of each observation.  Negative numbers mean the
    // source is uncertain.  Non negative numbers less than the number
    // of mixture components indicate which component generated each
    // observation.  Greater numbers are an error.  The length of the
    // argument must match the number of observations at the time
    void set_data_source(const std::vector<int> &which_mixture_component);

    // Indicates which mixture component the specified observation
    // comes from.  A negative answer (the usual case) means the
    // source of the observation is uncertain.
    int which_mixture_component(int observation_number)const;
  private:
    dsetPtr dat_;  // Model owns pointer to its data.
    std::vector<Ptr<CategoricalData> > latent_;
    Ptr<CatKey> pkey_;

    // The following vector will be empty in most cases.  It will be filled when
    std::vector<int> known_data_source_;
  };
  //======================================================================

  template<class FwdIt>
  void MixtureDataPolicy::set_data(FwdIt Beg, FwdIt End){
    clear_data();
    while(Beg!=End){
      add_data(*Beg);
      ++Beg;
    }
  }

  //============================================================

}
#endif //BOOM_MIXTURE_DATA_POLICIES_HPP
