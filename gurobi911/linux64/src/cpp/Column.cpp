// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"

unsigned int 
GRBColumn::size(void) const 
{ 
  return constrs.size(); 
}

GRBConstr 
GRBColumn::getConstr(int i) const 
{ 
  return constrs[i]; 
}

double 
GRBColumn::getCoeff(int i) const 
{ 
  return coeffs[i]; 
}

void
GRBColumn::addTerm(double coeff, GRBConstr constr)
{
  coeffs.push_back(coeff);
  constrs.push_back(constr);
}

void
GRBColumn::addTerms(const double* coeff, const GRBConstr* constr, int cnt)
{
  for (int i = 0; i < cnt; i++) {
    coeffs.push_back(coeff[i]);
    constrs.push_back(constr[i]);
  }
}

void
GRBColumn::remove(int i)
{
  if (i < 0 || i >= (int) constrs.size()) throw
    GRBException("invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  coeffs.erase(coeffs.begin()+i);
  constrs.erase(constrs.begin()+i);
}

bool
GRBColumn::remove(GRBConstr c)
{
  unsigned int i;
  for(i = 0; i < constrs.size(); i++) {
    if (c.conRep == constrs[i].conRep) break;
  }
  if (i < constrs.size()) {
    coeffs.erase(coeffs.begin()+i);
    constrs.erase(constrs.begin()+i);
    return true;
  } else {
    return false;
  }
}

void
GRBColumn::clear(void)
{
  coeffs.clear();
  constrs.clear();
}
