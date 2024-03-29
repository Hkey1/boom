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
#ifndef BOOM_MVT_REG_HPP
#define BOOM_MVT_REG_HPP
#include <Models/Policies/ParamPolicy_3.hpp>
#include <Models/Policies/IID_DataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/SpdParams.hpp>
#include <cpputil/Ptr.hpp>
#include <Models/Glm/Glm.hpp>  // for MvRegData

namespace BOOM{
  class MvtRegModel
    : public ParamPolicy_3<MatrixParams,SpdParams,UnivParams>,
      public IID_DataPolicy<MvRegData>,
      public PriorPolicy,
      public LoglikeModel
  {
  public:
    MvtRegModel(uint xdim, uint ydim);
    MvtRegModel(const Mat &X,const Mat &Y, bool add_intercept=false);
    MvtRegModel(const Mat &B, const Spd &Sigma, double nu);

    MvtRegModel(const MvtRegModel &rhs);
    MvtRegModel * clone()const;

    uint xdim()const;  // x includes intercept
    uint ydim()const;

    const Mat & Beta()const;     // [xdim rows, ydim columns]
    const Spd & Sigma()const;
    const Spd & Siginv()const;
    double ldsi()const;
    double nu()const;

    Ptr<MatrixParams> Beta_prm();
    Ptr<SpdParams> Sigma_prm();
    Ptr<UnivParams> Nu_prm();
    const Ptr<MatrixParams> Beta_prm()const;
    const Ptr<SpdParams> Sigma_prm()const;
    const Ptr<UnivParams> Nu_prm()const;

    void set_Beta(const Mat &B);
    void set_beta(const Vec & b, uint m);
    void set_Sigma(const Spd &V);
    void set_Siginv(const Spd &iV);

    void set_nu(double nu);

    //--- estimation and probability calculations
    virtual void mle();
    virtual double loglike()const;
    virtual double pdf(dPtr,bool)const;
    virtual Vec predict(const Vec &x)const;

    //---- simulate MV regression data ---
    virtual MvRegData * simdat()const;
    virtual MvRegData * simdat(const Vec &X)const;
    Vec simulate_fake_x()const;  // no intercept

  };
}

#endif //BOOM_MVT_REG_HPP
