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
#include <Models/PosteriorSamplers/ExchangeableDirichletSampler.hpp>
#include <Samplers/ScalarSliceSampler.hpp>
#include <distributions.hpp>

namespace BOOM{

typedef ExchangeableDirichletSampler EDS;

EDS::ExchangeableDirichletSampler(DirichletModel *m, Ptr<DoubleModel> pri)
    : mod_(m),
      pri_(pri)
{}


double EDS::logpri()const{
  const Vec & nu(mod_->nu());
  double ans=0;
  for(uint i=0; i<nu.size(); ++i) ans += pri_->logp(nu[i]);
  return ans;
}

struct target
    : public ScalarTargetFun
{
  const Vec & sumlog_;
  double nobs_;
  Vec & nu_;
  uint which_;
  Ptr<DoubleModel> pri_;

  target(const Vec & sumlog,
         double nobs,
         Vec & nu_,
         uint i,
         Ptr<DoubleModel> &pri)
      : sumlog_(sumlog),
        nobs_(nobs),
        nu_(nu_),
        which_(i),
        pri_(pri)
  {}

  double operator()(double nu)const{
    nu_[which_] = nu;
    double ans = pri_->logp(nu);
    if(!finite(ans)) return ans;
    ans += dirichlet_loglike(nu_, 0, 0, sumlog_, nobs_);
    return ans;
  }
};

void EDS::draw(){

  Vec nu(mod_->nu());
  uint d = nu.size();
  const Vec & sumlog(mod_->suf()->sumlog());
  double nobs = mod_->suf()->n();

  for(uint i=0; i<d; ++i){
    target logp(sumlog, nobs, nu, i, pri_);
    ScalarSliceSampler sam(logp);
    sam.set_lower_limit(0);
    nu[i] = sam.draw(nu[i]);
  }
  mod_->set_nu(nu);
}

}
