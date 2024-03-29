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
 *  Mathlib - A Mathematical Function Library
 *  Copyright (C) 1998  Ross Ihaka
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* NaNs propagated correctly */


/*-- FIXME:  Eliminate calls to these
 *   =====   o   from C code when
 *	     o   it is only used to initialize "static" variables (threading)
 *  and use the DBL_... constants instead
 */

#include "nmath.hpp"
namespace Rmath{

double d1mach(int i)
{
    switch(i) {
    case 1: return numeric_limits<double>::min();
    case 2: return numeric_limits<double>::max();

    case 3: /* = FLT_RADIX  ^ - DBL_MANT_DIG
	      for IEEE:  = 2^-53 = 1.110223e-16 = .5*numeric_limits<double>::epsilon() */
	return pow((double)i1mach(10), -(double)i1mach(14));

    case 4: /* = FLT_RADIX  ^ (1- DBL_MANT_DIG) =
	      for IEEE:  = 2^52 = 4503599627370496 = 1/numeric_limits<double>::epsilon() */
	return pow((double)i1mach(10), 1-(double)i1mach(14));

    case 5: return log10(2.0);/* = M_LOG10_2 in Bmath.hpp */


    default: return 0.0;
    }
}

// double F77_NAME(d1mach)(int *i)
// {
//     return Rf_d1mach(*i);
// }
}

