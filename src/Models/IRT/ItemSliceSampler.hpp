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
#ifndef BOOM_IRT_ITEM_SLICE_SAMPLER_HPP
#define BOOM_IRT_ITEM_SLICE_SAMPLER_HPP
#include <Models/PosteriorSamplers/PosteriorSampler.hpp>
#include <Models/IRT/Item.hpp>

namespace BOOM{
  class SliceSampler;
  namespace IRT{

    class ItemSliceSampler
      : public PosteriorSampler{
    public:
      ItemSliceSampler(Ptr<Item>);
      ItemSliceSampler * clone()const;
      virtual void draw();
      virtual double logpri()const;
    private:
      Ptr<Item> it;
      Ptr<SliceSampler> sam;
    };
  }
}
#endif  // BOOM_IRT_ITEM_SLICE_SAMPLER_HPP
