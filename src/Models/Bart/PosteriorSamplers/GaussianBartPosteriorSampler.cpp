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

#include <Models/Bart/PosteriorSamplers/GaussianBartPosteriorSampler.hpp>
#include <distributions.hpp>
#include <cpputil/math_utils.hpp>

namespace BOOM {

  namespace Bart {
    GaussianResidualRegressionData::GaussianResidualRegressionData(
        const Ptr<RegressionData> &dp,
        double original_prediction)
        : ResidualRegressionData(dp->Xptr().get()),
          observed_response_(dp.get()),
          residual_(dp->y() - original_prediction)
    {}

    void GaussianResidualRegressionData::add_to_gaussian_suf(
        GaussianBartSufficientStatistics &suf) const {
      suf.update(*this);
    }

  }  // namespace Bart

  GaussianBartPosteriorSampler::GaussianBartPosteriorSampler(
      GaussianBartModel *model,
      double prior_sigma_guess,
      double prior_sigma_weight,
      double prior_mean_guess,
      double prior_mean_sd,
      double prior_tree_depth_alpha,
      double prior_tree_depth_beta)
      : BartPosteriorSamplerBase(model,
                                 prior_mean_guess,
                                 prior_mean_sd,
                                 prior_tree_depth_alpha,
                                 prior_tree_depth_beta),
        model_(model),
        siginv_prior_(new ChisqModel(prior_sigma_weight,
                                     prior_sigma_guess))
  {}

  void GaussianBartPosteriorSampler::draw() {
    BartPosteriorSamplerBase::draw();
    draw_residual_variance();
  }

  double GaussianBartPosteriorSampler::draw_mean(Bart::TreeNode *leaf) {
    double sigsq = model_->sigsq();
    const Bart::GaussianBartSufficientStatistics *suf(
        dynamic_cast<const Bart::GaussianBartSufficientStatistics *>(
            leaf->compute_suf()));
    double ivar = suf->n() / sigsq + 1.0 / node_mean_prior()->sigsq();
    double mean = (suf->sum() / sigsq)
        + node_mean_prior()->mu() / node_mean_prior()->sigsq();
    mean /= ivar;
    double sd = sqrt(1.0 / ivar);
    double value = rnorm_mt(rng(), mean, sd);
    return value;
  }

  //----------------------------------------------------------------------
  // This is exact and includes all normalizing constants, even those
  // that might cancel in the MH ratio.  If a profiler says we're
  // taking too many logs we can do some of the cancelation by hand.
  //
  // Args:
  //   suf:  Sufficient statistics describing the data at a node.
  //
  // Returns:
  //   The log probability density of the data described by suf.
  //
  //   The log integrated likelihood is derived below:
/*
  \documentclass{article}
  \usepackage{amsmath}
  \begin{document}
  The integrated Gaussian likelihood for a vector of observations $\bf y$ is
  \begin{equation*}
  \begin{split}
  p({\bf y} | \sigma) &=
  \int
  \left(\frac{1}{2\pi}\right)^{n/2}
  \left(\frac{1}{\sigma^2}\right)^{n/2}
  \exp\left( -\frac{1}{2} \frac{(n-1)s^2}{\sigma^2}
  - \frac{1}{2} \frac{n}{\sigma^2} (\mu - \bar y)^2\right)
  \frac{1}{\sqrt{2\pi}}
  \frac{1}{\tau}
  \exp\left( -\frac{1}{2} \frac{(\mu - \mu_0)^2}{\tau^2}\right)
  \ d \mu  \\
  &= \frac{C_1}{\sqrt{2\pi}} \int
  %%%
  %
  \exp\left(
  -\frac{1}{2}\left[
  \left(\frac{n}{\sigma^2} + \frac{1}{\tau^2}\right) \mu^2
  - 2 \mu \left( \frac{n}{\sigma^2} \bar y + \frac{\mu_0}{\tau^2} \right)
  + \frac{n}{\sigma^2}\bar y^2 + \frac{\mu_0^2}{\tau^2}
  \right]
  \right) \ d \mu \\
  %%%
  %
  &= \frac{C_2}{\sqrt{2\pi}} v^{1/2}\frac{1}{v^{1/2}}\int
  \exp\left( -\frac{1}{2}\frac{1}{v}
  \left[\mu^2 - 2 \mu \tilde \mu + \tilde\mu^2 - \tilde\mu^2\right]
  \right) \ d \mu \\
  &= C_2 v^{1/2} \\
  &=
  \left(\frac{1}{2\pi\sigma^2}\right)^{n/2}
  \left(\frac{v}{\tau^2}\right)^{1/2}
  \exp\left(
  -\frac{1}{2}
  \left[
  \frac{(n-1)s^2}{\sigma^2}
  + \frac{n\bar y^2}{\sigma^2}
  + \frac{\mu_0^2}{\tau^2}
  - \frac{\tilde\mu^2}{v}
  \right]
  \right)
  \end{split}
  \end{equation*}
  \end{document}
*/
  double GaussianBartPosteriorSampler::log_integrated_likelihood(
      const Bart::SufficientStatisticsBase *suf) const {
    return log_integrated_gaussian_likelihood(
        dynamic_cast<const Bart::GaussianBartSufficientStatistics *>(suf));
  }

