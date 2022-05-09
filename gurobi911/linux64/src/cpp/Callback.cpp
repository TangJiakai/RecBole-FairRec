// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"

#define TYPE_INT    1
#define TYPE_DOUBLE 2
#define TYPE_STRING 3
#define SCALAR      0

GRBCallback::GRBCallback()
{
  Cmodel = NULL;
  cols   = 0;
  cbdata = NULL;
  where  = 0;
  x      = NULL;
  newx   = NULL;
  relx   = NULL;
}

int
GRBCallback::xcb(GRBmodel *xmodel, void *xcbdata, int xwhere, void *xuserdata)
{
  GRBCallback *cb = (GRBCallback *) xuserdata;
  cb->cbdata = xcbdata;
  cb->where  = xwhere;
  cb->callback();
  if (cb->newx != NULL)
    cb->useSolution();
  if (cb->x != NULL) {
    delete[] cb->x;
    cb->x = NULL;
  }
  if (cb->relx != NULL) {
    delete[] cb->relx;
    cb->relx = NULL;
  }
  return 0;
}

void
GRBCallback::setcb(GRBCallback *cb, GRBmodel *xmodel, int xcols)
{
  Cmodel  = xmodel;
  cols    = xcols;
  int error = GRBsetcallbackfunc(Cmodel, xcb, (void*) cb);
  if (error) throw GRBException("Setting callback", error);
}

double
GRBCallback::getDoubleInfo(int what)
{
  double t;
  int type;
  int size;
  int error;

  error = GRBgetcbwhatinfo(cbdata, what, &type, &size);
  if (error) throw GRBException("getDoubleInfo", error);

  if (!(type == TYPE_DOUBLE && size == SCALAR)) {
    error = GRB_ERROR_INVALID_ARGUMENT;
    throw GRBException("getDoubleInfo: data requested must be a scalar double", error);
  }

  error = GRBcbget(cbdata, where, what, &t);
  if (error) throw GRBException("getDoubleInfo", error);
  return t;
}

int
GRBCallback::getIntInfo(int what)
{
  int t;
  int type;
  int size;
  int error;

  error = GRBgetcbwhatinfo(cbdata, what, &type, &size);
  if (error) throw GRBException("getIntInfo", error);

  if (!(type == TYPE_INT && size == SCALAR)) {
    error = GRB_ERROR_INVALID_ARGUMENT;
    throw GRBException("getIntInfo: data requested must be a scalar int", error);
  }

  error = GRBcbget(cbdata, where, what, &t);
  if (error) throw GRBException("getIntInfo", error);
  return t;
}

const string
GRBCallback::getStringInfo(int what) const
{
  const char *msg;
  int type;
  int size;
  int error;

  error = GRBgetcbwhatinfo(cbdata, what, &type, &size);
  if (error) throw GRBException("getStringInfo", error);

  if (!(type == TYPE_STRING && size == SCALAR)) {
    error = GRB_ERROR_INVALID_ARGUMENT;
    throw GRBException("getStringInfo: data requested must be a scalar string", error);
  }

  error = GRBcbget(cbdata, where, what, &msg);
  if (error) throw GRBException("getStringInfo", error);
  if (msg == NULL) return "";
  else             return string(msg);
}

double
GRBCallback::getSolution(GRBVar v)
{
  if (x == NULL) {
    x = new double[cols];
    int error;
    if (where == GRB_CB_MIPSOL) {
      error = GRBcbget(cbdata, where, GRB_CB_MIPSOL_SOL, x);
    } else {
      error = GRBcbget(cbdata, where, GRB_CB_MULTIOBJ_SOL, x);
    }
    if (error) throw GRBException("getSolution", error);
  }

  int k = v.getcolno();
  if (k < 0 || k >= cols) {
    throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  }
  return x[k];
}

