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

#ifndef IRT_SUBJECT_HPP
#define IRT_SUBJECT_HPP

#include "IRT.hpp"
#include <Models/Policies/ParamPolicy_1.hpp>
#include <Models/Policies/IID_DataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>

namespace BOOM{
  namespace IRT{
    class Item;

    // 'Subject' means 'observational unit' (e.g. student) not
    // 'subject matter'

    class Subject : virtual public Data  // data is the sequence of responses
    {
    public:
      friend class IrtModel;

      Subject(const string &Id, uint nscal);
      Subject(const string &Id, const Vec & theta);
      Subject(const string &Id, uint nscal, const Vec & background_vars);
      Subject(const Subject &rhs);
      Subject * clone()const;

      Response add_item(Ptr<Item>, uint response);
      Response add_item(Ptr<Item>, const string &response);
      Response add_item(Ptr<Item>, Response r);

      // find this subject's response to an item

      const ItemResponseMap & item_responses()const;
      Response response(const Ptr<Item>)const;
      Ptr<Item> find_item(const string &item_id, bool nag=false)const;

      Ptr<VectorParams> Theta_prm();
      const Ptr<VectorParams> Theta_prm()const;
      const Vec & Theta()const;
      void set_Theta(const Vec &v);
      uint io_Theta(IO io_prm);
      void set_Theta_fname(const string &fname);

      ostream & display(ostream &)const;
      ostream & display_responses(ostream &)const;
      //      istream & read(istream &);
      uint size(bool minimal=true)const;
      // size must return number of responses, because Subject
      // inherits from data, and when viewed as a data element a
      // subject's data IS his set of responses

      uint Nitems()const;
      uint Nscales()const;

      virtual double loglike()const;
      const string & id()const;
      Spd xtx()const;
      // returns \sum_i \Beta_i \Beta_i^T for betas

      Response simulate_response(Ptr<Item>);
    private:
      string id_;                        // subject identifier
      ItemResponseMap responses_;
      Ptr<Item> search_helper;
      Ptr<VectorParams> Theta_;
      Vec x_;                            // covariates
      Response prototype;
    };
    //----------------------------------------------------------------------


  }
}
#endif // IRT_SUBJECT_HPP
