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

#include <Models/Policies/MixtureDataPolicy.hpp>
#include <distributions.hpp>

namespace BOOM{
  MixtureDataPolicy::MixtureDataPolicy(uint S)
    : dat_(new std::vector<Ptr<Data> >),
      pkey_(new CatKey(S))
  {
  }


  MixtureDataPolicy::MixtureDataPolicy(const MixtureDataPolicy &rhs)
    : Model(rhs),
      DataTraits(rhs),
      dat_(new std::vector<Ptr<DataType> >(*rhs.dat_)),
      latent_(std::vector<Ptr<CategoricalData> >(rhs.latent_)),
      pkey_(rhs.pkey_)
  {
    // copy pointer elements for observed data
    // create new data storage for latent data
    uint n = latent_.size();
    for(uint i=0; i<n; ++i){
      latent_[i] = latent_[i]->clone();
    }
  }

  MixtureDataPolicy & MixtureDataPolicy::operator=(const MixtureDataPolicy &rhs){
    if(&rhs!=this){
      dat_ = new std::vector<Ptr<DataType> >(*rhs.dat_);
      latent_ = rhs.latent_;
    }
    return *this;
  }


  void MixtureDataPolicy::clear_data(){
    dat().clear();
    latent_data().clear();
  }

  //------------------------------------------------------------


  void MixtureDataPolicy::set_data(const DatasetType &d){
    clear_data();
    for(uint i=0; i<d.size(); ++i) add_data(d[i]);
  }

  std::vector<Ptr<CategoricalData> >  &
  MixtureDataPolicy::latent_data(){ return latent_; }

  const std::vector<Ptr<CategoricalData> >  &
  MixtureDataPolicy::latent_data()const{ return latent_;}



  void MixtureDataPolicy::set_data(const dsetPtr d){
    clear_data();
    for(uint i=0; i<d->size(); ++i) add_data((*d)[i]);
  }

  void MixtureDataPolicy::add_data(Ptr<DataType> d){
    dat().push_back(d);
    uint h = random_int(0, pkey_->size()-1);
    NEW(CategoricalData, pcat)(h, pkey_);
    latent_data().push_back(pcat);
  }

void MixtureDataPolicy::combine_data(const Model & other, bool){
  const MixtureDataPolicy & m(dynamic_cast<const MixtureDataPolicy &>(other));
  const std::vector<Ptr<Data> > & d(*m.dat_);
  dat_->reserve(dat_->size() + d.size());
  dat_->insert(dat_->end(), d.begin(), d.end());

  const std::vector<Ptr<CategoricalData> > & mis(m.latent_);
  latent_.reserve(latent_.size() + mis.size());
  latent_.insert(latent_.end(), mis.begin(), mis.end());
}

}
