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

#ifndef BOOM_HMM_HPP
#define BOOM_HMM_HPP

#include <BOOM.hpp>
#include <vector>
#include <Models/Policies/CompositeParamPolicy.hpp>
#include <Models/TimeSeries/TimeSeriesDataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/DataTypes.hpp>

namespace BOOM{

class MarkovModel;
class EmMixtureComponent;

class HmmFilter;
class HmmEmFilter;
class HmmDataImputer;

class HiddenMarkovModel :
      public TimeSeriesDataPolicy<Data>,
      public CompositeParamPolicy,
      public PriorPolicy,
      public LoglikeModel
{
 public:
  // constructors...
  HiddenMarkovModel(std::vector<Ptr<MixtureComponent> > Mix, Ptr<MarkovModel> Mark) ;
  HiddenMarkovModel(const HiddenMarkovModel &rhs);
  virtual HiddenMarkovModel * clone()const;

  template <class Fwd>        // needed for copy constructor
  void set_mixture_components(Fwd b, Fwd e){
    mix_.assign(b,e);
    ParamPolicy::set_models(b,e);
    ParamPolicy::add_model(mark_);
  }

  uint state_space_size() const;
  virtual void initialize_params();
  void set_nthreads(uint);

  double pdf(dPtr dp, bool logscale) const;
  void clear_client_data();

  std::vector<Ptr<MixtureComponent> > mixture_components();
  Ptr<MixtureComponent> mixture_component(uint s);

  // returns loglike as a side effect
  double impute_latent_data();
  uint nthreads()const;

  Ptr<MarkovModel> mark();
  virtual double loglike()const;
  double saved_loglike()const;
  void randomly_assign_data();
  void save_loglike(const string & fname, uint ping=1);
  void save_logpost(const string & fname, uint ping=1);

  // For managing the distribution of hidden states.
  void save_state_probs();
  void clear_prob_hist();
  Mat report_state_probs(const DataSeriesType &ts)const;

  const Vec &pi0() const;
  const Mat &Q() const;
  void set_pi0(const Vec &Pi0);
  void set_Q(const Mat &Q);

  // Options for managing the distribution of the initial state.
  void fix_pi0(const Vec &Pi0);
  void fix_pi0_stationary();
  void fix_pi0_uniform();
  void free_pi0();
  bool pi0_fixed()const;

  virtual uint io_params(IO io_prm);

 protected:
  void set_loglike(double);
  void set_logpost(double);
  void write_loglike(double);  // deprecated
  void write_logpost(double);  // deprecated
  void set_filter(Ptr<HmmFilter> f);
 private:
  Ptr<MarkovModel> mark_;
  std::vector<Ptr<MixtureComponent> > mix_;
  Ptr<HmmFilter> filter_;
  std::map<Ptr<Data>, Vec > prob_hist_;
  Ptr<UnivParams> loglike_;
  Ptr<UnivParams> logpost_;
  std::vector<Ptr<HmmDataImputer> > workers_;

  double impute_latent_data_with_threads();
};
//----------------------------------------------------------------------


class HMM_EM : public HiddenMarkovModel
{
 public:
  typedef EmMixtureComponent EMC;
  HMM_EM(std::vector<Ptr<EMC> >Mix, Ptr<MarkovModel> Mark);
  HMM_EM(const HMM_EM &rhs);
  HMM_EM * clone()const;

  virtual void initialize_params();
  virtual void mle();
  double Estep(bool bayes=false);
  void Mstep(bool bayes=false);
  void find_posterior_mode();
  void map();    // throw an exception if any of the mixture
  // components do not have a conjugate prior set
  void trace(bool=true);
  void mle_trace();
  void set_epsilon(double);
 private:
  void find_mode(bool bayes=false, bool save_history=false);
  std::vector<Ptr<MixtureComponent> > tomod(const std::vector<Ptr<EMC> > &v)const;

  std::vector<Ptr<EMC> > mix_;
  Ptr<HmmEmFilter> filter_;
  double eps;
  bool trace_;
};
}
#endif  // BOOM_HMM_HPP
