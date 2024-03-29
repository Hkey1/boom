/*
  Copyright (C) 2005-2012 Steven L. Scott

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
#include <Models/ZeroInflatedPoissonModel.hpp>
#include <Models/PosteriorSamplers/ZeroInflatedPoissonSampler.hpp>
#include <distributions.hpp>

namespace BOOM {
  ZeroInflatedPoissonSampler::ZeroInflatedPoissonSampler(
      ZeroInflatedPoissonModel *model,
      Ptr<GammaModel> lambda_prior,
      Ptr<BetaModel> zero_prob_prior)
      : model_(model),
      lambda_prior_(lambda_prior),
      zero_probability_prior_(zero_prob_prior)
      {}

  void ZeroInflatedPoissonSampler::draw(){
    double p = model_->zero_probability();
    double pbinomial = p;
    double ppoisson = (1-p) * dpois(0, model_->lambda());
    double nc = pbinomial + ppoisson;
    pbinomial /= nc;

    int nzero = lround(model_->suf()->number_of_zeros());

    double nzero_binomial = rbinom_mt(rng(), nzero, pbinomial);
    double nzero_poission = nzero - nzero_binomial;

    p = rbeta_mt(rng(),
                 zero_probability_prior_->a() + nzero_binomial,
                 zero_probability_prior_->b() + nzero - nzero_binomial +
                 model_->suf()->number_of_positives());
    model_->set_zero_probability(p);

    double a = lambda_prior_->alpha() + model_->suf()->sum_of_positives();
    double b = lambda_prior_->beta() + model_->suf()->number_of_positives();
    b += nzero_poission;
    double lambda = rgamma_mt(rng(), a, b);
    model_->set_lambda(lambda);
  }

  double ZeroInflatedPoissonSampler::logpri()const{
    double ans = zero_probability_prior_->logp(model_->zero_probability());
    ans += lambda_prior_->logp(model_->lambda());
    return ans;
  }

}
