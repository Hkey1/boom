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

#include "MnpBetaGivenSigmaSampler.hpp"
#include <Models/Glm/MultinomialProbitModel.hpp>
#include <distributions.hpp>
#include <Models/ParamTypes.hpp>

namespace BOOM{

  typedef MnpBetaGivenSigmaSampler MBS;
  typedef MultinomialProbitModel MNP;

  MBS::MnpBetaGivenSigmaSampler(MNP *Mod,
                                Ptr<VectorParams> B,
                                Ptr<UnivParams> K)
    : mnp(Mod),
      b(B),
      kappa(K),
      b0_fixed(true)
  {}

  MBS::MnpBetaGivenSigmaSampler(MNP *Mod,
                                const Vec &B,
                                double K)
    : mnp(Mod),
      b(new VectorParams(B)),
      kappa(new UnivParams(K)),
      b0_fixed(true)
  {}

  void MBS::draw(){
    double n = mnp->n();
    double k = kappa->value();
    const Spd & xtx(mnp->xtx());
    Spd ivar = (xtx)*(1+k/n);
    const Vec &B(b->value());
    Vec mean = mnp->xty() + (xtx*B)*(k/n);
    mean = ivar.solve(mean);
    Vec beta = rmvn_ivar(mean, ivar);
    if(b0_fixed){
      uint start = 0;
      uint p = mnp->subject_nvars();
      Vec b0(beta.begin(), beta.begin()+p);
      for(uint i=0; i<mnp->Nchoices(); ++i){
	int stop = start+p;
	if(stop>0) --stop;
	subvector(beta, start, stop) -= b0;
	start+=p;}}
    mnp->set_beta(beta);
  }

  double MBS::logpri()const{
    double n = mnp->n();
    double k = kappa->value();
    Spd ivar = mnp->xtx()*k/n;
    const Vec &B(b->value());
    return dmvn(mnp->beta(), B, ivar, ivar.logdet(), true);
  }
  void MBS::fix_beta0(bool yn){b0_fixed=yn;}
}
