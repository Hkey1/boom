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

#ifndef BOOM_BREG_VS_SAMPLER_HPP
#define BOOM_BREG_VS_SAMPLER_HPP
#include <Models/Glm/RegressionModel.hpp>
#include <Models/PosteriorSamplers/PosteriorSampler.hpp>
#include <Models/MvnGivenScalarSigma.hpp>
#include <Models/Glm/VariableSelectionPrior.hpp>
#include <Models/MvnGivenSigma.hpp>
#include <Models/GammaModel.hpp>

namespace BOOM{
  // prior:  beta | gamma, sigma ~ Normal(b, sigma^2 * Omega)
  //                   1/sigma^2 ~ Gamma(sigma.guess, df)
  //                       gamma ~ VsPrior (product of Bernoulli)
  //
  // A good choice for Omega^{-1} is kappa * XTX/n, which is kappa
  // 'typical' observations.
  //
  // Note that with this prior it is possible for a really poor guess
  // at the prior mean 'b' to inflate the "sum of squares" statistic
  // used to draw the variance.  A reasonable value for b is to set
  // the intercept to the sample mean of the responses, and set the
  // slopes to zero.
  class BregVsSampler : public PosteriorSampler{
   public:

    // Omega inverse is 'prior_nobs' * XTX/n. The intercept term in
    // 'b' is ybar (sample mean of the responses).  The slope terms in
    // b are all zero.  The prior for 1/sigsq is
    // Gamma(prior_nobs/2, prior_ss/2), with
    // prior_ss = prior_nobs*sigma_guess^2, and
    // sigma_guess = sample_variance*(1-expected_rsq)
    BregVsSampler(RegressionModel *,
                  double prior_nobs,          // Omega is prior_nobs * XTX/n
                  double expected_rsq,        // sigsq_guess = sample var*this
                  double expected_model_size, // prior inc probs = this/dim
                  bool first_term_is_intercept = true);

    // Omega inverse is kappa*[(1-alpha)*XTX/n + alpha*(XTX/n)].
    // kappa is 'prior_beta_nobs', and alpha is 'diagonal_shrinkage'.
    // The prior on 1/sigsq is Gamma(prior_sigma_nobs/2, priors_ss/2)
    // with prior_ss = prior_sigma_guess^2 * prior_sigma_nobs.
    // b = [ybar, 0, 0, ...]
    BregVsSampler(RegressionModel *,
                  double prior_sigma_nobs,
                  double prior_sigma_guess,
                  double prior_beta_nobs,
                  double diagonal_shrinkage,
                  double prior_inclusion_probability,
                  bool force_intercept = true);

    // Use this constructor if you want full control over the
    // parameters of the prior distribution, but you don't want to
    // supply actual model objects.  You won't be able to modify the
    // values of the prior parameters afterwards.
    BregVsSampler(RegressionModel *,
                  const Vec & b,
                  const Spd & Omega_inverse,
                  double sigma_guess,
                  double df,
                  const Vec &prior_inclusion_probs);

    // This constructor offers full control.  If external copies of
    // the pointers supplied to the constructor are kept then the
    // values of the prior parameters can be modified.  This would be
    // useful in a hierarchical model, for example.
    BregVsSampler(RegressionModel *m,
                  Ptr<MvnGivenScalarSigma> bpri,
                  Ptr<GammaModel> sinv_pri,
                  Ptr<VariableSelectionPrior> vpri);

    virtual void draw();
    virtual double logpri()const;
    double log_model_prob(const Selector &inc)const;

    // Model selection can be turned on and off altogether, or if very
    // large sets of predictors are being considered then the number
    // of exploration steps can be limited to a specified number.
    void supress_model_selection();
    void allow_model_selection();
    void limit_model_selection(uint nflips);

    // For testing purposes, the draw of beta and/or sigma can be
    // supressed.  This is also useful in cases where sigma is known.
    void supress_beta_draw();
    void supress_sigma_draw();
    void allow_sigma_draw();
    void allow_beta_draw();

    double prior_df()const;
    double prior_ss()const;

    bool model_is_empty()const;
   private:
    // The model whose paramaters are to be drawn.
    RegressionModel * m_;

    // A conditionally (given sigma) Gaussian prior distribution for
    // the coefficients of the full model (with all variables included).
    Ptr<MvnGivenScalarSigma> bpri_;

    // A marginal prior distribution for 1/sigma^2.
    Ptr<GammaModel> spri_;

    // A marginal prior for the set of 0's and 1's indicating which
    // variables are in/out of the model.
    Ptr<VariableSelectionPrior> vpri_;

    std::vector<uint> indx;
    uint max_nflips_;
    bool draw_beta_;
    bool draw_sigma_;

    mutable Vec beta_tilde_;      // this is work space for computing
    mutable Spd iV_tilde_;        // posterior model probs
    mutable double DF_, SS_;

    double set_reg_post_params(const Selector &g, bool do_ldoi)const;
    double mcmc_one_flip(Selector &g, uint which_var, double logp_of_g);

    void draw_beta();
    void draw_model_indicators();
    void draw_sigma();
  };
}
#endif// BOOM_BREG_VS_SAMPLER_HPP
