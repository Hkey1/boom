/*
  Copyright (C) 2005 Steven L. Scott

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

#include "RegressionModel.hpp"
#include <stats/Design.hpp>
#include <LinAlg/Types.hpp>
#include <Models/PosteriorSamplers/PosteriorSampler.hpp>
#include <Models/Glm/PosteriorSamplers/RegressionConjSampler.hpp>
#include <Models/GammaModel.hpp>
#include <Models/Glm/MvnGivenXandSigma.hpp>
#include <sstream>
#include <cmath>
#include <distributions.hpp>
#include <cpputil/ThrowException.hpp>
#include <Models/SufstatAbstractCombineImpl.hpp>

namespace BOOM{
  inline void incompatible_X_and_y(const Mat &X, const Vec &y){
    ostringstream out;
    out << "incompatible X and Y" << endl
	<< "X = " << endl << X << endl
	<< "Y = " << endl << y << endl;
    throw_exception<std::runtime_error>(out.str());
  };

  inline void index_out_of_bounds(uint i, uint bound){
    ostringstream out;
    out << "requested index " << i << " out of bounds." << endl
	<< "bound is " << bound << "."<< endl;
    throw_exception<std::runtime_error>(out.str());
  };

  Mat add_intercept(const Mat &X){
    Vec one(X.nrow(), 1.0);
    return cbind(one, X); }

  Vec add_intercept(const Vec &x){ return concat(1.0, x); }

  //============================================================
//   RegressionData::RegressionData(double Y, const Vec &X):
//     GlmData(X),
//     y_(Y)
//   {}

//   RegressionData::RegressionData(const RegressionData &rhs)
//     : Data(rhs),
//       GlmData(rhs),
//       y_(rhs.y_)
//   {}

//   RegressionData * RegressionData::clone()const{
//     return new RegressionData(*this);}

//   ostream & RegressionData::display(ostream &out)const{
//     out << y() << " " << x() << " ";
//     return out;}

//   istream & RegressionData::read(istream &in){
//     in >> y_;
//     GlmData::read(in);
//     return in;}

//   RegressionData & RegressionData::operator=(const RegressionData &rhs){
//     if(&rhs==this) return *this;
//     y_ = rhs.y_;
//     GlmData::operator=(rhs);
//     return *this;
//   }

  //============================================================
  anova_table RegSuf::anova()const{
    anova_table ans;
    double nobs = n();
    double p = size();  // p+1 really

    ans.SSE = SSE();
    ans.SST = SST();
    ans.SSM = ans.SST - ans.SSE;
    ans.dft = nobs-1;
    ans.dfe = nobs - p;
    ans.dfm = p-1;
    ans.MSE = ans.SSE/ ans.dfe;
    ans.MSM = ans.SSM/ans.dfm;
    ans.F = ans.MSM/ans.MSE;
    ans.p_value = pf(ans.F, ans.dfm, ans.dfe, false, false);
    return ans;
  }

  ostream & anova_table::display(ostream &out)const{
    out << "ANOVA Table:" << endl
 	<< "\tdf\tSum Sq.\t\tMean Sq.\tF:  " << F << endl
 	<< "Model\t" <<dfm << "\t" <<SSM <<"\t\t" <<MSM<<  endl
 	<< "Error\t" << dfe << "\t"<< SSE <<"\t\t" <<MSE<<"\t p-value: "
 	<< p_value << endl
 	<< "Total\t" << dft <<"\t"<<SST <<endl;
    return out;
  }
  ostream & operator<<(ostream &out, const anova_table &tab){
    tab.display(out);
    return out;}
  //======================================================================
  ostream & RegSuf::print(ostream &out)const{
    out << "sample size: " << n() << endl
        << "xty: " << xty() << endl
        << "xtx: " << endl << xtx();
    return out;
  }
  //======================================================================
  QrRegSuf::QrRegSuf(const Mat&X, const Vec &y, bool add_icpt):
    qr(add_icpt ? add_intercept(X) : X),
    Qty(),
    sumsqy(0.0),
    current(true)
  {
    Mat Q(qr.getQ());
    Qty = y*Q;
    sumsqy = y.dot(y);
  }

  QrRegSuf::QrRegSuf(const QrRegSuf &rhs)
    : Sufstat(rhs),
      RegSuf(rhs),
      SufstatDetails<DataType>(rhs),
      qr(rhs.qr),
      Qty(rhs.Qty),
      sumsqy(rhs.sumsqy),
      current(rhs.current)
  {}



  uint QrRegSuf::size()const{   // dimension of beta
    //    if(!current) refresh_qr();
    return Qty.size();}

  Spd QrRegSuf::xtx()const{
    //    if(!current) refresh_qr();
    return RTR(qr.getR());}

  Vec QrRegSuf::xty()const{
    //    if(!current) refresh_qr();
    return Qty*qr.getR(); }

  Spd QrRegSuf::xtx(const Selector &inc)const{
    //    if(!current) refresh_qr();
    return RTR(inc.select_square(qr.getR()));
  }

  Vec QrRegSuf::xty(const Selector &inc)const{
    //    if(!current) refresh_qr();
    return inc.select(Qty)*inc.select_square(qr.getR()); }


  double QrRegSuf::yty()const{return sumsqy;}
  void QrRegSuf::clear(){
    sumsqy=0;
    Qty=0;
    qr.clear();
  }

  QrRegSuf * QrRegSuf::clone()const{
    return new QrRegSuf(*this);}


  Vec QrRegSuf::beta_hat()const{
    //if(!current) refresh_qr();
    return qr.Rsolve(Qty); }

  Vec QrRegSuf::beta_hat(const Vec &y)const{
    //    if(!current) refresh_qr();
    return qr.solve(y);}

  void QrRegSuf::Update(const DataType &dp){
    current=false;
    Ptr<DataType> d= dp.clone();
  }  // QR not built for updating

  void QrRegSuf::add_mixture_data(double , const Vec &, double){
    report_error("use NeRegSuf for regression model mixture components.");
  }

  void QrRegSuf::add_mixture_data(double , const ConstVectorView &, double){
    report_error("use NeRegSuf for regression model mixture components.");
  }

  void QrRegSuf::refresh_qr(const std::vector<Ptr<RegressionData> > &raw_data) const {
    if(current) return;
    int n = raw_data.size();  // number of observations
    if(n==0){
      current=false;
      return;}

    Ptr<RegressionData> rdp = DAT(raw_data[0]);
    uint dim_beta = rdp->size();
    Mat X(n, dim_beta);
    Vec y(n);
    sumsqy=0.0;
    for(int i = 0; i<n; ++i){
      rdp = DAT(raw_data[i]);
      y[i] = rdp->y();
      const Vec & x(rdp->x());
      X.set_row(i,x);
//       X(i,0)=1.0;    // this stuff is no longer needed b/c the intercept is explicit
//       int k=0;
//       for(int j=x.lo(); j<=x.hi(); ++j) X(i,++k) = x[j];
      sumsqy += y[i]*y[i];
    }
    qr.decompose(X);
    X = qr.getQ();
    Qty = y*X;
    current=true;
  }

  double QrRegSuf::SSE()const{
    //    if(!current) refresh_qr();
    return sumsqy - Qty.dot(Qty); }

  double QrRegSuf::ybar()const{
    //    if(!current) refresh_qr();
    return qr.getR()(0,0)*Qty[0]/n(); }

  double QrRegSuf::SST()const{
    //    if(!current) refresh_qr();
    return sumsqy - n()*pow(ybar(),2); }

  double QrRegSuf::n()const{
    //    if(!current) refresh_qr();
    return qr.nrow();}

  void QrRegSuf::combine(Ptr<RegSuf>){
    throw_exception<std::runtime_error>("cannot combine QrRegSuf");
  }

  void QrRegSuf::combine(const RegSuf &){
    throw_exception<std::runtime_error>("cannot combine QrRegSuf");
  }

  QrRegSuf * QrRegSuf::abstract_combine(Sufstat *s){
    return abstract_combine_impl(this,s); }

  Vec QrRegSuf::vectorize(bool)const{
    throw_exception<std::runtime_error>("cannot combine QrRegSuf");
    return Vec(1, 0.0);
  }

  Vec::const_iterator QrRegSuf::unvectorize(Vec::const_iterator &v,
                                  bool){
    throw_exception<std::runtime_error>("cannot combine QrRegSuf");
    return v;
  }

  Vec::const_iterator QrRegSuf::unvectorize(const Vec &v, bool minimal){
    Vec::const_iterator it(v.begin());
    return unvectorize(it, minimal);
  }

  ostream & QrRegSuf::print(ostream &out)const{
    return out << "sumsqy = " << yty() << endl
               << "xty_ = " << xty() << endl
               << "xtx  = " << endl << xtx();
  }
  //---------------------------------------------
  NeRegSuf::NeRegSuf(uint p)
  : xtx_(p),
    needs_to_reflect_(false),
    xty_(p),
    xtx_is_fixed_(false),
    sumsqy(0.0),
    n_(0),
    sumy_(0.0)
  { }

  NeRegSuf::NeRegSuf(const Mat &X, const Vec &y, bool add_icpt)
      : needs_to_reflect_(false),
        xtx_is_fixed_(false),
        sumsqy(y.normsq()),
        n_(nrow(X)),
        sumy_(y.sum())
  {
    Mat tmpx = add_icpt ? add_intercept(X) : X;
    xty_ =y*tmpx;
    xtx_ = tmpx.inner();
    sumsqy = y.dot(y);
  }

  NeRegSuf::NeRegSuf(const Spd & XTX, const Vec & XTY, double YTY, double n)
    : xtx_(XTX),
      needs_to_reflect_(true),
      xty_(XTY),
      xtx_is_fixed_(false),
      sumsqy(YTY),
      n_(n),
      sumy_(XTY[0])
  {}

  NeRegSuf::NeRegSuf(const NeRegSuf &rhs)
    : Sufstat(rhs),
      RegSuf(rhs),
      SufstatDetails<DataType>(rhs),
      xtx_(rhs.xtx_),
      needs_to_reflect_(rhs.needs_to_reflect_),
      xty_(rhs.xty_),
      xtx_is_fixed_(rhs.xtx_is_fixed_),
      sumsqy(rhs.sumsqy),
      n_(rhs.n_),
      sumy_(rhs.sumy_)
  {}

  NeRegSuf * NeRegSuf::clone()const{
    return new NeRegSuf(*this);}

  void NeRegSuf::add_mixture_data(double y, const Vec &x, double prob){
    if(!xtx_is_fixed_) {
      xtx_.add_outer(x, prob, false);
      needs_to_reflect_ = true;
    }
    xty_.axpy(x, y * prob);
    sumsqy+= y * y * prob;
    n_ += prob;
    sumy_ += y * prob;
  }

  void NeRegSuf::add_mixture_data(double y, const ConstVectorView &x, double prob){
    if(!xtx_is_fixed_) {
      xtx_.add_outer(x, prob, false);
      needs_to_reflect_ = true;
    }
    xty_.axpy(x, y * prob);
    sumsqy+= y * y * prob;
    n_ += prob;
    sumy_ += y * prob;
  }

  void NeRegSuf::clear(){
    if(!xtx_is_fixed_) xtx_=0.0;
    xty_=0.0;
    sumsqy=0.0;
    n_ = 0;
    sumy_ = 0.0;
  }

  void NeRegSuf::Update(const RegressionData &rdp){
    ++n_;
    int p = rdp.size();
    if(xtx_.nrow()==0 || xtx_.ncol()==0)
      xtx_ = Spd(p,0.0);
    if(xty_.size()==0) xty_ = Vec(p, 0.0);
    const Vec & tmpx(rdp.x());  // add_intercept(rdp.x());
    double y = rdp.y();
    xty_.axpy(tmpx, y);
    if(!xtx_is_fixed_) {
      xtx_.add_outer(tmpx, 1.0, false);
      needs_to_reflect_ = true;
    }
    sumsqy+= y*y;
    sumy_ += y;
  }

  uint NeRegSuf::size()const{ return xtx_.ncol();}  // dim(beta)
  Spd NeRegSuf::xtx()const{
    reflect();
    return xtx_;
  }
  Vec NeRegSuf::xty()const{ return xty_;}

  Spd NeRegSuf::xtx(const Selector &inc)const{
    reflect();
    return inc.select(xtx_);
  }
  Vec NeRegSuf::xty(const Selector &inc)const{
    return inc.select(xty_);}
  double NeRegSuf::yty()const{ return sumsqy;}

  Vec NeRegSuf::beta_hat()const{
    reflect();
    return xtx_.solve(xty_);
  }

  double NeRegSuf::SSE()const{
    Spd ivar = xtx().inv();
    return yty() - ivar.Mdist(xty()); }
  double NeRegSuf::SST()const{ return sumsqy - n()*pow(ybar(),2); }
  double NeRegSuf::n()const{ return n_; }
  double NeRegSuf::ybar()const{ return sumy_/n_;}

  void NeRegSuf::combine(Ptr<RegSuf> sp){
    Ptr<NeRegSuf> s(sp.dcast<NeRegSuf>());
    xtx_ += s->xtx_;   // Do we want to combine xtx_ if xtx_is_fixed_?
    needs_to_reflect_ = needs_to_reflect_ || s->needs_to_reflect_;
    xty_ += s->xty_;
    sumsqy += s->sumsqy;
    sumy_ += s->sumy_;
    n_ += s->n_;
  }

  void NeRegSuf::combine(const RegSuf & sp){
    const NeRegSuf& s(dynamic_cast<const NeRegSuf &>(sp));
    xtx_ += s.xtx_;   // Do we want to combine xtx_ if xtx_is_fixed_?
    needs_to_reflect_ = needs_to_reflect_ || s.needs_to_reflect_;
    xty_ += s.xty_;
    sumsqy += s.sumsqy;
    sumy_ += s.sumy_;
    n_ += s.n_;
  }

  NeRegSuf * NeRegSuf::abstract_combine(Sufstat *s){
    return abstract_combine_impl(this,s); }

  Vec NeRegSuf::vectorize(bool minimal)const{
    reflect();
    Vec ans = xtx_.vectorize(minimal);
    ans.concat(xty_);
    ans.push_back(sumsqy);
    ans.push_back(n_);
    ans.push_back(sumy_);
    return ans;
  }

  Vec::const_iterator NeRegSuf::unvectorize(Vec::const_iterator &v,
                                  bool minimal){
    // do we want to store xtx_is_fixed_?
    xtx_.unvectorize(v, minimal);
    needs_to_reflect_ = true;
    uint dim = xty_.size();
    xty_.assign(v, v+dim);
    v+=dim;
    sumsqy = *v;  ++v;
    n_ = lround(*v); ++v;
    sumy_ = *v; ++v;
    return v;
  }

  Vec::const_iterator NeRegSuf::unvectorize(const Vec &v, bool minimal){
    // do we want to store xtx_is_fixed_?
    Vec::const_iterator it = v.begin();
    return unvectorize(it, minimal);
  }

  ostream & NeRegSuf::print(ostream &out)const{
    reflect();
    return out << "sumsqy = " << sumsqy << endl
               << "sumy_  = " << sumy_ << endl
               << "n_     = " << n_ << endl
               << "xty_ = " << xty_ << endl
               << "xtx  = " << endl << xtx_;
  }

  void NeRegSuf::fix_xtx(bool fix){
    reflect();
    xtx_is_fixed_ = fix;
  }

  void NeRegSuf::reflect()const{
    if(needs_to_reflect_){
      xtx_.reflect();
      needs_to_reflect_ = false;
    }
  }

  //======================================================================
  typedef RegressionDataPolicy RDP;

  RDP::RegressionDataPolicy(Ptr<RegSuf> s)
    : DPBase(s)
  {}
  RDP::RegressionDataPolicy(Ptr<RegSuf>s, const DatasetType &d)
    : DPBase(s,d)
  {}

  RDP::RegressionDataPolicy(const RegressionDataPolicy &rhs)
    : Model(rhs),
      DPBase(rhs)
  {}

  RegressionDataPolicy & RDP::operator=(const RegressionDataPolicy &rhs){
    if(&rhs!=this) DPBase::operator=(rhs);
    return *this;
  }


  //======================================================================
  typedef RegressionModel RM;

  RM::RegressionModel(uint p)
    : GlmModel(),
      ParamPolicy(new GlmCoefs(p), new UnivParams(1.0)),
      DataPolicy(new NeRegSuf(p)),
      ConjPriorPolicy()
  {}

  RM::RegressionModel(const Vec &b, double Sigma)
    : GlmModel(),
      ParamPolicy(new GlmCoefs(b), new UnivParams(Sigma*Sigma)),
      DataPolicy(new NeRegSuf(b.size())),
      ConjPriorPolicy()
  {}

  RM::RegressionModel(const Mat &X, const Vec &y, bool add_icpt)
    : GlmModel(),
      ParamPolicy(new GlmCoefs(X.ncol()), new UnivParams(1.0)),
      DataPolicy(new QrRegSuf(X,y, add_icpt)),
      ConjPriorPolicy()
  {
    mle();
  }

  RM::RegressionModel(const DesignMatrix &X, const Vec &y, bool add_icpt)
    : GlmModel(),
      ParamPolicy(new GlmCoefs(X.ncol()), new UnivParams(1.0)),
      DataPolicy(new QrRegSuf(X,y, add_icpt)),
      ConjPriorPolicy()
  {}

  RM::RegressionModel(const DatasetType &d, bool all)
    : GlmModel(),
      ParamPolicy(new GlmCoefs(d[0]->size(), all), new UnivParams(1.0)),
      DataPolicy(new NeRegSuf(d.begin(), d.end())),
      ConjPriorPolicy()
  {}

  RM::RegressionModel(const RegressionModel &rhs)
    : Model(rhs),
      MLE_Model(rhs),
      GlmModel(rhs),
      ParamPolicy(rhs),
      DataPolicy(rhs),
      ConjPriorPolicy(rhs),
      NumOptModel(rhs),
      EmMixtureComponent(rhs)
  {}

  RM * RM::clone()const{return new RegressionModel(*this); }

  uint RM::nvars()const{ return coef().nvars(); }
  uint RM::nvars_possible()const{ return coef().nvars_possible(); }

  Spd RM::xtx(const Selector &inc)const{ return suf()->xtx(inc);}
  Vec RM::xty(const Selector &inc)const{ return suf()->xty(inc);}

  Spd RM::xtx()const{ return xtx( coef().inc() ) ;}
  Vec RM::xty()const{ return xty( coef().inc() ) ;}
  double RM::yty()const{ return suf()->yty();  }

  Vec RM::simulate_fake_x()const{
    uint p = nvars_possible();
    Vec x(p-1);
    for(uint i=0; i<p-1; ++i) x[i] = rnorm();
    return x;
  }

  RegressionData * RM::simdat()const{
    Vec x = simulate_fake_x();
    double yhat = predict(x);
    double y = rnorm(yhat, sigma());
    return new RegressionData(y,x);
  }

  RegressionData * RM::simdat(const Vec &X)const{
    double yhat = predict(X);
    double y = rnorm(yhat, sigma());
    return new RegressionData(y,X);
  }

  //======================================================================
  GlmCoefs & RM::coef(){return ParamPolicy::prm1_ref();}
  const GlmCoefs & RM::coef()const{return ParamPolicy::prm1_ref();}
  Ptr<GlmCoefs> RM::coef_prm(){return ParamPolicy::prm1();}
  const Ptr<GlmCoefs> RM::coef_prm()const{return ParamPolicy::prm1();}
  void RM::set_sigsq(double s2){ParamPolicy::prm2_ref().set(s2);}

  Ptr<UnivParams> RM::Sigsq_prm(){return ParamPolicy::prm2();}
  const Ptr<UnivParams> RM::Sigsq_prm()const {return ParamPolicy::prm2();}

  double RM::sigsq()const{return ParamPolicy::prm2_ref().value();}
  double RM::sigma()const{return sqrt(sigsq());}

  void RM::make_X_y(Mat &X, Vec &Y)const{
    uint p = xdim();
    uint n = dat().size();
    X = Mat(n,p);
    Y = Vec(n);
    for(uint i=0; i<n; ++i){
      Ptr<RegressionData> rdp = dat()[i];
      const Vec &x(rdp->x());
      assert(x.size()==p);
      X.set_row(i,x);
      Y[i] = rdp->y();
    }
  }

  void RM::mle(){
    set_beta(suf()->beta_hat());
    set_sigsq(suf()->SSE()/suf()->n());
  }

  double RM::pdf(dPtr dp, bool logscale)const{
    Ptr<RegressionData> rd = DAT(dp);
    const Vec &x = rd->x();
    return dnorm(rd->y(), predict(x), sigma(), logscale);
  }

  double RM::pdf(const Data *dp, bool logscale)const{
    const RegressionData *rd =  dynamic_cast<const RegressionData *>(dp);
    return dnorm(rd->y(), predict(rd->x()), sigma(), logscale);
  }

  double RM::Loglike(Vec &g, Mat &h, uint nd)const{
    const double log2pi = 1.83787706640935;
    const Vec b = this->beta();
    const double sigsq = this->sigsq();
    double n = suf()->n();
    if(b.size()==0) return empty_loglike(g, h, nd);

    double SSE = yty() - 2*b.dot(xty()) + xtx().Mdist(b);
    double ans =  -.5*(n * log2pi  + n *log(sigsq)+ SSE/sigsq);

    if(nd>0){  // sigsq derivs come first in CP2 vectorization
      Spd xtx = this->xtx();
      Vec gbeta = (xty() - xtx*b)/sigsq;
      double sig4 = sigsq*sigsq;
      double gsigsq = -n/(2*sigsq) + SSE/(2*sig4);
      g = concat(gsigsq, gbeta);
      if(nd>1){
 	double h11 = .5*n/sig4 - SSE/(sig4*sigsq);
 	h = unpartition(h11, (-1/sigsq)*gbeta, (-1/sigsq)*xtx);}}
    return ans;
  }

  // Log likelihood when beta is empty, so that xbeta = 0.  In this
  // case the only parameter is sigma^2
  double RM::empty_loglike(Vec &g, Mat &h, uint nd)const{
    double v = sigsq();
    double n = suf()->n();
    double ss = suf()->yty();
    const double log2pi = 1.83787706640935;
    double ans = -.5*n*(log2pi + log(v)) - .5*ss/v;
    if(nd > 0){
      double v2 = v*v;
      g[0] = -.5*n/v + .5*ss/v2;
      if(nd > 1){
        h(0,0) = .5*n/v2 - ss/(v2*v);
      }
    }
    return ans;
  }

  void RM::set_conjugate_prior(Ptr<MvnGivenXandSigma> b, Ptr<GammaModel> siginv){
    NEW(RegressionConjSampler, pri)(this, b,siginv);
    this->set_conjugate_prior(pri);
  }

  void RM::set_conjugate_prior(Ptr<RegressionConjSampler> pri){
    ConjPriorPolicy::set_conjugate_prior(pri);
  }

  void RM::add_mixture_data(Ptr<Data> dp, double prob){
    Ptr<RegressionData> d(DAT(dp));
    suf()->add_mixture_data(d->y(), d->x(), prob);
  }

  /*
     SSE = (y-Xb)^t (y-Xb)
     = (y - QQTy)^T (y - Q Q^Ty)
     = ((I=QQ^T)y))^T(I-QQ^T)y)
     = y^T (1-QQ^T)(I-QQ^T)y
     =  y^T ( I - QQ^T - QQ^T +  QQ^TQQ^T)y
     = y^T(I -QQ^T)y

     b = xtx-1xty = (rt qt q r)^{-1} rt qt y
     = r^[-1] qt y
  */

} // ends namespace BOOM