double*
GRBCallback::getSolution(const GRBVar* xvars, int len)
{
  if (len <= 0) return NULL;
  if (xvars == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  if (x == NULL) {
    x = new double[cols];
    int error;
    if (where == GRB_CB_MIPSOL) {
      error = GRBcbget(cbdata, where, GRB_CB_MIPSOL_SOL, x);
    } else {
      error = GRBcbget(cbdata, where, GRB_CB_MULTIOBJ_SOL, x);
    }
    if (error) throw GRBException("getSolution", error);
  }

  double *sol = new double[len];
  for (int i = 0; i < len; i++) {
    int k = xvars[i].getcolno();
    if (k < 0 || k >= cols) {
      delete[] sol;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    sol[i] = x[k];
  }

  return sol;
}

double
GRBCallback::getNodeRel(GRBVar v)
{
  if (relx == NULL) {
    relx = new double[cols];
    int error = GRBcbget(cbdata, where, GRB_CB_MIPNODE_REL, relx);
    if (error) throw GRBException("getNodeRel", error);
  }

  int k = v.getcolno();
  if (k < 0 || k >= cols) {
    throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  }
  return relx[k];
}

double*
GRBCallback::getNodeRel(const GRBVar* xvars, int len)
{
  if (len <= 0) return NULL;
  if (xvars == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  if (relx == NULL) {
    relx = new double[cols];
    int error = GRBcbget(cbdata, where, GRB_CB_MIPNODE_REL, relx);
    if (error) throw GRBException("getNodeRel", error);
  }

  double *sol = new double[len];
  for (int i = 0; i < len; i++) {
    int k = xvars[i].getcolno();
    if (k < 0 || k >= cols) {
      delete[] sol;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    sol[i] = relx[k];
  }

  return sol;
}

void
GRBCallback::setSolution(GRBVar v, double val)
{
  if (newx == NULL) {
    newx = new double[cols];
    for (int j = 0; j < cols; j++) newx[j] = GRB_UNDEFINED;
  }

  int k = v.getcolno();
  if (k < 0 || k >= cols) {
    throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  }
  newx[k] = val;
}

void
GRBCallback::setSolution(const GRBVar* xvars, const double* sol, int len)
{
  if (len <= 0) return;
  if (xvars == NULL || sol == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  if (newx == NULL) {
    newx = new double[cols];
    for (int j = 0; j < cols; j++) newx[j] = GRB_UNDEFINED;
  }

  for (int i = 0; i < len; i++) {
    int k = xvars[i].getcolno();
    if (k < 0 || k >= cols) {
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    newx[k] = sol[i];
  }
}

double
GRBCallback::useSolution()
{
  double obj = GRB_INFINITY;
  if (newx != NULL) {
    double* tmpx = newx;
    newx = NULL;
    int error = GRBcbsolution(cbdata, tmpx, &obj);
    delete[] tmpx;
    if (error) throw GRBException("setSolution", error);
  }
  return obj;
}

void
GRBCallback::addCut(const GRBTempConstr& tc)
{
  if (tc.expr.size() > 0)
    throw GRBException("AddCut is only for linear constraints",
                       GRB_ERROR_INVALID_ARGUMENT);
  addCutOrLazy(tc.expr.getLinExpr(), tc.sense, 0.0, true);
}

void
GRBCallback::addCut(const GRBLinExpr& expr,
                    char              sense,
                    double            rhs)
{
  addCutOrLazy(expr, sense, rhs, true);
}

void
GRBCallback::addLazy(const GRBTempConstr& tc)
{
  if (tc.expr.size() > 0)
    throw GRBException("AddLazy is only for linear constraints",
                       GRB_ERROR_INVALID_ARGUMENT);
  addCutOrLazy(tc.expr.getLinExpr(), tc.sense, 0.0, false);
}

void
GRBCallback::addLazy(const GRBLinExpr& expr,
                     char              sense,
                     double            rhs)
{
  addCutOrLazy(expr, sense, rhs, false);
}

void
GRBCallback::addCutOrLazy(const GRBLinExpr& expr,
                          char              sense,
                          double            rhs,
                          bool              isCut)
{
  int    error = 0;
  int    nz   = (int) expr.size();
  double xrhs = rhs - expr.getConstant();
  int    *ind = new int[nz];
  double *val = new double[nz];
  int    i, k;

  for (i = 0; i < nz; i++) {
    GRBVar v = expr.getVar(i);
    k = v.getcolno();
    if (k < 0 || k >= cols) {
      delete[] ind;
      delete[] val;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
  }

  nz = 0;
  for (i = 0; i < (int) expr.size(); i++) {
    GRBVar v = expr.getVar(i);
    k = v.getcolno();
    if (k < 0) {
      delete[] ind;
      delete[] val;
      throw GRBException("Internal Error", GRB_ERROR_INTERNAL);
    }
    ind[nz] = k;
    val[nz] = expr.getCoeff(i);
    nz++;
  }

  // handle duplicates and zeros

  GRBclean2(&nz, ind, val);

  if (isCut) {
    error = GRBcbcut(cbdata, nz, ind, val, sense, xrhs);
  if (error) throw GRBException("addCut", error);
  } else {
    error = GRBcblazy(cbdata, nz, ind, val, sense, xrhs);
  if (error) throw GRBException("addLazy", error);
  }

  if (ind != NULL) delete[] ind;
  if (val != NULL) delete[] val;

}

void
GRBCallback::abort()
{
  GRBterminate(Cmodel);
}

void
GRBCallback::stopOneMultiObj(int objnum)
{
  int error = GRBcbstoponemultiobj(Cmodel, cbdata, objnum);
  if (error) throw GRBException("stopOneMultiObj", error);
}
