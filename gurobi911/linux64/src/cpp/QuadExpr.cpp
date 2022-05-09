// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"

GRBQuadExpr::GRBQuadExpr(double c)
{
  linexpr = GRBLinExpr(c);
}

GRBQuadExpr::GRBQuadExpr(GRBVar v, double coeff)
{
  linexpr = coeff * v;
}

GRBQuadExpr::GRBQuadExpr(GRBLinExpr le)
{
  linexpr = le;
}

GRBQuadExpr
GRBQuadExpr::operator=(const GRBQuadExpr& rhs)
{
  linexpr = rhs.linexpr;
  coeffs  = rhs.coeffs;
  vars1   = rhs.vars1;
  vars2   = rhs.vars2;

  return *this;
}

void
GRBQuadExpr::multAdd(double m, const GRBQuadExpr& expr)
{
  if (m == 0) return;

  linexpr.multAdd(m, expr.linexpr);

  GRBQuadExpr tmp;
  const vector<double>* addcoeffs;
  const vector<GRBVar>* addvars1;
  const vector<GRBVar>* addvars2;

  /* to avoid an endless loop when adding an expression to itself, we copy
   * the content of the expr and use the data inside the copy */
  if (&this->coeffs == &expr.coeffs) {
    tmp = GRBQuadExpr();
    tmp.coeffs.insert(tmp.coeffs.end(), expr.coeffs.begin(), expr.coeffs.end());
    tmp.vars1.insert(tmp.vars1.end(), expr.vars1.begin(), expr.vars1.end());
    tmp.vars2.insert(tmp.vars2.end(), expr.vars2.begin(), expr.vars2.end());
    addcoeffs = &(tmp.coeffs);
    addvars1  = &(tmp.vars1);
    addvars2  = &(tmp.vars2);
  } else  {
    addcoeffs = &(expr.coeffs);
    addvars1  = &(expr.vars1);
    addvars2  = &(expr.vars2);
  }

  if (m == 1.0) {
    coeffs.insert(coeffs.end(), addcoeffs->begin(), addcoeffs->end());
  } else {
    unsigned int size = (*addcoeffs).size();

    for (unsigned int i = 0; i < size; i++) {
      coeffs.push_back(m * (*addcoeffs)[i]);
    }
  }
  vars1.insert(vars1.end(), addvars1->begin(), addvars1->end());
  vars2.insert(vars2.end(), addvars2->begin(), addvars2->end());
}

void
GRBQuadExpr::operator+=(const GRBQuadExpr& expr)
{
  this->multAdd(1.0, expr);
}

void
GRBQuadExpr::operator-=(const GRBQuadExpr& expr)
{
  this->multAdd(-1.0, expr);
}

void
GRBQuadExpr::operator*=(double mult)
{
  linexpr *= mult;
  for (unsigned int i = 0; i < vars1.size(); i++) {
    coeffs[i] *= mult;
  }
}

void
GRBQuadExpr::operator/=(double a)
{
  linexpr /= a;
  for (unsigned int i = 0; i < vars1.size(); i++) {
    coeffs[i] /= a;
  }
}

GRBQuadExpr
GRBQuadExpr::operator+(const GRBQuadExpr& rhs)
{
  GRBQuadExpr result = *this;
  result.multAdd(1.0, rhs);
  return result;
}

GRBQuadExpr
GRBQuadExpr::operator-(const GRBQuadExpr& rhs)
{
  GRBQuadExpr result = *this;
  result.multAdd(-1.0, rhs);
  return result;
}

unsigned int
GRBQuadExpr::size(void) const
{
  return vars1.size();
}

GRBVar
GRBQuadExpr::getVar1(int i) const
{
  return vars1[i];
}

GRBVar
GRBQuadExpr::getVar2(int i) const
{
  return vars2[i];
}

double
GRBQuadExpr::getCoeff(int i) const
{
  return coeffs[i];
}

GRBLinExpr
GRBQuadExpr::getLinExpr() const
{
  return linexpr;
}

double
GRBQuadExpr::getValue() const
{
  double value = linexpr.getValue();
  for (unsigned int i = 0; i < vars1.size(); i++)
    value += coeffs[i]*vars1[i].get(GRB_DoubleAttr_X)*
             vars2[i].get(GRB_DoubleAttr_X);
  return value;
}

void
GRBQuadExpr::addConstant(double c)
{
  linexpr += c;
}

