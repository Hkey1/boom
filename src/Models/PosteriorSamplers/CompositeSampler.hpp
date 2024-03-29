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
#ifndef BOOM_COMPOSITE_SAMPLER_HPP
#define BOOM_COMPOSITE_SAMPLER_HPP

#include <Models/PosteriorSamplers/PosteriorSampler.hpp>

namespace BOOM{

  class CompositeSampler;
  class CompositeSamplerAdder{
  public:
    CompositeSamplerAdder(CompositeSampler *pcs);
    CompositeSamplerAdder operator()(Ptr<PosteriorSampler>,
				     double wgt=1.0);
  private:
    CompositeSampler * cs;
  };
  //----------------------------------------------------------------------
  // A posterior sampler that is made of one or more other posterior
  // samplers.  Each iteration one of the component samplers will be
  // selected at random and run.
  class CompositeSampler: public PosteriorSampler{
  public:
    CompositeSampler();
    CompositeSampler(Ptr<PosteriorSampler> s, double prob=1.0);
    CompositeSampler(const std::vector<Ptr<PosteriorSampler> > & s);
    CompositeSampler(const std::vector<Ptr<PosteriorSampler> > & s,
		     const Vec & Probs);
    template <class It>
    CompositeSampler(It b, It e)
      : samplers_(b,e),
	probs_(samplers_.size(), 1.0/samplers_.size())
    {}

    virtual void draw();
    virtual double logpri()const;
    CompositeSamplerAdder add_sampler(Ptr<PosteriorSampler>, double w=1.0);
  private:
    std::vector<Ptr<PosteriorSampler> > samplers_;
    Vec probs_;
    Ptr<PosteriorSampler> choose_sampler()const;
  };
}

#endif //BOOM_COMPOSITE_SAMPLER_HPP
