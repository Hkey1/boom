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

/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000 The R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
 *
 *  SYNOPSIS
 *
 *    #include "mathlib.h"
 *    double rf(double dfn, double dfd);
 *
 *  DESCRIPTION
 *
 *    Pseudo-random variates from an F distribution.
 *
 *  NOTES
 *
 *    This function calls rchisq to do the real work
 */

#include "nmath.hpp"
#include "rchisq_mt.hpp"
namespace Rmath{

  template <class ENG>
  double rf_mt(ENG & eng, double n1, double n2)
  {
    double v1, v2;
    if (ISNAN(n1) || ISNAN(n2) || n1 <= 0. || n2 <= 0.)
      ML_ERR_return_NAN;

    v1 = R_FINITE(n1) ? (rchisq_mt(eng, n1) / n1) : 1;
    v2 = R_FINITE(n2) ? (rchisq_mt(eng, n2) / n2) : 1;
    return v1 / v2;
  }
}

