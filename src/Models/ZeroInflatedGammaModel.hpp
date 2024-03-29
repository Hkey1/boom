/*
  Copyright (C) 2005-2013 Steven L. Scott

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

#ifndef BOOM_ZERO_INFLATED_GAMMA_MODEL_HPP_
#define BOOM_ZERO_INFLATED_GAMMA_MODEL_HPP_

#include <Models/Policies/CompositeParamPolicy.hpp>
#include <Models/Policies/ConjugatePriorPolicy.hpp>
#include <Models/DoubleModel.hpp>
#include <Models/BinomialModel.hpp>
#include <Models/GammaModel.hpp>

namespace BOOM{

  // The ZeroInflatedGammaModel describes non-negative data that can
  // be exactly zero, but are positive otherwise.  The model is
  //
  // y ~ (1 - p) * I(y = 0) + p * Gamma(y | mu, a)
  //
  // The mean of this distribution is p * mu. The variance is p * mu^2
  // * (1 - p + (1/a)).
  //
  // The Gamma distribution used here is parameterized as Ga(mu, a),
  // instead of the arguably more conventional Ga(a, b).  The mapping
  // between the two parameterizations is mu = a/b and a = a.
  class ZeroInflatedGammaModel
      : public CompositeParamPolicy,
        public PriorPolicy,
        public DoubleModel
  {
   public:
    ZeroInflatedGammaModel();
    ZeroInflatedGammaModel(Ptr<BinomialModel> positive_probability,
                           Ptr<GammaModel> positive_density);
    ZeroInflatedGammaModel(int number_of_zeros,
                           int number_of_positives,
                           double sum_of_positives,
                           double sum_of_logs_of_positives);
    ZeroInflatedGammaModel(const ZeroInflatedGammaModel &rhs);
    virtual ZeroInflatedGammaModel * clone()const;

    virtual double pdf(Ptr<Data>, bool logscale)const;
    virtual double pdf(const Data *, bool logscale)const;
    virtual double logp(double x)const;
    virtual double sim()const;

    // This model does not keep copies of the original data set.  It
    // uses the sufficient statistics of its component models instead.
    virtual void add_data(Ptr<Data>);
    void add_data_raw(double y);
    virtual void add_mixture_data(Ptr<Data>, double weight);
    void add_mixture_data_raw(double y, double weight);
    virtual void clear_data();

    // Combine the data owned by rhs to the data owned by *this.
    // Throws an exception if rhs is not a ZeroInflatedGammaModel.
    virtual void combine_data(const Model &rhs, bool just_suf = true);

    virtual void mle();

    // The probability that an event is greater than zero.
    double positive_probability()const;
    void set_positive_probability(double prob);

    // Mean of the positive part (i.e. the gamma part) of the
    // distribution.
    double mean_parameter()const;
    void set_mean_parameter(double mu);

    // Shape parameter of the positive part (i.e. the gamma part) of
    // the distribution.
    double shape_parameter()const;
    void set_shape_parameter(double a);

    // Scale parameter of the positive part (i.e. the gamma part) of
    // the distribution.  This is shape_parameter() / mean_parameter().
    double scale_parameter()const;

    // Moments of the actual random variables produced by the model,
    // including both the gamma part and the zero part.
    double mean()const;
    double variance()const;
    double sd()const;

    Ptr<GammaModel> Gamma_model();
    Ptr<BinomialModel> Binomial_model();

   private:
    // The GammaModel describes the distribution of positive outcomes.
    Ptr<GammaModel> gamma_;

    // The BinomialModel describes the probability of a positive
    // outcome.
    Ptr<BinomialModel> binomial_;

    // The zero_threshold_ is a real number below which a value is
    // assumed to be zero.
    double zero_threshold_;

    // These have to be mutable because of logical constness.  The
    // flag log_probabilities_are_current_ is set to false whenever
    // the binomial parameter is changed.  When the log probabilities
    // are needed, check_log_probabilities() will ensure they are set
    // to the new values, and the flag is set to true.
    mutable double log_probability_of_positive_;
    mutable double log_probability_of_zero_;
    mutable bool log_probabilities_are_current_;

    Ptr<DoubleData> DAT(Ptr<Data>)const;

    boost::function<void(void)> create_binomial_observer();
    // An 'observer' that will be called whenever the binomial
    // probability parameter is set.
    void observe_binomial_probability();

    void check_log_probabilities()const;

    // To be called by all constructors after gamma_ and binomial_
    // have been created.  Registers the models with the ParamPolicy
    // and adds the observer to the binomial parameter.
    void setup();
  };

}

#endif // BOOM_ZERO_INFLATED_GAMMA_MODEL_HPP_
