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
#include <Samplers/MH_Proposals.hpp>
#include <LinAlg/Cholesky.hpp>
#include <distributions.hpp>
namespace BOOM{


  MH_Proposal::MH_Proposal()
      : rng_(seed_rng())
  {}

  typedef MvtMhProposal MVTP;

  MVTP::MvtMhProposal(const Spd & Ivar, double nu)
      : nu_(nu)
  {
    set_ivar(Ivar);
  }

  Vec MVTP::draw(const Vec & old)const{
    int n = old.size();
    Vec ans(n);
    for(int i = 0; i < n; ++i) ans[i] = rnorm_mt(rng(), 0,1);
    ans = chol_ * ans;
    if(finite(nu_) && nu_ > 0){
      double w = rgamma_mt(rng(), nu_/2.0, nu_/2.0);
      ans /= sqrt(w);
    }
    ans += mu(old);
    return ans;
  }

  void MVTP::set_nu(double nu){nu_ = nu;}
  uint MVTP::dim()const{return siginv_.nrow();}

  void MVTP::set_var(const Spd &V){
    Chol L(V);
    chol_ = L.getL();
    siginv_ = L.inv();
    ldsi_ = -2 * sum(log(diag(chol_)));
  }

  void MVTP::set_ivar(const Spd &H){
    Chol cholesky(H);
    siginv_ = H;
    chol_ = cholesky.getL();
    ldsi_ = 2 * sum(log(diag(chol_)));
    chol_ = chol_.t().inv();
    // now chol_ is an upper triangular matrix with chol_ * chol_.t() = Sigma
  }

  double MVTP::logf(const Vec &x, const Vec & old)const{
    if(finite(nu_) && nu_>0){
      return dmvt(x, mu(old), siginv_, nu_, ldsi_, true);
    }
    return dmvn(x, mu(old) ,siginv_, ldsi_, true);
  }


  typedef MvtRwmProposal MVTR;
  MVTR::MvtRwmProposal(const Spd &Ivar, double nu)
      : MVTP(Ivar, nu)
  {}

  typedef MvtIndepProposal MVTI;
  MVTI::MvtIndepProposal(const Vec & mu, const Spd & Ivar, double nu)
      : MVTP(Ivar, nu),
        mu_(mu)
  {}

  void MVTI::set_mu(const Vec & mu){ mu_ = mu; }

  //======================================================================
  MH_ScalarProposal::MH_ScalarProposal()
      : rng_(seed_rng())
  {}

  //======================================================================
  typedef TScalarMhProposal TSP;

  TSP::TScalarMhProposal(double Sd, double Df)
    : sig_(Sd),
      nu_(Df)
  {}

  double TSP::draw(double old)const{
    if(finite(nu_) && nu_ > 0)
      return rstudent_mt(rng(), mu(old), sig_, nu_);
    return rnorm_mt(rng(), mu(old), sig_);
  }

  double TSP::logf(double x, double old)const{
    if(finite(nu_) && nu_ > 0){
      return dstudent(x, mu(old), sig_, nu_, true);
    }
    return dnorm(x, mu(old), sig_, true);
  }

}
