/*
  Copyright (C) 2005-2012 Steven L. Scott

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

#ifndef BOOM_HOMOGENEOUS_POISSON_PROCESS_HPP_
#define BOOM_HOMOGENEOUS_POISSON_PROCESS_HPP_

#include <Models/PointProcess/PointProcess.hpp>
#include <Models/PointProcess/PoissonProcess.hpp>
#include <Models/Policies/ParamPolicy_1.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/Policies/SufstatDataPolicy.hpp>
#include <cpputil/DateTime.hpp>

namespace BOOM{

  // A PoissonProcessSuf differs from a PoissonSuf (in
  // Models/PoissonModel.hpp) because
  class PoissonProcessSuf : public SufstatDetails<PointProcess> {
   public:
    PoissonProcessSuf(int count = 0, double exposure = 0);
    virtual PoissonProcessSuf * clone()const;

    int count()const;
    double exposure()const;

    void Update(const PointProcess &process);
    virtual void clear();
    void update_raw(int number_of_events, double duration);
    void update_raw(const PointProcess &data);

    PoissonProcessSuf * combine(Ptr<PoissonProcessSuf> rhs);
    PoissonProcessSuf * combine(const PoissonProcessSuf &rhs);
    virtual PoissonProcessSuf * abstract_combine(Sufstat *rhs);

    // Vectorized sufficient stats have two entries: count and
    // exposure.
    virtual Vec vectorize(bool minimal = true)const;
    virtual Vec::const_iterator unvectorize(Vec::const_iterator &v,
                                            bool minimal = true);
    virtual Vec::const_iterator unvectorize(const Vec &v,
                                            bool minimal = true);

    virtual ostream & print(ostream &out)const;
   private:
    int number_of_events_;
    double exposure_time_;

  };
//======================================================================
  class HomogeneousPoissonProcess
      : public PoissonProcess,
        public ParamPolicy_1<UnivParams>,
        public SufstatDataPolicy<PointProcess, PoissonProcessSuf>,
        public PriorPolicy,
        public LoglikeModel
  {
   public:
    HomogeneousPoissonProcess(double lambda = 1.0);
    HomogeneousPoissonProcess(const std::vector<DateTime> &timestamps);

    virtual HomogeneousPoissonProcess * clone()const;
    double lambda()const;
    void set_lambda(double lambda);
    Ptr<UnivParams> Lambda_prm(){return ParamPolicy::prm();}
    const Ptr<UnivParams> Lambda_prm()const{return ParamPolicy::prm();}

    virtual double event_rate(const DateTime &t)const;
    virtual double expected_number_of_events(const DateTime &t0,
                                             const DateTime &t1)const;

    // Updates sufficient statistics, but does not allocate a new
    // Ptr<PointProcess> data element.
    void add_data_raw(int incremental_events, double incremental_duration);
    void add_data_raw(const PointProcess &);
    virtual void add_exposure_window(const DateTime &t0,
                                   const DateTime &t1);
    virtual void add_event(const DateTime &t);

    virtual double loglike()const;
    virtual void mle();
    virtual PointProcess simulate(
        const DateTime &t0,
        const DateTime &t1,
        boost::function<Data*()> mark_generator = NullDataGenerator())const;
  };

}
#endif // BOOM_HOMOGENEOUS_POISSON_PROCESS_HPP_
