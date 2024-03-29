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
#include "Markov.hpp"
#include <LinAlg/Vector.hpp>
#include <LinAlg/Matrix.hpp>
#include <LinAlg/Selector.hpp>
#include <vector>

namespace BOOM{

  Vec get_stat_dist(const Mat &Q){
    Mat P = Q.t();  // transpose
    P.diag()-= 1.0;
    P.row(0) = 1.0;
    Vec ans(Q.nrow(), 0.0);
    ans[0]=1.0;
    return P.solve(ans); }

  Mat compute_conditional_absorption_probs(const Mat &P, const Selector & abs){
    Selector transient(abs.complement());
    Mat Q = transient.select_square(P);
    Mat R = abs.select_cols(transient.select_rows(P));
    uint ntrans = Q.nrow();
    Mat F(ntrans, ntrans, 0.0);
    F.set_diag(1.0);
    F-= Q;                // F = I - Q is the fundamental matrix
    return F.solve(R);
  }

  // returns the probability that a markov chain with initial
  // distribution pi0 and transition matrix Q enters state r before
  // entering state s
  double preceeds(const Selector & r, const Selector &s,
                  const Vec &pi0, const Mat & P){
    assert(P.ncol()==P.nrow());
    assert(pi0.size()==P.nrow());
    assert(r.nvars_possible()==P.nrow());
    assert(s.nvars_possible()==P.nrow());
    Selector absorbing = r.Union(s);
    Selector transient = absorbing.complement();

    Mat Q = transient.select_square(P);
    Mat R = absorbing.select_cols(transient.select_rows(P));
    Mat F = Q.Id() - Q;;

    Vec pi0_trans = transient.select(pi0);
    Vec pi0_abs = absorbing.select(pi0);
    Vec subtotal =  pi0_trans * F.solve(R);

    // rmask is a 0/1 vector of length absorbing.nvars(), with 1's
    // inidcating an 'r' position and 0 indicating an 's' position
    Vec rmask = absorbing.select(r.vec());
    double ans =  subtotal.dot(rmask) + pi0_abs.dot(rmask);
    return ans;
  }

  double preceeds(uint r, uint s, const Vec &pi0, const Mat &P){
    // returns the probability that state r happens before state s in
    // a Markov chain with initial distribution pi0 and transition
    // matrix P.
    if(r==s) return 0;
    uint dim = P.ncol();
    Selector R(dim, false);
    R.add(r);
    Selector S(dim, false);
    S.add(s);
    return preceeds(R,S,pi0,P);
  }
}
