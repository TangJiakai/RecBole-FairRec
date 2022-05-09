// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"

GRBLinExpr::GRBLinExpr(double xconstant)
{
  constant = xconstant;
}

GRBLinExpr::GRBLinExpr(GRBVar var, double coeff)
{
  constant = 0.0;
  coeffs.push_back(coeff);
  vars.push_back(var);
}

GRBLinExpr
GRBLinExpr::operator=(const GRBLinExpr& rhs)
{
  constant = rhs.constant;
  coeffs   = rhs.coeffs;
  vars     = rhs.vars;

  return *this;
}

void
GRBLinExpr::multAdd(double m, const GRBLinExpr& expr)
{
  if (m == 0) return;

  GRBLinExpr tmp;
  const vector<double>* addcoeffs;
  const vector<GRBVar>* addvars;

  /* to avoid an endless loop when adding an expression to itself, we copy
   * the expr and use the data inside the copy */
  if (&this->coeffs == &expr.coeffs) {
    tmp = expr;
    addcoeffs = &(tmp.coeffs);
    addvars   = &(tmp.vars);
  } else {
    addcoeffs = &(expr.coeffs);
    addvars   = &(expr.vars);
  }

  if (m == 1.0) {
    coeffs.insert(coeffs.end(), addcoeffs->begin(), addcoeffs->end());
  } else {
    unsigned int size = (*addcoeffs).size();

    for (unsigned int i = 0; i < size; i++) {
      coeffs.push_back(m * (*addcoeffs)[i]);
    }
  }
  vars.insert(vars.end(), addvars->begin(), addvars->end());

  constant += m * expr.constant;
}

void
GRBLinExpr::operator+=(const GRBLinExpr& expr)
{
  this->multAdd(1.0, expr);
}

void
GRBLinExpr::operator-=(const GRBLinExpr& expr)
{
  this->multAdd(-1.0, expr);
}

void
GRBLinExpr::operator*=(double mult)
{
  constant *= mult;
  for (unsigned int i = 0; i < vars.size(); i++) {
    coeffs[i] *= mult;
  }
}

void
GRBLinExpr::operator/=(double a)
{
  constant /= a;
  for (unsigned int i = 0; i < vars.size(); i++) {
    coeffs[i] /= a;
  }
}

GRBLinExpr
GRBLinExpr::operator+(const GRBLinExpr& rhs)
{
  GRBLinExpr result = *this;
  result.multAdd(1.0, rhs);
  return result;
}

GRBLinExpr
GRBLinExpr::operator-(const GRBLinExpr& rhs)
{
  GRBLinExpr result = *this;
  result.multAdd(-1.0, rhs);
  return result;
}

unsigned int 
GRBLinExpr::size(void) const 
{ 
  return vars.size(); 
}

GRBVar 
GRBLinExpr::getVar(int i) const 
{ 
  return vars[i]; 
}

double 
GRBLinExpr::getCoeff(int i) const 
{ 
  return coeffs[i]; 
}

double
GRBLinExpr::getConstant() const
{
  return constant;
}

double
GRBLinExpr::getValue() const
{
  double value = constant;
  for (unsigned int i = 0; i < vars.size(); i++)
    value += coeffs[i]*vars[i].get(GRB_DoubleAttr_X);
  return value;
}

void
GRBLinExpr::addTerms(const double* coeff,
                     const GRBVar* var,
                     int   cnt)
{
  for (int i = 0; i < cnt; i++) {
    if (coeff == NULL) coeffs.push_back(1.0);
    else               coeffs.push_back(coeff[i]);
    vars.push_back(var[i]);
  }
}

void
GRBLinExpr::remove(int i)
{
  if (i < 0 || i >= (int) vars.size()) throw
    GRBException("invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  coeffs.erase(coeffs.begin()+i);
  vars.erase(vars.begin()+i);
}

bool
GRBLinExpr::remove(GRBVar v)
{
  int size = (int) vars.size();
  for(int i = size -1; i >= 0; i--) {
    if (v.varRep == vars[i].varRep) {
      coeffs.erase(coeffs.begin()+i);
      vars.erase(vars.begin()+i);
    }
  }
  if (size > (int) vars.size()) return true;
  else                          return false;
}

void
GRBLinExpr::clear(void)
{
  constant = 0.0;
  coeffs.clear();
  vars.clear();
}

ostream &
operator<<(ostream &stream, GRBLinExpr expr)
{
  for (unsigned int i = 0; i < expr.coeffs.size(); i++)
    stream << "+ " << expr.coeffs[i] << " "
           << expr.vars[i].get(GRB_StringAttr_VarName) << " " ;
  if (expr.constant != 0)
    stream << "+ " << expr.constant;

  return stream;
}

GRBLinExpr
operator+(const GRBLinExpr& x,
          const GRBLinExpr& y)
{
  GRBLinExpr result = x;

  result += y;

  return result;
}

GRBLinExpr
operator-(const GRBLinExpr& x,
          const GRBLinExpr& y)
{
  GRBLinExpr result = x;

  result -= y;

  return result;
}

GRBLinExpr
operator+(const GRBLinExpr& x)
{
  GRBLinExpr result = x;

  return result;
}

GRBLinExpr
operator+(GRBVar x, GRBVar y)
{
  GRBLinExpr result = GRBLinExpr(x);
  result += y;

  return result;
}

GRBLinExpr
operator+(GRBVar x, double a)
{
  GRBLinExpr result = GRBLinExpr(x);
  result += a;

  return result;
}

GRBLinExpr
operator+(double a, GRBVar x)
{
  GRBLinExpr result = GRBLinExpr(x);
  result += a;

  return result;
}

GRBLinExpr
operator-(GRBVar x, GRBVar y)
{
  GRBLinExpr result = GRBLinExpr(x);
  result -= y;

  return result;
}

GRBLinExpr
operator-(GRBVar x, double a)
{
  GRBLinExpr result = GRBLinExpr(x);
  result -= a;

  return result;
}

GRBLinExpr
operator-(double a, GRBVar x)
{
  GRBLinExpr result = GRBLinExpr(a);
  result -= x;

  return result;
}

GRBLinExpr
operator-(const GRBLinExpr& x)
{
  GRBLinExpr result = GRBLinExpr();
  result -= x;

  return result;
}

GRBLinExpr
operator-(GRBVar x)
{
  return -1.0 * x;
}

GRBLinExpr
operator*(double a,
          GRBVar x)
{
  return GRBLinExpr(x, a);
}
GRBLinExpr
operator*(GRBVar x,
          double a)
{
  return GRBLinExpr(x, a);
}

GRBLinExpr
operator*(const GRBLinExpr& x,
          double a)
{
  GRBLinExpr result = 0;
  result.multAdd(a, x);
  return result;
}
GRBLinExpr
operator*(double a,
          const GRBLinExpr& x)
{
  GRBLinExpr result = 0;
  result.multAdd(a, x);
  return result;
}

GRBLinExpr
operator/(GRBVar x,
          double a)
{
  return GRBLinExpr(x, 1.0/a);
}

GRBLinExpr
operator/(const GRBLinExpr& x,
          double a)
{
  GRBLinExpr result = x;
  return (1.0/a) * result;
}