  double GaussianBartPosteriorSampler::log_integrated_gaussian_likelihood(
      const Bart::GaussianBartSufficientStatistics *suf) const {
    double n = suf->n();
    if (n < 5) {
      return negative_infinity();
    }
    const double log_2_pi = 1.83787706640935;
    double prior_mean = node_mean_prior()->mu();
    double prior_variance = node_mean_prior()->sigsq();
    double sigsq = model_->sigsq();

    double ybar = suf->ybar();
    double sample_variance = suf->sample_var();

    double ivar = (n / sigsq) + (1.0 / prior_variance);
    double posterior_variance = 1.0 / ivar;
    double posterior_mean =
        ((n * ybar / sigsq) + (prior_mean / prior_variance)) / ivar;

    double ans = -n * (log_2_pi + log(sigsq))
        + log(posterior_variance / prior_variance)
        - (n - 1) * sample_variance / sigsq
        - n * square(ybar) / sigsq
        - square(prior_mean) / prior_variance
        + square(posterior_mean) / posterior_variance;
    return .5 * ans;
  }

  void GaussianBartPosteriorSampler::clear_residuals() {
    residuals_.clear();
  }

  int GaussianBartPosteriorSampler::residual_size() const {
    return residuals_.size();
  }

  Bart::GaussianResidualRegressionData *
  GaussianBartPosteriorSampler::create_and_store_residual(int i) {
    Ptr<RegressionData> dp = model_->dat()[i];
    double original_prediction = model_->predict(dp->x());
    boost::shared_ptr<Bart::GaussianResidualRegressionData> data(
        new Bart::GaussianResidualRegressionData(
            dp, original_prediction));
    residuals_.push_back(data);
    return data.get();
  }

  void GaussianBartPosteriorSampler::draw_residual_variance() {
    double ss = siginv_prior_->beta() / 2.0;
    double df = siginv_prior_->alpha() / 2.0;
    int n = residuals_.size();
    df += n;
    for (int i = 0; i < n; ++i) {
      ss += square(residuals_[i]->residual());
    }

    double siginv = rgamma_mt(rng(), df / 2.0, ss / 2.0);
    model_->set_sigsq(1.0 / siginv);
  }

  const std::vector<const Bart::GaussianResidualRegressionData *>
  GaussianBartPosteriorSampler::residuals() const {
    std::vector<const Bart::GaussianResidualRegressionData *> ans;
    ans.reserve(residuals_.size());
    for (int i = 0; i < residuals_.size(); ++i) {
      ans.push_back(residuals_[i].get());
    }
    return ans;
  }

}  // namespace BOOM
