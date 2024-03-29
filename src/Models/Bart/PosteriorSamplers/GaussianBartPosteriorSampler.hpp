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
#ifndef BOOM_GAUSSIAN_BART_POSTERIOR_SAMPLER_HPP_
#define BOOM_GAUSSIAN_BART_POSTERIOR_SAMPLER_HPP_

#include <Models/Bart/Bart.hpp>
#include <Models/Bart/ResidualRegressionData.hpp>
#include <Models/Bart/GaussianBartModel.hpp>
#include <Models/Bart/PosteriorSamplers/BartPosteriorSampler.hpp>
#include <Models/ChisqModel.hpp>

namespace BOOM {
  namespace Bart {
    class GaussianBartSufficientStatistics;
    // This is the internal data type managed by the
    // GaussianBartPosteriorSampler, and fed to the nodes of the
    // GaussianBartModel model being managed.
    class GaussianResidualRegressionData
        : public ResidualRegressionData {
     public:
      // The data_point argument retains ownership of the data it
      // manages.  It must remain in scope while the
      // GaussianBartPosteriorSampler does its thing.  This should be
      // fine, as data_point itself is owned by the GaussianBartModel
      // managed by the GaussianBartPosteriorSampler.
      //
      // At construction time, the residual is the same as the
      // original observed response.
      GaussianResidualRegressionData(
          const Ptr<RegressionData> &data_point,
          double original_prediction);
      double y()const {return observed_response_->y();}
      double residual()const {return residual_;}
      virtual void add_to_residual(double value) {residual_ += value;}

      virtual void add_to_gaussian_suf(GaussianBartSufficientStatistics &suf)const;
     private:
      const RegressionData *observed_response_;
      double residual_;
    };

    class GaussianBartSufficientStatistics
        : public SufficientStatisticsBase {
     public:
      virtual GaussianBartSufficientStatistics * clone() const {
        return new GaussianBartSufficientStatistics(*this);
      }
      virtual void clear() {suf_.clear();}
      virtual void update(const ResidualRegressionData &abstract_data) {
        abstract_data.add_to_gaussian_suf(*this);
      }
      virtual void update(const GaussianResidualRegressionData &data) {
        suf_.update_raw(data.residual());
      }
      double n() const {return suf_.n();}
      double ybar() const {return suf_.ybar();}
      double sum() const {return suf_.sum();}
      double sample_var() const {return suf_.sample_var();}
     private:
      GaussianSuf suf_;
    };

  }  // namespace Bart

  class GaussianBartPosteriorSampler
      : public BartPosteriorSamplerBase {
    // The prior is that the probability of a node at depth 'd'
    // splitting is a / (1 + d)^b.  Given a split, a variable is
    // chosen uniformly from the set of available variables, and a
    // cutpoint uniformly from the set of available cutpoints.  Note
    // that 'available' is influenced by a node's position in the
    // tree, because splits made by ancestors will make some splits
    // logically impossible, and impossible splits are not
    // 'available.'  For example, descendents cannot split on the same
    // dummy variable as an ancestor.  The conditional prior on the
    // mean parameters at the leaves is N(prior_mean_guess,
    // prior_mean_sd), and the prior on the residual variance is
    // 1/sigma^2 ~ Gamma( prior_sigma_weight / 2, prior_sigma_weight *
    // prior_sigma_guess^2 / 2).
   public:
    GaussianBartPosteriorSampler(GaussianBartModel *model,
                                 double prior_sigma_guess,
                                 double prior_sigma_weight,
                                 double prior_mean_guess,
                                 double prior_mean_sd,
                                 double prior_tree_depth_alpha,
                                 double prior_tree_depth_beta);
    //----------------------------------------------------------------------
    // Virtual function over-rides....

    virtual void draw();
    virtual double draw_mean(Bart::TreeNode *leaf);

    virtual double log_integrated_likelihood(
        const Bart::SufficientStatisticsBase *suf)const;
    double log_integrated_gaussian_likelihood(
        const Bart::GaussianBartSufficientStatistics *suf)const;
    virtual void clear_residuals();
    virtual int residual_size()const;

    virtual Bart::GaussianResidualRegressionData *
    create_and_store_residual(int i);

    Bart::GaussianBartSufficientStatistics * create_suf() const {
      return new Bart::GaussianBartSufficientStatistics;
    }

    //----------------------------------------------------------------------
    // non-virtual functions start here.

    // Draw the residual variance given structure and mean parameters.
    void draw_residual_variance();

    const std::vector<const Bart::GaussianResidualRegressionData *>
    residuals() const;

   private:
    GaussianBartModel *model_;
    Ptr<ChisqModel> siginv_prior_;

    // Residuals will be held by all the nodes in all the trees.
    // Local changes will be reflected in other trees, so they need to
    // be locally adjusted before they are used.  This makes the
    // algorithm thread-unsafe.
    std::vector<
      boost::shared_ptr<
        Bart::GaussianResidualRegressionData> > residuals_;
  };

}  // namespace BOOM

#endif  //  BOOM_GAUSSIAN_BART_POSTERIOR_SAMPLER_HPP_
