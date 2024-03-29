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
#include <Models/Glm/PosteriorSamplers/MvtRegSampler.hpp>
#include <Models/GammaModel.hpp>
#include <distributions.hpp>
#include <LinAlg/Cholesky.hpp>
#include <Samplers/SliceSampler.hpp>
#include <TargetFun/Loglike.hpp>
//#include <TargetFun/ScalarLogpostTF.hpp>

namespace BOOM{

  typedef MvtRegSampler MVTRS;

  struct Logp_nu{
    Logp_nu( Ptr<ScaledChisqModel> Numod, Ptr<DoubleModel> Pri)
      : loglike(Numod.get() ), pri(Pri) {}
    double operator()(const Vec &x)const{ return loglike(x) + pri->logp(x[0]); }
    LoglikeTF loglike;
    Ptr<DoubleModel> pri;
  };

  MVTRS::MvtRegSampler(MvtRegModel *m, const Mat &B_guess, double prior_nobs,
		       double prior_df, const Spd & Sigma_guess,
		       Ptr<DoubleModel> Nu_prior)
    : mod(m),
      reg_model(new MvReg(mod->Beta(), mod->Sigma())),
      nu_model(new ScaledChisqModel(m->nu())),
      nu_prior(Nu_prior)
  {
    reg_model->set_params(mod->Beta_prm(), mod->Sigma_prm());
    reg_sampler = new MvRegSampler(reg_model.get(), B_guess, prior_nobs, prior_df, Sigma_guess);
    nu_model->set_prm(mod->Nu_prm());
    Logp_nu nu_logpost(nu_model, nu_prior);
    nu_sampler = new SliceSampler(nu_logpost, true);
  }

  void MVTRS::draw(){
    clear_suf();
    impute_w();
    draw_Sigma();
    draw_Beta();
    draw_nu();
  }

  double MVTRS::logpri()const{
    double ans = nu_model->logp(mod->nu());
    ans +=  reg_sampler->logpri();
    return ans;
  }

  void MVTRS::clear_suf(){
    reg_model->suf()->clear();
    nu_model->suf()->clear();
  }

  void MVTRS::impute_w(){
    Ptr<NeMvRegSuf> rs = reg_model->suf().dcast<NeMvRegSuf>();
    Ptr<GammaSuf> gs = nu_model->suf();

    const std::vector<Ptr<MvRegData> > & dat( mod->dat() );
    uint n = dat.size();
    double w(0);
    for(uint i=0; i<n; ++i){
      Ptr<MvRegData> dp = dat[i];
      w = impute_w(dp);
      rs->update_raw_data(dp->y(), dp->x(), w);
      gs->update_raw(w);
    }
  }

  double MVTRS::impute_w(Ptr<MvRegData> dp){
    const Vec &y(dp->y());
    const Vec &x(dp->x());
    yhat = mod->predict(x);
    double nu = mod->nu();
    double ss = mod->Siginv().Mdist(y,yhat);
    double w = rgamma( (nu+y.size())/2, (nu+ss)/2);
    return w;
  }

  void MVTRS::draw_Sigma(){
    reg_sampler->draw_Sigma();
  }

  void MVTRS::draw_Beta(){
    reg_sampler->draw_Beta();
  }

  void MVTRS::draw_nu(){
    Vec nu(1,mod->nu());
    nu = nu_sampler->draw(nu);
    mod->set_nu(nu[0]);
  }
}