void
GRBQuadExpr::addTerm(double coeff, GRBVar var)
{
  linexpr += coeff*var;
}

void
GRBQuadExpr::addTerm(double coeff, GRBVar var1, GRBVar var2)
{
  coeffs.push_back(coeff);
  vars1.push_back(var1);
  vars2.push_back(var2);
}

void
GRBQuadExpr::addTerms(const double* coeff,
                      const GRBVar* var,
                      int   cnt)
{
  linexpr.addTerms(coeff, var, cnt);
}

void
GRBQuadExpr::addTerms(const double* coeff,
                      const GRBVar* var1,
                      const GRBVar* var2,
                      int cnt)
{
  for (int i = 0; i < cnt; i++) {
    coeffs.push_back(coeff[i]);
    vars1.push_back(var1[i]);
    vars2.push_back(var2[i]);
  }
}

void
GRBQuadExpr::add(const GRBLinExpr le)
{
  linexpr += le;
}

void
GRBQuadExpr::remove(int i)
{
  if (i < 0 || i >= (int) vars1.size()) throw
    GRBException("invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  coeffs.erase(coeffs.begin()+i);
  vars1.erase(vars1.begin()+i);
  vars2.erase(vars2.begin()+i);
}

bool
GRBQuadExpr::remove(GRBVar v)
{
  int size = (int) vars1.size();
  for(int i = size -1; i >= 0; i--) {
    if (v.varRep == vars1[i].varRep || v.varRep == vars2[i].varRep) {
      coeffs.erase(coeffs.begin()+i);
      vars1.erase(vars1.begin()+i);
      vars2.erase(vars2.begin()+i);
    }
  }

  bool ret = linexpr.remove(v);

  if (ret || size > (int) vars1.size()) return true;
  else                                  return false;
}

void
GRBQuadExpr::clear(void)
{
  linexpr.clear();
  coeffs.clear();
  vars1.clear();
  vars2.clear();
}

ostream &
operator<<(ostream &stream, GRBQuadExpr expr)
{
  for (unsigned int i = 0; i < expr.coeffs.size(); i++)
    stream << "+ " << expr.coeffs[i] << " "
           << expr.vars1[i].get(GRB_StringAttr_VarName) << " * "
           << expr.vars2[i].get(GRB_StringAttr_VarName) << " " ;
  stream << expr.linexpr;

  return stream;
}

GRBQuadExpr
operator+(const GRBQuadExpr& x,
          const GRBQuadExpr& y)
{
  GRBQuadExpr result = x;

  result += y;

  return result;
}

GRBQuadExpr
operator-(const GRBQuadExpr& x,
          const GRBQuadExpr& y)
{
  GRBQuadExpr result = x;

  result -= y;

  return result;
}

GRBQuadExpr
operator+(const GRBQuadExpr& x)
{
  GRBQuadExpr result = x;

  return result;
}

GRBQuadExpr
operator-(const GRBQuadExpr& x)
{
  GRBQuadExpr result = GRBQuadExpr();
  result -= x;

  return result;
}

GRBQuadExpr
operator*(const GRBQuadExpr& x,
          double a)
{
  return a * x;
}

GRBQuadExpr
operator*(double a,
          const GRBQuadExpr& x)
{
  GRBQuadExpr result = x;
  result *= a;
  return result;
}

GRBQuadExpr
operator*(GRBVar x,
          GRBVar y)
{
  GRBQuadExpr result;
  result.addTerm(1.0, x, y);
  return result;
}

GRBQuadExpr
operator*(GRBVar x, const GRBLinExpr& y)
{
  GRBQuadExpr result;

  if (y.getConstant() != 0.0)
    result.linexpr += y.getConstant() * x;

  for (unsigned int i = 0; i < y.size(); i++)
    result.addTerm(y.getCoeff(i), y.getVar(i), x);

  return result;
}

GRBQuadExpr
operator*(const GRBLinExpr& y, GRBVar x)
{
  return x*y;
}

GRBQuadExpr
operator*(const GRBLinExpr& x, const GRBLinExpr& y)
{
  double c = y.getConstant();
  GRBQuadExpr result;

  if (c != 0.0)
    result += c * x;

  for (unsigned int i = 0; i < y.size(); i++)
    result += y.getVar(i) * x * y.getCoeff(i);

  return result;
}

GRBQuadExpr
operator/(const GRBQuadExpr& x,
          double a)
{
  GRBQuadExpr result = x;
  return (1.0/a) * result;
}
