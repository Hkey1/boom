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
 *  AUTHOR
 *    Catherine Loader, catherine@research.bell-labs.com.
 *    October 23, 2000.
 *
 *  Merge in to R:
 *	Copyright (C) 2000, 2001 The R Core Development Team
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
 *
 *  DESCRIPTION
 *
 *    Computes the geometric probabilities, Pr(X=x) = p(1-p)^x.
 */

#include "nmath.hpp"
#include "dpq.hpp"
namespace Rmath{

double dgeom(double x, double p, int give_log)
{
    double prob;

#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(p)) return x + p;
#endif

    if (p < 0 || p > 1) ML_ERR_return_NAN;

    R_D_nonint_check(x);
    if (x < 0 || !R_FINITE(x) || p == 0) return R_D__0;
    x = R_D_forceint(x);

    /* prob = (1-p)^x, stable for small p */
    prob = dbinom_raw(0.,x, p,1-p, give_log);

    return((give_log) ? log(p) + prob : p*prob);
}
}

