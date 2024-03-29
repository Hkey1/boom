/*
  Copyright (C) 2005-2010 Steven L. Scott

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

#include <Models/PosteriorSamplers/ZeroMeanMvnConjSampler.hpp>
#include <Models/ZeroMeanMvnModel.hpp>
#include <Models/MvnBase.hpp>
#include <distributions.hpp>

namespace BOOM{
  typedef ZeroMeanMvnConjSampler ZMMCS;
  ZMMCS::ZeroMeanMvnConjSampler(ZeroMeanMvnModel *m,
                                Ptr<WishartModel> siginv_prior)
      :m_(m),
       siginv_prior_(siginv_prior)
  {}
  ZMMCS::ZeroMeanMvnConjSampler(ZeroMeanMvnModel *m,
                                double prior_df,
                                double sigma_guess)
      :m_(m),
       siginv_prior_(new WishartModel(m->dim(), prior_df, pow(sigma_guess, 2)))
  {}

  void ZMMCS::draw(){
    Ptr<MvnSuf> s = m_->suf();
    double df = s->n() + siginv_prior_->nu();
    Spd S = s->center_sumsq(m_->mu()) + siginv_prior_->sumsq();
    S = rWish(df, S.inv());
    m_->prm()->set_ivar(S);
  }

  double ZMMCS::logpri()const{
    return siginv_prior_->logp(m_->siginv());
  }

  // posterior mode with respect to Sigma inverse.
  void ZMMCS::find_posterior_mode(){
    Ptr<MvnSuf> s = m_->suf();
    Spd Sumsq = s->center_sumsq(m_->mu()) + siginv_prior_->sumsq();
    double nu = s->n() + siginv_prior_->nu();

    nu = nu - m_->dim() - 1;
    if(nu < 0) {
      report_error("degrees of freedom too small in "
                   "ZeroMeanMvnConjSampler::find_posterior_mode");
    }
    m_->prm()->set_var(Sumsq / nu);
  }

}
