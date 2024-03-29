/*
  Copyright (C) 2008 Steven L. Scott

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
#include "stack_columns.hpp"
#include <LinAlg/Matrix.hpp>

namespace BOOM{
  Vector stack_columns(const Matrix &m){
    uint n = m.size();
    Vector ans(n);
    Vector::iterator it = ans.begin();
    uint nc = m.ncol();
    for(uint i=0; i<nc; ++i)
      it = std::copy(m.col_begin(i), m.col_end(i), it);
    return ans;
  }
}
