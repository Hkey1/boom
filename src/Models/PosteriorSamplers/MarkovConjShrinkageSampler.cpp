/*
  Copyright (C) 2005-2009 Steven L. Scott

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

#include <Models/PosteriorSamplers/MarkovConjShrinkageSampler.hpp>
#include <Models/MarkovModel.hpp>
#include <distributions.hpp>

namespace BOOM{
typedef MarkovConjShrinkageSampler MCSS;

MCSS::MarkovConjShrinkageSampler(uint dim)
  : pri_(new ProductDirichletModel(dim))
{}

MCSS::MarkovConjShrinkageSampler(const Mat & Nu)
  : pri_(new ProductDirichletModel(Nu))
{}

MCSS::MarkovConjShrinkageSampler(const Mat & Nu, const Vec & nu)
    : pri_(new ProductDirichletModel(Nu)),
      ipri_(new DirichletModel(nu))
{}

MCSS::MarkovConjShrinkageSampler(Ptr<ProductDirichletModel> Nu)
  : pri_(Nu)
{}

MCSS::MarkovConjShrinkageSampler(Ptr<ProductDirichletModel> Nu,
                                 Ptr<DirichletModel> nu)
    : pri_(Nu),
      ipri_(nu)
{}

MCSS * MCSS::clone()const{return new MCSS(*this);}

void MCSS::draw(){
  pri_->clear_data();
  if(!!ipri_) ipri_->clear_data();

  MarkovModel *m;
  for(uint i=0; i<Nmodels(); ++i){
    m=models_[i];
    Mat N = pri_->Nu() + m->suf()->trans();
    Mat Q(N);
    for(uint s=0; s<dim(); ++s)
      Q.row(s) = rdirichlet_mt(rng(), N.row(s));
    m->set_Q(Q);
    pri_->add_data(Ptr<MatrixData>(m->Q_prm()));

    if(!!ipri_){
      Vec n = ipri_->nu() + m->suf()->init();
      Vec pi0 = rdirichlet_mt(rng(), n);
      m->set_pi0(pi0);
      ipri_->add_data(Ptr<VectorData>(m->Pi0_prm()));
    }
  }
}

double MCSS::logpri()const{
  double ans=0;
  for(uint i=0; i<Nmodels(); ++i){
    ans += pri_->pdf(models_[i]->Q(), true);
    if(!!ipri_) ans += ipri_->pdf(models_[i]->pi0(), true);
  }
  return ans;
}

uint MCSS::dim()const{return pri_->Nu().nrow();}
uint MCSS::Nmodels()const{ return models_.size();}

MCSS * MCSS::add_model(MarkovModel *m){
  check_dim(m->state_space_size());
  models_.push_back(m);
  return this;
}

void MCSS::check_dim(uint d){
  if(dim()==d) return;
  if(!(models_.empty())){
    ostringstream err;
    err << "Attempt to add a Markov Model of dimension " << d
        << " to a MarkovConjShrinkageSampler of dimension " << dim()
        << "." << endl;
    throw_exception<std::runtime_error>(err.str());
  }
  Mat Nu(d,d,1.0);
  pri_->set_Nu(Nu);
}

}
