// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include <string.h>
#include <assert.h>
#include <cstdlib>
#include "Common.h"
#include "attrprivate.h"
#include "parprivate.h"

#define RANGEBD 1E+25

GRBModel::GRBModel()
{
  Cenv          = NULL;
  cb            = NULL;
  cols          = 0;
  rows          = 0;
  numsos        = 0;
  numqconstrs   = 0;
  numgenconstrs = 0;
  newranges     = 0;
  updatemode    = -1;
}

GRBModel::GRBModel(const GRBEnv& env)
{
  Cenv = env.env;

  int error = GRBnewmodel(Cenv, &Cmodel, NULL, 0, NULL, NULL,
                          NULL, NULL, NULL);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  // environment gets copied in GRBnewmodel
  Cenv = GRBgetenv(Cmodel);

  populate(true);
}

GRBModel::GRBModel(const GRBEnv& env,
                   const string& filename)
{
  Cenv = env.env;

  int error = GRBreadmodel(Cenv, filename.c_str(), &Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  // environment gets copied in GRBreadmodel
  Cenv = GRBgetenv(Cmodel);

  populate();
}

GRBModel::GRBModel(const GRBModel& xmodel)
{
  Cenv   = xmodel.Cenv;
  Cmodel = GRBcopymodel(xmodel.Cmodel);
  if (Cmodel == NULL) throw
    GRBException("Unable to create a model copy",
                 GRB_ERROR_FAILED_TO_CREATE_MODEL);

  // environment gets copied in GRBcopymodel
  Cenv = GRBgetenv(Cmodel);

  populate();

  cb = xmodel.cb;
}

GRBModel::~GRBModel()
{
  unsigned int i;

  for (i = 0; i < vars.size(); i++)
    if (vars[i].varRep) delete vars[i].varRep;
  for (i = 0; i < constrs.size(); i++)
    if (constrs[i].conRep) delete constrs[i].conRep;
  for (i = 0; i < sos.size(); i++)
    if (sos[i].sosRep) delete sos[i].sosRep;
  for (i = 0; i < qconstrs.size(); i++)
    if (qconstrs[i].qconRep) delete qconstrs[i].qconRep;
  for (i = 0; i < genconstrs.size(); i++)
    if (genconstrs[i].genconRep) delete genconstrs[i].genconRep;

  GRBfreemodel(Cmodel);
}

void
GRBModel::read(const string& filename)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int error = GRBread(Cmodel, filename.c_str());
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::write(const string& filename)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  if (GRBismodelfile(filename.c_str()) ||
      GRBisattrfile(filename.c_str())    )
    update();
  int error = GRBwrite(Cmodel, filename.c_str());
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::sync()
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int error = GRBsync(Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

GRBModel
GRBModel::relax()
{
  GRBModel xmodel = GRBModel();
  int error = GRBrelaxmodel(Cmodel, &xmodel.Cmodel);
  if (error) throw
    GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (xmodel.Cmodel == NULL) throw
    GRBException("Unable to create relaxation",
                 GRB_ERROR_FAILED_TO_CREATE_MODEL);

  xmodel.Cenv = GRBgetenv(xmodel.Cmodel);

  xmodel.populate();

  return xmodel;
}

GRBModel
GRBModel::fixedModel()
{
  GRBModel xmodel = GRBModel();
  int error = GRBfixmodel(Cmodel, &xmodel.Cmodel);
  if (error) throw
    GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (xmodel.Cmodel == NULL) throw
    GRBException("Unable to create fixed model",
                 GRB_ERROR_FAILED_TO_CREATE_MODEL);

  xmodel.Cenv = GRBgetenv(xmodel.Cmodel);

  xmodel.populate();

  return xmodel;
}

GRBModel
GRBModel::presolve()
{
  update();

  GRBModel xmodel = GRBModel();
  int error = GRBpresolvemodel(Cmodel, &xmodel.Cmodel);
  if (error) throw
    GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (xmodel.Cmodel == NULL) throw
    GRBException("Unable to create presolved model",
                 GRB_ERROR_FAILED_TO_CREATE_MODEL);

  xmodel.Cenv = GRBgetenv(xmodel.Cmodel);

  xmodel.populate();

  return xmodel;
}

GRBModel
GRBModel::feasibility()
{
  GRBModel xmodel = GRBModel();
  int error = GRBfeasibility(Cmodel, &xmodel.Cmodel);
  if (error) throw
    GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (xmodel.Cmodel == NULL) throw
    GRBException("Unable to create feasibility model",
                 GRB_ERROR_FAILED_TO_CREATE_MODEL);

  xmodel.Cenv = GRBgetenv(xmodel.Cmodel);

  xmodel.populate();

  return xmodel;
}

GRBModel
GRBModel::linearize()
{
  GRBModel xmodel = GRBModel();
  int error = GRBlinearizemodel(Cmodel, &xmodel.Cmodel);
  if (error) throw
    GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (xmodel.Cmodel == NULL) throw
    GRBException("Unable to create linearization",
                 GRB_ERROR_FAILED_TO_CREATE_MODEL);

  xmodel.Cenv = GRBgetenv(xmodel.Cmodel);

  xmodel.populate();

  return xmodel;
}

GRBModel
GRBModel::singleScenarioModel()
{
  GRBModel xmodel = GRBModel();
  int error = GRBsinglescenariomodel(Cmodel, &xmodel.Cmodel);
  if (error) throw
    GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (xmodel.Cmodel == NULL) throw
    GRBException("Unable to create single scenario model",
                 GRB_ERROR_FAILED_TO_CREATE_MODEL);

  xmodel.Cenv = GRBgetenv(xmodel.Cmodel);

  xmodel.populate();

  return xmodel;
}

double
GRBModel::feasRelax(int type, bool minrelax, bool vrelax, bool crelax)
{
  update();

  if (type < 0 || type > 2)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  // nothing to relax

  int vlen = - ((int) vrelax);
  int clen = - ((int) crelax);

  if (vlen == 0 && clen == 0) return 0;

  return feasRelaxP(type, minrelax, vlen, clen, NULL, NULL, NULL,
                    NULL, NULL);
}

double
GRBModel::feasRelax(int              type,
                    bool              minrelax,
                    int              vlen,
                    const GRBVar*    xvars,
                    const double*    lbpen,
                    const double*    ubpen,
                    int              clen,
                    const GRBConstr* xconstrs,
                    const double*    rhspen)
{
  update();

  if (type     < 0 || type     > 2   ||
      vlen     < 0 || vlen     > cols ||
      clen     < 0 || clen     > rows   )
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  return feasRelaxP(type, minrelax, vlen, clen, xvars, lbpen, ubpen,
                    xconstrs, rhspen);
}

double
GRBModel::feasRelaxP(int              type,
                     bool             xminrelax,
                     int              vlen,
                     int              clen,
                     const GRBVar*    xvars,
                     const double*    lbpen,
                     const double*    ubpen,
                     const GRBConstr* xconstrs,
                     const double*    rhspen)
{
  int     minrelax = (int) xminrelax;
  double* plb  = NULL;
  double* pub  = NULL;
  double* prhs = NULL;
  int i, j, k;

  if (vlen != 0) {
    plb = new double[cols];
    pub = new double[cols];

    if (vlen > 0) {
      for (j = 0; j < cols; j++) {
        plb[j] = GRB_INFINITY;
        pub[j] = GRB_INFINITY;
      }
    } else {
      for (j = 0; j < cols; j++) {
        plb[j] = 1.0;
        pub[j] = 1.0;
      }
    }

    for (k = 0; k < vlen; k++) {
      j = xvars[k].getcolno();
      if (j < 0 || j >= cols)
        throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
      plb[j] = lbpen[k];
      pub[j] = ubpen[k];
    }
  }

  if (clen != 0) {
    prhs = new double[rows];

    if (clen > 0) {
      for (i = 0; i < rows; i++) {
        prhs[i] = GRB_INFINITY;
      }
    } else {
      for (i = 0; i < rows; i++) {
        prhs[i] = 1.0;
      }
    }

    for (k = 0; k < clen; k++) {
      i = xconstrs[k].getrowno();
      if (i < 0 || i >= rows)
        throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
      prhs[i] = rhspen[k];
    }
  }

  double feasobj;
  int error = GRBfeasrelax(Cmodel, type, minrelax, plb, pub, prhs, &feasobj);
  if (plb) delete[] plb;
  if (pub) delete[] pub;
  if (prhs) delete[] prhs;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int xcols          = get(GRB_IntAttr_NumVars);
  int xrows          = get(GRB_IntAttr_NumConstrs);
  int xnumqconstrs   = get(GRB_IntAttr_NumQConstrs);
  int xnumgenconstrs = get(GRB_IntAttr_NumGenConstrs);

  for (j = cols; j < xcols; j++)
    vars.push_back(GRBVar(Cmodel, j));

  for (i = rows; i < xrows; i++)
    constrs.push_back(GRBConstr(Cmodel, i));

  for (i = numqconstrs; i < xnumqconstrs; i++)
    qconstrs.push_back(GRBQConstr(Cmodel, i));

  for (i = numgenconstrs; i < xnumgenconstrs; i++)
    genconstrs.push_back(GRBGenConstr(Cmodel, i));

  cols = xcols;
  rows = xrows;
  numqconstrs = xnumqconstrs;
  numgenconstrs = xnumgenconstrs;

  return feasobj;
}

int
GRBModel::getupdmode()
{
  if (updatemode == -1) {
    int error = GRBgetintparam(GRBgetenv(Cmodel), "UpdateMode",
                               &updatemode);
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }
  return updatemode;
}

void
GRBModel::populate(bool emptyModel)
{
  int i, j;

  cb = NULL;

  if (emptyModel) {
    cols = rows = numsos = numqconstrs = numgenconstrs = 0;
  } else {
    cols          = get(GRB_IntAttr_NumVars);
    rows          = get(GRB_IntAttr_NumConstrs);
    numsos        = get(GRB_IntAttr_NumSOS);
    numqconstrs   = get(GRB_IntAttr_NumQConstrs);
    numgenconstrs = get(GRB_IntAttr_NumGenConstrs);
  }

  newranges   = 0;
  updatemode  = -1;

  for (j = 0; j < cols; j++)
    vars.push_back(GRBVar(Cmodel, j));

  for (i = 0; i < rows; i++)
    constrs.push_back(GRBConstr(Cmodel, i));

  for (i = 0; i < numsos; i++)
    sos.push_back(GRBSOS(Cmodel, i));

  for (i = 0; i < numqconstrs; i++)
    qconstrs.push_back(GRBQConstr(Cmodel, i));

  for (i = 0; i < numgenconstrs; i++)
    genconstrs.push_back(GRBGenConstr(Cmodel, i));
}

void
GRBModel::update()
{
  if (Cmodel == NULL) return;

  int error = 0;
  unsigned int i;
  int j;

  int delcols = 0;
  int delvars = 0;
  for (i = 0; i < vars.size(); i++) {
    GRBVar v = vars[i];
    j = v.getcolno();
    if (j <= -2) delvars++;
    if (j <  -2) delcols++;
  }

  if (delvars > 0) {
    vector<GRBVar> tmpvars;
    i = 1;
    if (delcols > 0) i = delcols;
    int *cdelind = new int[i];
    delcols = 0;
    for (i = 0; i < vars.size(); i++) {
      GRBVar v = vars[i];
      j = v.getcolno();
      if (j <  -2) cdelind[delcols++] = -3 -j;
      if (j >= -1) tmpvars.push_back(v);
      else if (v.varRep) delete v.varRep;
    }

    if (delcols > 0) {
      error = GRBdelvars(Cmodel, delcols, cdelind);
    }
    delete[] cdelind;
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    vars.clear();
    vars = tmpvars;
  }

  int delrows = 0;
  int delcons = 0;
  for (i = 0; i < constrs.size(); i++) {
    GRBConstr c = constrs[i];
    j = c.getrowno();
    if (j <= -2) delcons++;
    if (j <  -2) delrows++;
  }

  if (delcons > 0) {
    vector<GRBConstr> tmpconstrs;
    i = 1;
    if (delrows > 0) i = delrows;
    int* rdelind = new int[i];
    delrows = 0;
    for (i = 0; i < constrs.size(); i++) {
      GRBConstr c = constrs[i];
      j = c.getrowno();
      if (j <  -2) rdelind[delrows++] = -3 - j;
      if (j >= -1) tmpconstrs.push_back(c);
      else if (c.conRep) delete c.conRep;
    }

    if (delrows > 0) {
      error = GRBdelconstrs(Cmodel, delrows, rdelind);
    }
    delete[] rdelind;
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    constrs.clear();
    constrs = tmpconstrs;
  }

  int delsos  = 0;
  delrows = 0;
  for (i = 0; i < sos.size(); i++) {
    GRBSOS c = sos[i];
    j = c.getindex();
    if (j <= -2) delsos++;
    if (j <  -2) delrows++;
  }

  if (delsos > 0) {
    vector<GRBSOS> tmpsos;
    i = 1;
    if (delrows > 0) i = delrows;
    int* sdelind = new int[i];
    delrows = 0;
    for (i = 0; i < sos.size(); i++) {
      GRBSOS c = (GRBSOS) sos[i];
      j = c.getindex();
      if (j <  -2) sdelind[delrows++] = -3 - j;
      if (j >= -1) tmpsos.push_back(c);
      else if (c.sosRep) delete c.sosRep;
    }

    if (delrows > 0) {
      error = GRBdelsos(Cmodel, delrows, sdelind);
    }
    delete[] sdelind;
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    sos.clear();
    sos = tmpsos;
  }

  int delqconstrs = 0;
  delrows = 0;
  for (i = 0; i < qconstrs.size(); i++) {
    GRBQConstr qc = qconstrs[i];
    j = qc.getindex();
    if (j <= -2) delqconstrs++;
    if (j <  -2) delrows++;
  }

  if (delqconstrs > 0) {
    vector<GRBQConstr> tmpqconstrs;
    i = 1;
    if (delrows > 0) i = delrows;
    int* sdelind = new int[i];
    delrows = 0;
    for (i = 0; i < qconstrs.size(); i++) {
      GRBQConstr qc = (GRBQConstr) qconstrs[i];
      j = qc.getindex();
      if (j <  -2) sdelind[delrows++] = -3 - j;
      if (j >= -1) tmpqconstrs.push_back(qc);
      else if (qc.qconRep) delete qc.qconRep;
    }

    if (delrows > 0) {
      error = GRBdelqconstrs(Cmodel, delrows, sdelind);
    }
    delete[] sdelind;
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    qconstrs.clear();
    qconstrs = tmpqconstrs;
  }

  int delgenconstrs = 0;
  delrows = 0;
  for (i = 0; i < genconstrs.size(); i++) {
    GRBGenConstr genc = genconstrs[i];
    j = genc.getindex();
    if (j <= -2) delgenconstrs++;
    if (j <  -2) delrows++;
  }

  if (delgenconstrs > 0) {
    vector<GRBGenConstr> tmpgenconstrs;
    i = 1;
    if (delrows > 0) i = delrows;
    int* sdelind = new int[i];
    delrows = 0;
    for (i = 0; i < genconstrs.size(); i++) {
      GRBGenConstr genc = (GRBGenConstr) genconstrs[i];
      j = genc.getindex();
      if (j <  -2) sdelind[delrows++] = -3 - j;
      if (j >= -1) tmpgenconstrs.push_back(genc);
      else if (genc.genconRep) delete genc.genconRep;
    }

    if (delrows > 0) {
      error = GRBdelgenconstrs(Cmodel, delrows, sdelind);
    }
    delete[] sdelind;
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    genconstrs.clear();
    genconstrs = tmpgenconstrs;
  }

  for (j = 0; j < newranges; j++)
    vars.push_back(GRBVar(Cmodel, -1));

  cols = vars.size();
  for (j = 0; j < cols; j++) vars[j].setcolno(j);

  rows = constrs.size();
  for (j = 0; j < rows; j++) constrs[j].setrowno(j);

  numsos = sos.size();
  for (j = 0; j < numsos; j++) sos[j].setindex(j);

  numqconstrs = qconstrs.size();
  for (j = 0; j < numqconstrs; j++) qconstrs[j].setindex(j);

  numgenconstrs = genconstrs.size();
  for (j = 0; j < numgenconstrs; j++) genconstrs[j].setindex(j);

  error = GRBupdatemodel(Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (cb != NULL) cb->cols = cols;

  newranges = 0;

  // reset to -1, so it is OK for users to change update mode after update
  updatemode = -1;

  assert(get(GRB_IntAttr_NumVars) == cols);
}

void
GRBModel::optimize()
{
  update();

  int error = GRBoptimize(Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

string
GRBModel::optimizeBatch()
{
  update();

  char batchID[GRB_MAX_STRLEN];
  int error = GRBoptimizebatch(Cmodel, batchID);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  return string(batchID);
}

void
GRBModel::optimizeasync()
{
  update();

  int error = GRBoptimizeasync(Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::computeIIS()
{
  update();

  int error = GRBcomputeIIS(Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::tune()
{
  update();

  int error = GRBtunemodel(Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::reset(int clearall /* default value = 0 */)
{
  int error = GRBreset(Cmodel, clearall);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::check()
{
  int error = GRBcheckmodel(Cmodel);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::terminate()
{
  GRBterminate(Cmodel);
}

void
GRBModel::getTuneResult(int i)
{
  int error = GRBgettuneresult(Cmodel, i);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

GRBQuadExpr
GRBModel::getObjective() const
{
  double objcon;
  int    error;

  error = GRBgetdblattr(Cmodel, "ObjCon", &objcon);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  double* obj = new double[cols];
  error = GRBgetdblattrarray(Cmodel, "Obj", 0, cols, obj);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  GRBLinExpr le = GRBLinExpr(objcon);
  for (int i = 0; i < cols; i++)
    if (obj[i] !=0.0)
      le += obj[i] * vars[i];

  delete[] obj;

  int qnz;
  error = GRBgetintattr(Cmodel, "NumQNZs", &qnz);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (qnz > 0) {

    // Extract quadratic objective

    GRBQuadExpr qe = GRBQuadExpr(le);
    int*    qrow = new int[qnz];
    int*    qcol = new int[qnz];
    double* qval = new double[qnz];

    error = GRBgetq(Cmodel, &qnz, qrow, qcol, qval);
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

    for (int i = 0; i < qnz; i++)
      qe.addTerm(qval[i], vars[qrow[i]], vars[qcol[i]]);

    delete[] qrow;
    delete[] qcol;
    delete[] qval;

    return qe;
  } else {
    return GRBQuadExpr(le);
  }
}

GRBLinExpr
GRBModel::getObjective(int index) const
{
  int    objnumber = -1;
  double objcon;
  int    error;

  error = GRBgetintparam(Cenv, "ObjNumber", &objnumber);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  error = GRBsetintparam(Cenv, "ObjNumber", index);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  error = GRBgetdblattr(Cmodel, "ObjNCon", &objcon);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  double* obj = new double[cols];
  error = GRBgetdblattrarray(Cmodel, "ObjN", 0, cols, obj);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  error = GRBsetintparam(Cenv, "ObjNumber", objnumber);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  GRBLinExpr le = GRBLinExpr(objcon);
  for (int i = 0; i < cols; i++)
    if (obj[i] != 0.0)
      le += obj[i] * vars[i];

  delete[] obj;

  if (index == 0) {
    int qnz;
    error = GRBgetintattr(Cmodel, "NumQNZs", &qnz);
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

    if (qnz > 0) throw GRBException("Objective is quadratic",
                                    GRB_ERROR_DATA_NOT_AVAILABLE);
  }

  return le;
}

int
GRBModel::getPWLObj(GRBVar v, double *ptval, double *ptobj) const
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  if (v.getcolno() < 0 || v.getcolno() >= cols) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int pts;

  int error = GRBgetpwlobj(Cmodel, v.getcolno(), &pts, NULL, NULL);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (ptval == NULL || ptobj == NULL) return pts;

  error = GRBgetpwlobj(Cmodel, v.getcolno(), &pts, ptval, ptobj);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  return pts;
}

void
GRBModel::setObjectiveN(GRBLinExpr obje,
                        int        index,
                        int        priority,
                        double     weight,
                        double     abstol,
                        double     reltol,
                        string     name)
{
  int     len = obje.size();
  int    *ind = new int[len];
  double *val = new double[len];

  double objcon = obje.getConstant();
  for (int i = 0; i < len; i++) {
    ind[i] = obje.getVar(i).getcolno();
    val[i] = obje.getCoeff(i);
    if (ind[i] < 0)
      throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  }

  int error = GRBsetobjectiven(Cmodel, index, priority, weight, abstol,
                               reltol, (char *) name.c_str(), objcon,
                               len, ind, val);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  delete[] ind;
  delete[] val;
}

void
GRBModel::setObjective(GRBLinExpr obje, int sense)
{
  setObjective(GRBQuadExpr(obje), sense);
}

void
GRBModel::setObjective(GRBQuadExpr obje, int sense)
{
  int i, tcols;
  int error;
  GRBLinExpr le = obje.getLinExpr();;

  GRBdelq(Cmodel);

  if (getupdmode()) tcols = vars.size();
  else              tcols = cols;
  double *obj = new double[tcols];
  for (i = 0; i < tcols; i++) obj[i] = 0.0;
  for (i = 0; i < (int) le.size(); i++) {
    int j = le.getVar(i).getcolno();
    if (j < 0)
      throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
    obj[j] += le.getCoeff(i);
  }
  error = GRBsetdblattrarray(Cmodel, "Obj", 0, tcols, obj);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  delete[] obj;

  error = GRBsetdblattr(Cmodel, "ObjCon", le.getConstant());
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (sense != 0)
    error = GRBsetintattr(Cmodel, "ModelSense", sense);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int qnz = (int) obje.size();

  if (qnz > 0) {
    int*    qrow = new int[qnz];
    int*    qcol = new int[qnz];
    double* qval = new double[qnz];

    for (int i = 0; i < qnz; i++) {
      qrow[i] = obje.getVar1(i).getcolno();
      qcol[i] = obje.getVar2(i).getcolno();
      if (qrow[i] < 0 || qcol[i] < 0)
        throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
      qval[i] = obje.getCoeff(i);
    }

    error = GRBaddqpterms(Cmodel, qnz, qrow, qcol, qval);
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

    delete[] qrow;
    delete[] qcol;
    delete[] qval;
  }
}

void
GRBModel::setPWLObj(GRBVar v, int points, double *ptval, double *ptobj)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  if (v.getcolno() < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBsetpwlobj(Cmodel, v.getcolno(), points, ptval, ptobj);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

GRBVar
GRBModel::getVar(int i) const
{
  if (i < 0 || i >= cols)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  return vars[i];
}

GRBVar*
GRBModel::getVars() const
{
  if (cols <= 0) return NULL;
  GRBVar* tvars = new GRBVar[cols];
  for (int i = 0; i < cols; i++) {
    tvars[i] = vars[i];
  }
  return tvars;
}

GRBVar
GRBModel::getVarByName(const string& name)
{
  char *cname = (char *) name.c_str();
  int   index = -1;

  int error = GRBgetvarbyname(Cmodel, cname, &index);

  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (index < 0)
    throw GRBException("No such variable", GRB_ERROR_INVALID_ARGUMENT);
  else
    return vars[index];
}

GRBConstr
GRBModel::getConstr(int i) const
{
  if (i < 0 || i >= rows)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  return constrs[i];
}

GRBConstr*
GRBModel::getConstrs() const
{
  if (rows <= 0) return NULL;
  GRBConstr* tconstrs = new GRBConstr[rows];
  for (int i = 0; i < rows; i++) tconstrs[i] = constrs[i];
  return tconstrs;
}

GRBConstr
GRBModel::getConstrByName(const string& name)
{
  char *cname = (char *) name.c_str();
  int   index = -1;

  int error = GRBgetconstrbyname(Cmodel, cname, &index);

  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  else if (index < 0)
    throw GRBException("No such constraint", GRB_ERROR_INVALID_ARGUMENT);
  else
    return constrs[index];
}

GRBSOS*
GRBModel::getSOSs() const
{
  if (numsos <= 0) return NULL;
  GRBSOS* tsoss = new GRBSOS[numsos];
  for (int i = 0; i < numsos; i++) tsoss[i] = sos[i];
  return tsoss;
}

GRBQConstr*
GRBModel::getQConstrs() const
{
  if (numqconstrs <= 0) return NULL;
  GRBQConstr* tqconstrs = new GRBQConstr[numqconstrs];
  for (int i = 0; i < numqconstrs; i++) tqconstrs[i] = qconstrs[i];
  return tqconstrs;
}

GRBGenConstr*
GRBModel::getGenConstrs() const
{
  if (numgenconstrs <= 0) return NULL;
  GRBGenConstr* tgenconstrs = new GRBGenConstr[numgenconstrs];
  for (int i = 0; i < numgenconstrs; i++) tgenconstrs[i] = genconstrs[i];
  return tgenconstrs;
}

GRBVar
GRBModel::addVar(double  lb,
                 double  ub,
                 double  obj,
                 char    vtype,
                 string  varname)
{
  int   zero = 0;
  char *name = (char *) varname.c_str();

  int error = GRBaddvars(Cmodel, 1, 0, &zero, NULL, NULL, &obj, &lb,
                         &ub, &vtype, &name);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int col = -1;
  if (getupdmode()) col = vars.size();

  vars.push_back(GRBVar(Cmodel, col));

  return vars.back();
}

GRBVar
GRBModel::addVar(double           lb,
                 double           ub,
                 double           obj,
                 char             vtype,
                 int              nonzeros,
                 const GRBConstr* xconstrs,
                 const double*    coeffs,
                 string           varname)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int     error = 0;
  int     zero = 0;
  char*   name = (char *) varname.c_str();
  double* xcoeffs = (double*) coeffs;
  int*    ind  = NULL;

  if (nonzeros > 0) {
    ind = new int[nonzeros];
    int i = 0;
    int l = 0;
    for (i = 0; i < nonzeros; i++) {
      int j = xconstrs[i].getrowno();
      if (j < 0) break;
      ind[l++] = j;
    }
    if (i < nonzeros) {
      delete[] ind;
      throw GRBException("Constraint isn't in the model",
                          GRB_ERROR_NOT_IN_MODEL);
    }
    if (xcoeffs == NULL) {
      xcoeffs = new double[nonzeros];
      for (int i = 0; i < nonzeros; i++) xcoeffs[i] = 1.0;
    }
  }

  error = GRBaddvars(Cmodel, 1, nonzeros, &zero, ind,
                     xcoeffs, &obj, &lb, &ub,
                     &vtype, &name);
  if (ind != NULL) delete[] ind;
  if (coeffs == NULL && xcoeffs != NULL) delete[] xcoeffs;
  if (error) GRBException(string(GRBgeterrormsg(Cenv)), error);

  int col = -1;
  if (getupdmode()) col = vars.size();

  vars.push_back(GRBVar(Cmodel, col));

  return vars.back();
}

GRBVar
GRBModel::addVar(double           lb,
                 double           ub,
                 double           obj,
                 char             vtype,
                 const GRBColumn& col,
                 string           varname)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int     error = 0;
  int     nonzeros = col.size();
  int     zero = 0;
  char*   name = (char *) varname.c_str();
  int*    ind  = NULL;
  double* val  = NULL;

  if (nonzeros > 0) {
    ind = new int[nonzeros];
    val = new double[nonzeros];
    int l = 0;
    int i;
    for (i = 0; i < nonzeros; i++) {
      int j = col.getConstr(i).getrowno();
      if (j < 0) break;
      ind[l] = j;
      val[l++] = col.getCoeff(i);
    }
    if (i < nonzeros) {
      delete[] ind;
      delete[] val;
      throw GRBException("Constraint isn't in the model",
                          GRB_ERROR_NOT_IN_MODEL);
    }
  }

  error = GRBaddvars(Cmodel, 1, nonzeros, &zero, ind, val,
                     &obj, &lb, &ub, &vtype, &name);
  if (ind != NULL) delete[] ind;
  if (val != NULL) delete[] val;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int thiscol = -1;
  if (getupdmode()) thiscol = vars.size();

  vars.push_back(GRBVar(Cmodel, thiscol));

  return vars.back();
}

GRBVar*
GRBModel::addVars(int  cnt,
                  char type)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  char* types = NULL;
  if (type != GRB_CONTINUOUS) {
    types = new char[cnt];
    for (int i = 0; i < cnt; i++) types[i] = type;
  }
  int error = GRBaddvars(Cmodel, cnt, 0, NULL, NULL, NULL, NULL,
                         NULL, NULL, types, NULL);
  if (types != NULL) delete[] types;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  GRBVar* tvars = new GRBVar[cnt];
  if (getupdmode()) {
    int col = vars.size();
    for (int i = 0; i < cnt; i++) {
      tvars[i] = GRBVar(Cmodel, col);
      col++;
      vars.push_back(tvars[i]);
    }
  } else {
    for (int i = 0; i < cnt; i++) {
      tvars[i] = GRBVar(Cmodel, -1);
      vars.push_back(tvars[i]);
    }
  }

  return tvars;
}

GRBVar*
GRBModel::addVars(const double* lb,
                  const double* ub,
                  const double* obj,
                  const char*   type,
                  const string* name,
                  int           len)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  if (len <= 0) return NULL;

  int error = 0;
  char**  xname = NULL;

  if (name != NULL) {
    xname = new char*[len];
    for (int i = 0; i < len; i++) {
      xname[i] = (char *) name[i].c_str();
    }
  }

  error = GRBaddvars(Cmodel, len, 0, NULL, NULL, NULL, (double*) obj,
                     (double*) lb, (double*) ub, (char*) type, xname);
  if (xname != NULL) delete[] xname;

  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  GRBVar* tvars = new GRBVar[len];
  if (getupdmode()) {
    int col = vars.size();
    for (int i = 0; i < len; i++) {
      tvars[i] = GRBVar(Cmodel, col);
      col++;
      vars.push_back(tvars[i]);
    }
  } else {
    for (int i = 0; i < len; i++) {
      tvars[i] = GRBVar(Cmodel, -1);
      vars.push_back(tvars[i]);
    }
  }

  return tvars;
}

GRBVar*
GRBModel::addVars(const double*    lb,
                  const double*    ub,
                  const double*    obj,
                  const char*      type,
                  const string*    name,
                  const GRBColumn* col,
                  int              len)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  if (len <= 0) return NULL;

  int error = 0;
  int i, j, k;
  size_t   l;
  size_t   nonzeros = 0;
  char**   xname = NULL;
  size_t*  beg   = NULL;
  int*     ind   = NULL;
  double*  val   = NULL;

  if (col != NULL) {
    for (i = 0; i < len; i++) nonzeros += col[i].size();
  }
  if (nonzeros > 0) {
    beg = new size_t[len];
    ind = new int[nonzeros];
    val = new double[nonzeros];
    l = 0;
    for (k = 0; k < len; k++) {
      beg[k] = l;
      for (i = 0; i < (int) col[k].size(); i++) {
        j = col[k].getConstr(i).getrowno();
        if (j < 0) break;
        ind[l] = j;
        val[l++] = col[k].getCoeff(i);
      }
      if (i < (int) col[k].size()) break;
    }
    if (k < len) {
      delete[] beg;
      delete[] ind;
      delete[] val;
      throw GRBException("Constraint isn't in the model",
                          GRB_ERROR_NOT_IN_MODEL);
    }
  }

  if (name != NULL) {
    xname = new char*[len];
    for (int i = 0; i < len; i++) {
      xname[i] = (char*) name[i].c_str();
    }
  }

  error = GRBXaddvars(Cmodel, len, nonzeros, beg, ind, val, (double*) obj,
                      (double*) lb, (double*) ub, (char*) type, xname);
  if (xname != NULL) delete[] xname;
  if (beg   != NULL) delete[] beg;
  if (ind   != NULL) delete[] ind;
  if (val   != NULL) delete[] val;

  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  GRBVar* tvars = new GRBVar[len];
  if (getupdmode()) {
    int col = vars.size();
    for (i = 0; i < len; i++) {
      tvars[i] = GRBVar(Cmodel, col);
      col++;
      vars.push_back(tvars[i]);
    }
  } else {
    for (i = 0; i < len; i++) {
      tvars[i] = GRBVar(Cmodel, -1);
      vars.push_back(tvars[i]);
    }
  }

  return tvars;
}

GRBConstr
GRBModel::addConstr(const GRBLinExpr& expr1,
                    char              sense,
                    const GRBLinExpr& expr2,
                    string            name)
{
  GRBLinExpr ex = GRBLinExpr(expr1);
  ex -= expr2;
  return addConstr(ex, sense, -GRB_INFINITY, 0.0, name);
}

GRBConstr
GRBModel::addConstr(const GRBLinExpr& expr1,
                    char              sense,
                    GRBVar            v,
                    string            name)
{
  GRBLinExpr ex = GRBLinExpr(expr1);
  ex -= v;
  return addConstr(ex, sense, -GRB_INFINITY, 0.0, name);
}

GRBConstr
GRBModel::addConstr(GRBVar v1,
                    char   sense,
                    GRBVar v2,
                    string name)
{
  GRBLinExpr ex;
  ex += v1;
  ex -= v2;
  return addConstr(ex, sense, -GRB_INFINITY, 0.0, name);
}

GRBConstr
GRBModel::addConstr(GRBVar v1,
                    char   sense,
                    double rhs,
                    string name)
{
  GRBLinExpr ex;
  ex += v1;
  return addConstr(ex, sense, -GRB_INFINITY, rhs, name);
}

GRBConstr
GRBModel::addConstr(const GRBLinExpr&  expr,
                    char               sense,
                    double             rhs,
                    string             cname)
{
  return addConstr(expr, sense, -GRB_INFINITY, rhs, cname);
}

GRBConstr
GRBModel::addRange(const GRBLinExpr&  expr,
                   double             lhs,
                   double             rhs,
                   string             cname)
{
  if ((lhs <= -RANGEBD) == (rhs >= RANGEBD)) {
    newranges++;
  }
  double xlhs = lhs;
  double xrhs = rhs;
  if (xlhs < -RANGEBD) xlhs = -RANGEBD;
  if (xrhs >  RANGEBD) xrhs =  RANGEBD;
  return addConstr(expr, GRB_LESS_EQUAL, xlhs, xrhs, cname);
}

GRBConstr
GRBModel::addConstr(const GRBLinExpr&  expr,
                    char               sense,
                    double             lhs,
                    double             rhs,
                    const string&      cname)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int    error  = 0;
  int    i, k;
  int    zero   = 0;
  int    nz     = expr.size();
  double xrhs   = rhs - expr.getConstant();
  double xlhs   = lhs;
  char   *name  = (char *) cname.c_str();

  int    *ind = new int[nz];
  double *val = new double[nz];

  if (xlhs > -GRB_INFINITY) xlhs -= expr.getConstant();

  for (i = 0; i < (int) expr.size(); i++) {
    GRBVar v = expr.getVar(i);
    k = v.getcolno();
    if (k < 0) {
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

  if (lhs > -GRB_INFINITY)
    error = GRBaddrangeconstrs(Cmodel, 1, nz, &zero, ind, val,
                               &xlhs, &xrhs, &name);
  else
    error = GRBaddconstrs(Cmodel, 1, nz, &zero, ind, val,
                          &sense, &xrhs, &name);

  if (ind != NULL) delete[] ind;
  if (val != NULL) delete[] val;

  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int row = -1;
  if (getupdmode()) row = constrs.size();

  constrs.push_back(GRBConstr(Cmodel, row));

  return constrs.back();
}

GRBConstr
GRBModel::addConstr(const GRBTempConstr& tc,
                    string               cname)
{
  if (tc.expr.size() > 0)
    throw GRBException("AddConstr is for linear constraints; use AddQConstr to create a quadratic constraint", GRB_ERROR_INVALID_ARGUMENT);
  return addConstr(tc.expr.getLinExpr(), tc.sense, 0, cname);
}

GRBConstr*
GRBModel::addConstrs(int cnt)
{
  return addConstrs(NULL, NULL, NULL, NULL, NULL, cnt);
}

GRBConstr*
GRBModel::addConstrs(const GRBLinExpr* expr,
                     const char*       sense,
                     const double*     rhs,
                     const string*     name,
                     int               len)
{
  return addConstrs(expr, sense, NULL, rhs, name, len);
}

GRBConstr*
GRBModel::addRanges(const GRBLinExpr* expr,
                    const double*     lhs,
                    const double*     rhs,
                    const string*     name,
                    int               len)
{
  double* xlhs = new double[len];
  double* xrhs = new double[len];
  for (int i = 0; i < len; i++) {
    if (lhs == NULL || lhs[i] < -RANGEBD) xlhs[i] = -RANGEBD;
    else                                  xlhs[i] = lhs[i];
    if (rhs == NULL || rhs[i] >  RANGEBD) xrhs[i] = RANGEBD;
    else                                  xrhs[i] = rhs[i];
    if ((xlhs[i] <= -RANGEBD) == (xrhs[i] >= RANGEBD)) {
      newranges++;
    }
  }
  GRBConstr* cons = addConstrs(expr, NULL, xlhs, xrhs, name, len);
  delete[] xlhs;
  delete[] xrhs;
  return cons;
}

GRBConstr*
GRBModel::addConstrs(const GRBLinExpr* expr,
                     const char*       sense,
                     const double*     lhs,
                     const double*     rhs,
                     const string*     name,
                     int               len)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  if (len <= 0) return NULL;
  if (lhs != NULL && rhs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int    error   = 0;
  string errmsg;
  int    i, k, k1;
  size_t nz      = 0;
  int    maxsize = 0;

  if (expr != NULL) {
    for (k1 = 0; k1 < len; k1++) {
      if ((int)expr[k1].size() > maxsize) maxsize = expr[k1].size();
      nz += expr[k1].size();
      for (i = 0; i < (int)expr[k1].size(); i++) {
        GRBVar v = expr[k1].getVar(i);
        k = v.getcolno();
        if (k < 0) {
          throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
        }
      }
    }
  }

  char**  xname  = NULL;
  char*   xsense = new char[len];
  double* xrhs   = new double[len];
  size_t* beg    = NULL;
  int*    ind    = NULL;
  double* val    = NULL;

  if (nz > 0) {
    beg = new size_t[len];
    ind = new int[nz];
    val = new double[nz];
  }

  if (name != NULL) {
    xname = new char*[len];
    for (int i = 0; i < len; i++) {
      xname[i] = (char *) name[i].c_str();
    }
  }

  int l = 0;
  for (k = 0; k < len; k++) {
    if (sense != NULL) xsense[l] = sense[k];
    else               xsense[l] = GRB_LESS_EQUAL;
    if (rhs != NULL)   xrhs[l] = rhs[k];
    else               xrhs[l] = 0;
    l++;
  }

  if (expr != NULL)
    for (k = 0; k < len; k++) xrhs[k] -= expr[k].getConstant();

  if (nz > 0) {
    int*    tind = new int[maxsize];
    double* tval = new double[maxsize];
    size_t m = 0;
    for (k1 = 0; k1 < len; k1++) {
      int nz1 = 0;
      beg[k1] = m;
      if (expr[k1].size() == 0) continue;
      for (i = 0; i < (int) expr[k1].size(); i++) {
        GRBVar v = expr[k1].getVar(i);
        k = v.getcolno();
        if (k < 0) {
          delete[] tind;
          delete[] tval;
          delete[] xsense;
          delete[] xrhs;
          if (beg) delete[] beg;
          if (ind) delete[] ind;
          if (val) delete[] val;
          if (xname) delete[] xname;
          throw GRBException("Internal Error", GRB_ERROR_INTERNAL);
        }
        tind[nz1] = k;
        tval[nz1] = expr[k1].getCoeff(i);
        nz1++;
      }

      // handle duplicates and zeros

      GRBclean2(&nz1, tind, tval);

      for (i = 0; i < nz1; i++) {
        ind[m] = tind[i];
        val[m] = tval[i];
        m++;
      }
    }
    nz = m;

    delete[] tind;
    delete[] tval;
  }

  if (lhs != NULL) {
    double* xlhs = new double[len];
    for (int i = 0; i < len; i++) xlhs[i] = lhs[i] + xrhs[i] - rhs[i];
    error = GRBXaddrangeconstrs(Cmodel, len, nz, beg, ind, val,
                                xlhs, xrhs, xname);
    delete[] xlhs;
  } else {
    error = GRBXaddconstrs(Cmodel, len, nz, beg, ind, val,
                           xsense, xrhs, xname);
  }

  if (beg) delete[] beg;
  if (ind) delete[] ind;
  if (val) delete[] val;
  if (xname) delete[] xname;
  delete[] xsense;
  delete[] xrhs;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  GRBConstr* tconstrs = new GRBConstr[len];

  if (getupdmode()) {
    int row = constrs.size();
    for (i = 0; i < len; i++) {
      tconstrs[i] = GRBConstr(Cmodel, row);
      row++;
      constrs.push_back(tconstrs[i]);
    }
  } else {
    for (i = 0; i < len; i++) {
      tconstrs[i] = GRBConstr(Cmodel, -1);
      constrs.push_back(tconstrs[i]);
    }
  }

  return tconstrs;
}

GRBSOS
GRBModel::addSOS(const GRBVar* xvars,
                 const double* weight,
                 int           len,
                 int           type)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  if (xvars == NULL)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int* ind = new int[len];
  int l = 0;
  for (int i = 0; i < len; i++) {
    int j = xvars[i].getcolno();
    if (j < 0) throw
      GRBException("Variable not in the model", GRB_ERROR_NOT_IN_MODEL);
    ind[l++] = j;
  }

  int beg = 0;
  int error = GRBaddsos(Cmodel, 1, len, &type, &beg,
                        ind, (double*)weight);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  sos.push_back(GRBSOS(Cmodel, -1));

  return sos.back();
}

GRBQConstr
GRBModel::addQConstr(const GRBQuadExpr& expr1,
                     char               sense,
                     double             rhs,
                     string             cname)
{
  return addQConstr(expr1-rhs, sense, cname);
}

GRBQConstr
GRBModel::addQConstr(const GRBQuadExpr& expr1,
                     char               sense,
                     const GRBQuadExpr& expr2,
                     string             cname)
{
  return addQConstr(expr1-expr2, sense, cname);
}

GRBQConstr
GRBModel::addQConstr(const GRBTempConstr& constr,
                     string               cname)
{
  return addQConstr(constr.expr, constr.sense, cname);
}

GRBQConstr
GRBModel::addQConstr(const GRBQuadExpr& expr,
                     char               sense,
                     const string&      cname)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  GRBLinExpr linexpr = expr.getLinExpr();

  int    error = 0;
  char  *name  = (char *) cname.c_str();

  int     i;
  int     lnz  = linexpr.size();
  int    *lind = new int[lnz];
  double *lval = new double[lnz];
  int     qnz  = expr.size();
  int    *qrow = new int[qnz];
  int    *qcol = new int[qnz];
  double *qval = new double[qnz];

  for (i = 0; i < (int) linexpr.size(); i++) {
    GRBVar v = linexpr.getVar(i);
    if (v.getcolno() < 0) {
      delete[] lind;
      delete[] lval;
      delete[] qrow;
      delete[] qcol;
      delete[] qval;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    lind[i] = v.getcolno();
    lval[i] = linexpr.getCoeff(i);
  }
  GRBclean2(&lnz, lind, lval);

  for (i = 0; i < (int) expr.size(); i++) {
    GRBVar v1 = expr.getVar1(i);
    GRBVar v2 = expr.getVar2(i);
    if (v1.getcolno() < 0 || v2.getcolno() < 0) {
      delete[] lind;
      delete[] lval;
      delete[] qrow;
      delete[] qcol;
      delete[] qval;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    if (v1.getcolno() < v2.getcolno()) {
      qrow[i] = v1.getcolno();
      qcol[i] = v2.getcolno();
    } else {
      qcol[i] = v1.getcolno();
      qrow[i] = v2.getcolno();
    }
    qval[i] = expr.getCoeff(i);
  }
  GRBclean3(&qnz, qrow, qcol, qval);

  error = GRBaddqconstr(Cmodel, lnz, lind, lval, qnz, qrow, qcol, qval,
                        sense, -linexpr.getConstant(), name);

  if (lind != NULL) delete[] lind;
  if (lval != NULL) delete[] lval;
  if (qrow != NULL) delete[] qrow;
  if (qcol != NULL) delete[] qcol;
  if (qval != NULL) delete[] qval;

  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  qconstrs.push_back(GRBQConstr(Cmodel, -1));

  return qconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrMax(GRBVar        resvar,
                          const GRBVar* xvars,
                          int           len,
                          double        constant,
                          string        cname)
{
  const char *name = (char *) cname.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int resind = resvar.getcolno();
  if (resind < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  if (len > 0 && xvars == NULL)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int* ind = new int[len];
  int l = 0;
  for (int i = 0; i < len; i++) {
    int j = xvars[i].getcolno();
    if (j < 0) throw
      GRBException("Variable not in the model", GRB_ERROR_NOT_IN_MODEL);
    ind[l++] = j;
  }

  int error = GRBaddgenconstrMax(Cmodel, name, resind, len, ind, constant);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrMin(GRBVar        resvar,
                          const GRBVar* xvars,
                          int           len,
                          double        constant,
                          string        cname)
{
  const char *name = (char *) cname.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int resind = resvar.getcolno();
  if (resind < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  if (len > 0 && xvars == NULL)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int* ind = new int[len];
  int l = 0;
  for (int i = 0; i < len; i++) {
    int j = xvars[i].getcolno();
    if (j < 0) throw
      GRBException("Variable not in the model", GRB_ERROR_NOT_IN_MODEL);
    ind[l++] = j;
  }

  int error = GRBaddgenconstrMin(Cmodel, name, resind, len, ind, constant);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrAbs(GRBVar        resvar,
                          GRBVar        argvar,
                          string        cname)
{
  const char *name = (char *) cname.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int resind = resvar.getcolno();
  if (resind < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  int argind = argvar.getcolno();
  if (argind < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrAbs(Cmodel, name, resind, argind);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrAnd(GRBVar        resvar,
                          const GRBVar* xvars,
                          int           len,
                          string        cname)
{
  const char *name = (char *) cname.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int resind = resvar.getcolno();
  if (resind < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  if (len > 0 && xvars == NULL)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int* ind = new int[len];
  int l = 0;
  for (int i = 0; i < len; i++) {
    int j = xvars[i].getcolno();
    if (j < 0) throw
      GRBException("Variable not in the model", GRB_ERROR_NOT_IN_MODEL);
    ind[l++] = j;
  }

  int error = GRBaddgenconstrAnd(Cmodel, name, resind, len, ind);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrOr(GRBVar        resvar,
                         const GRBVar* xvars,
                         int           len,
                         string        cname)
{
  const char *name = (char *) cname.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int resind = resvar.getcolno();
  if (resind < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  if (len > 0 && xvars == NULL)
    throw GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int* ind = new int[len];
  int l = 0;
  for (int i = 0; i < len; i++) {
    int j = xvars[i].getcolno();
    if (j < 0) throw
      GRBException("Variable not in the model", GRB_ERROR_NOT_IN_MODEL);
    ind[l++] = j;
  }

  int error = GRBaddgenconstrOr(Cmodel, name, resind, len, ind);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrIndicator(GRBVar            binvar,
                                int               binval,
                                const GRBLinExpr& expr,
                                char              sense,
                                double            rhs,
                                string            cname)
{
  const char *name = (char *) cname.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int binind = binvar.getcolno();
  if (binind < 0) throw
    GRBException("Variable not in the model", GRB_ERROR_NOT_IN_MODEL);

  // copy expression into C arrays
  int     len = expr.size();
  int*    ind = new int[len];
  double* val = new double[len];

  for (int i = 0; i < len; i++) {
    GRBVar v = expr.getVar(i);
    int j = v.getcolno();
    if (j < 0) {
      delete[] ind;
      delete[] val;
      throw GRBException("Variable not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    ind[i] = j;
    val[i] = expr.getCoeff(i);
  }

  // handle duplicates and zeros
  GRBclean2(&len, ind, val);

  // move constant of expression to rhs
  rhs -= expr.getConstant();

  int error = GRBaddgenconstrIndicator(Cmodel, name, binind, binval, len, ind, val, sense, rhs);
  delete[] ind;
  delete[] val;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrIndicator(GRBVar               binvar,
                                int                  binval,
                                const GRBTempConstr& constr,
                                string               cname)
{
  if (constr.expr.size() > 0)
    throw GRBException("Indicators for quadratic constraints are not supported", GRB_ERROR_INVALID_ARGUMENT);
  return addGenConstrIndicator(binvar, binval, constr.expr.getLinExpr(), constr.sense, 0.0, cname);
}

GRBGenConstr
GRBModel::addGenConstrPWL(GRBVar        xvar,
                          GRBVar        yvar,
                          int           npts,
                          const double *xpts,
                          const double *ypts,
                          string        cname)
{
  const char *name = (char *) cname.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);
  if (npts < 2) throw
    GRBException("Invalid arguments: < 2 points for PWL constraint",
                 GRB_ERROR_INVALID_ARGUMENT);
  if (xpts == NULL || ypts == NULL) throw
    GRBException("Invalid arguments: NULL point", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrPWL(Cmodel, name, xind, yind, npts, xpts, ypts);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrPoly(GRBVar        xvar,
                           GRBVar        yvar,
                           int           plen,
                           const double* p,
                           string        cname,
                           string        coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrPoly(Cmodel, name, xind, yind, plen, p, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrExp(GRBVar xvar,
                          GRBVar yvar,
                          string cname,
                          string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrExp(Cmodel, name, xind, yind, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrExpA(GRBVar xvar,
                           GRBVar yvar,
                           double a,
                           string cname,
                           string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrExpA(Cmodel, name, xind, yind, a, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrLog(GRBVar xvar,
                          GRBVar yvar,
                          string cname,
                          string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrLog(Cmodel, name, xind, yind, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrLogA(GRBVar xvar,
                           GRBVar yvar,
                           double a,
                           string cname,
                           string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrLogA(Cmodel, name, xind, yind, a, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrPow(GRBVar xvar,
                          GRBVar yvar,
                          double a,
                          string cname,
                          string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrPow(Cmodel, name, xind, yind, a, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrSin(GRBVar xvar,
                          GRBVar yvar,
                          string cname,
                          string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrSin(Cmodel, name, xind, yind, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrCos(GRBVar xvar,
                          GRBVar yvar,
                          string cname,
                          string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrCos(Cmodel, name, xind, yind, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}

GRBGenConstr
GRBModel::addGenConstrTan(GRBVar xvar,
                          GRBVar yvar,
                          string cname,
                          string coptions)
{
  const char *name    = (char *) cname.c_str();
  const char *options = (char *) coptions.c_str();

  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int xind = xvar.getcolno();
  int yind = yvar.getcolno();
  if (xind < 0 || yind < 0) throw
    GRBException("xvar or yvar not in the model", GRB_ERROR_NOT_IN_MODEL);

  int error = GRBaddgenconstrTan(Cmodel, name, xind, yind, options);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  genconstrs.push_back(GRBGenConstr(Cmodel, -1));

  return genconstrs.back();
}


void
GRBModel::remove(GRBVar v)
{
  v.remove();
}

void
GRBModel::remove(GRBConstr c)
{
  c.remove();
}

void
GRBModel::remove(GRBSOS s)
{
  s.remove();
}

void
GRBModel::remove(GRBQConstr qc)
{
  qc.remove();
}

void
GRBModel::remove(GRBGenConstr genc)
{
  genc.remove();
}

void
GRBModel::chgCoeff(GRBConstr c,
                   GRBVar    v,
                   double    val)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  int i = c.getrowno();
  int j = v.getcolno();
  if (i < 0 || j < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
  int error = GRBchgcoeffs(Cmodel, 1, &i, &j, &val);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::chgCoeffs(const GRBConstr* xconstrs,
                    const GRBVar*    xvars,
                    const double*    val,
                    int              len)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  if (xconstrs == NULL || xvars == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int* cind = new int[len];
  int* vind = new int[len];
  int i, j, k, l = 0;

  for (k = 0; k < len; k++) {
    i = xconstrs[k].getrowno();
    j = xvars[k].getcolno();
    if (i < 0 || j < 0) {
      delete[] cind;
      delete[] vind;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    cind[l] = i;
    vind[l] = j;
    l++;
  }

  int error = GRBchgcoeffs(Cmodel, len, cind, vind, (double*) val);
  delete[] cind;
  delete[] vind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::chgCoeffs(const GRBConstr* xconstrs,
                    const GRBVar*    xvars,
                    const double*    val,
                    size_t           len)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);
  if (xconstrs == NULL || xvars == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);

  int* cind = new int[len];
  int* vind = new int[len];
  int i, j;
  size_t k, l = 0;

  for (k = 0; k < len; k++) {
    i = xconstrs[k].getrowno();
    j = xvars[k].getcolno();
    if (i < 0 || j < 0) {
      delete[] cind;
      delete[] vind;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    cind[l] = i;
    vind[l] = j;
    l++;
  }

  int error = GRBXchgcoeffs(Cmodel, len, cind, vind, (double*) val);
  delete[] cind;
  delete[] vind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

double
GRBModel::getCoeff(GRBConstr c, GRBVar v) const
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int i = c.getrowno();
  int j = v.getcolno();
  if (i < 0 || i >= rows || j < 0 || j >= cols) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  double val;
  int error = GRBgetcoeff(Cmodel, i, j, &val);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  return val;
}

GRBColumn
GRBModel::getCol(GRBVar v)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  if (v.getcolno() < 0 || v.getcolno() >= cols) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int nz;

  int error = GRBgetvars(Cmodel, &nz, NULL, NULL, NULL, v.getcolno(), 1);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int*    ind = new int[nz];
  double* val = new double[nz];
  int beg;

  error = GRBgetvars(Cmodel, &nz, &beg, ind, val, v.getcolno(), 1);
  if (error) {
    delete[] ind;
    delete[] val;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }

  GRBColumn co = GRBColumn();
  for (int i = 0; i < nz; i++) {
    co.addTerm(val[i], constrs[ind[i]]);
  }

  delete[] ind;
  delete[] val;

  return co;
}

GRBLinExpr
GRBModel::getRow(GRBConstr c)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  if (c.getrowno() < 0 || c.getrowno() >= rows) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int nz;

  int error = GRBgetconstrs(Cmodel, &nz, NULL, NULL, NULL,
                            c.getrowno(), 1);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int*    ind = new int[nz];
  double* val = new double[nz];
  int beg;

  error = GRBgetconstrs(Cmodel, &nz, &beg, ind, val,
                        c.getrowno(), 1);
  if (error) {
    delete[] ind;
    delete[] val;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }

  GRBLinExpr le = GRBLinExpr();
  for (int i = 0; i < nz; i++) {
    le += val[i] * vars[ind[i]];
  }

  delete[] ind;
  delete[] val;

  return le;
}

int
GRBModel::getSOS(GRBSOS    xsos,
                 GRBVar*   xvars,
                 double*   weight,
                 int*      typeP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int sosi = xsos.getindex();
  if (sosi < 0) return 0;

  int nz;
  int error = GRBgetsos(Cmodel, &nz, typeP, NULL, NULL, NULL, sosi, 1);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvars == NULL || weight == NULL) return nz;

  int* ind = new int[nz];
  int beg;

  error = GRBgetsos(Cmodel, &nz, typeP, &beg, ind, weight, sosi, 1);
  if (error) {
    delete[] ind;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }

  for (int i = 0; i < nz; i++) xvars[i] = vars[ind[i]];
  delete[] ind;

  return nz;
}

void
GRBModel::getGenConstrMax(GRBGenConstr  genc,
                          GRBVar*       resvarP,
                          GRBVar*       xvars,
                          int*          lenP,
                          double*       constantP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int resvar;
  int nvars;

  int error = GRBgetgenconstrMax(Cmodel, genci, &resvar, &nvars, NULL, constantP);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvars != NULL) {
    int* ind = new int[nvars];
    error = GRBgetgenconstrMax(Cmodel, genci, NULL, NULL, ind, NULL);
    if (error) {
      delete[] ind;
      throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    }
    for (int i = 0; i < nvars; i++)
      xvars[i] = vars[ind[i]];
    delete[] ind;
  }

  if (resvarP != NULL)
    *resvarP = vars[resvar];
  if (lenP != NULL)
    *lenP = nvars;
}

void
GRBModel::getGenConstrMin(GRBGenConstr  genc,
                          GRBVar*       resvarP,
                          GRBVar*       xvars,
                          int*          lenP,
                          double*       constantP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int resvar;
  int nvars;

  int error = GRBgetgenconstrMin(Cmodel, genci, &resvar, &nvars, NULL, constantP);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvars != NULL) {
    int* ind = new int[nvars];
    error = GRBgetgenconstrMin(Cmodel, genci, NULL, NULL, ind, NULL);
    if (error) {
      delete[] ind;
      throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    }
    for (int i = 0; i < nvars; i++)
      xvars[i] = vars[ind[i]];
    delete[] ind;
  }

  if (resvarP != NULL)
    *resvarP = vars[resvar];
  if (lenP != NULL)
    *lenP = nvars;
}

void
GRBModel::getGenConstrAbs(GRBGenConstr  genc,
                          GRBVar*       resvarP,
                          GRBVar*       argvarP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int resvar;
  int argvar;

  int error = GRBgetgenconstrAbs(Cmodel, genci, &resvar, &argvar);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (resvarP != NULL)
    *resvarP = vars[resvar];
  if (argvarP != NULL)
    *argvarP = vars[argvar];
}

void
GRBModel::getGenConstrAnd(GRBGenConstr  genc,
                          GRBVar*       resvarP,
                          GRBVar*       xvars,
                          int*          lenP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int resvar;
  int nvars;

  int error = GRBgetgenconstrAnd(Cmodel, genci, &resvar, &nvars, NULL);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvars != NULL) {
    int* ind = new int[nvars];
    error = GRBgetgenconstrAnd(Cmodel, genci, NULL, NULL, ind);
    if (error) {
      delete[] ind;
      throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    }
    for (int i = 0; i < nvars; i++)
      xvars[i] = vars[ind[i]];
    delete[] ind;
  }

  if (resvarP != NULL)
    *resvarP = vars[resvar];
  if (lenP != NULL)
    *lenP = nvars;
}

void
GRBModel::getGenConstrOr(GRBGenConstr  genc,
                         GRBVar*       resvarP,
                         GRBVar*       xvars,
                         int*          lenP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int resvar;
  int nvars;

  int error = GRBgetgenconstrOr(Cmodel, genci, &resvar, &nvars, NULL);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvars != NULL) {
    int* ind = new int[nvars];
    error = GRBgetgenconstrOr(Cmodel, genci, NULL, NULL, ind);
    if (error) {
      delete[] ind;
      throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    }
    for (int i = 0; i < nvars; i++)
      xvars[i] = vars[ind[i]];
    delete[] ind;
  }

  if (resvarP != NULL)
    *resvarP = vars[resvar];
  if (lenP != NULL)
    *lenP = nvars;
}

void
GRBModel::getGenConstrIndicator(GRBGenConstr  genc,
                                GRBVar*       binvarP,
                                int*          binvalP,
                                GRBLinExpr*   exprP,
                                char*         senseP,
                                double*       rhsP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int binvar;
  int nvars;

  int error = GRBgetgenconstrIndicator(Cmodel, genci, &binvar, binvalP, &nvars, NULL, NULL, senseP, rhsP);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (exprP != NULL) {
    int*    ind = new int[nvars];
    double* val = new double[nvars];
    error = GRBgetgenconstrIndicator(Cmodel, genci, NULL, NULL, NULL, ind, val, NULL, NULL);
    if (error) {
      delete[] ind;
      delete[] val;
      throw GRBException(string(GRBgeterrormsg(Cenv)), error);
    }

    GRBVar* xvars = new GRBVar[nvars];
    for (int i = 0; i < nvars; i++)
      xvars[i] = vars[ind[i]];

    // Note: this may leak memory if addTerms() throws an exception!
    exprP->clear();
    exprP->addTerms(val, xvars, nvars);

    delete[] ind;
    delete[] val;
    delete[] xvars;
  }

  if (binvarP != NULL)
    *binvarP = vars[binvar];
}

void
GRBModel::getGenConstrPWL(GRBGenConstr  genc,
                          GRBVar*       xvarP,
                          GRBVar*       yvarP,
                          int*          nptsP,
                          double*       xpts,
                          double*       ypts)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrPWL(Cmodel, genci, &xind, &yind,
                                 nptsP, xpts, ypts);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrPoly(GRBGenConstr  genc,
                           GRBVar*       xvarP,
                           GRBVar*       yvarP,
                           int*          plenP,
                           double*       p)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrPoly(Cmodel, genci, &xind, &yind, plenP, p);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrExp(GRBGenConstr  genc,
                          GRBVar*       xvarP,
                          GRBVar*       yvarP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrExp(Cmodel, genci, &xind, &yind);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrExpA(GRBGenConstr  genc,
                           GRBVar*       xvarP,
                           GRBVar*       yvarP,
                           double*       aP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrExpA(Cmodel, genci, &xind, &yind, aP);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrLog(GRBGenConstr  genc,
                          GRBVar*       xvarP,
                          GRBVar*       yvarP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrLog(Cmodel, genci, &xind, &yind);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrLogA(GRBGenConstr  genc,
                           GRBVar*       xvarP,
                           GRBVar*       yvarP,
                           double*       aP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrLogA(Cmodel, genci, &xind, &yind, aP);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrPow(GRBGenConstr  genc,
                          GRBVar*       xvarP,
                          GRBVar*       yvarP,
                          double*       aP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrPow(Cmodel, genci, &xind, &yind, aP);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrSin(GRBGenConstr  genc,
                          GRBVar*       xvarP,
                          GRBVar*       yvarP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrSin(Cmodel, genci, &xind, &yind);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrCos(GRBGenConstr  genc,
                          GRBVar*       xvarP,
                          GRBVar*       yvarP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrCos(Cmodel, genci, &xind, &yind);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

void
GRBModel::getGenConstrTan(GRBGenConstr  genc,
                          GRBVar*       xvarP,
                          GRBVar*       yvarP)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  int genci = genc.getindex();
  if (genci < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int xind, yind;
  int error = GRBgetgenconstrTan(Cmodel, genci, &xind, &yind);
  if (error)
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (xvarP != NULL)
    *xvarP = vars[xind];
  if (yvarP != NULL)
    *yvarP = vars[yind];
}

GRBQuadExpr
GRBModel::getQCRow(GRBQConstr qc)
{
  if (Cmodel == NULL) throw
    GRBException("Model not loaded", GRB_ERROR_INTERNAL);

  if (qc.getindex() < 0) throw
    GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);

  int lnz, qnz;

  int error = GRBgetqconstr(Cmodel, qc.getindex(), &lnz, NULL, NULL,
                            &qnz, NULL, NULL, NULL);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  int*    lind = new int[lnz];
  double* lval = new double[lnz];
  int*    qrow = new int[qnz];
  int*    qcol = new int[qnz];
  double* qval = new double[qnz];

  error = GRBgetqconstr(Cmodel, qc.getindex(), &lnz, lind, lval,
                        &qnz, qrow, qcol, qval);
  if (error) {
    delete[] lind;
    delete[] lval;
    delete[] qrow;
    delete[] qcol;
    delete[] qval;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }

  GRBQuadExpr qe = GRBQuadExpr();
  int i;
  for (i = 0; i < lnz; i++)
    qe += lval[i] * vars[lind[i]];
  for (i = 0; i < qnz; i++)
    qe += qval[i] * vars[qrow[i]] * vars[qcol[i]];

  delete[] lind;
  delete[] lval;
  delete[] qrow;
  delete[] qcol;
  delete[] qval;

  return qe;
}

GRBEnv
GRBModel::getEnv() const
{
  return GRBEnv(Cenv);
}

GRBEnv
GRBModel::getConcurrentEnv(int num)
{
  GRBenv *concurrentenv = GRBgetconcurrentenv(Cmodel, num);

  if (concurrentenv == NULL)
    throw GRBException("Failed to create concurrent env",
                       GRB_ERROR_INVALID_ARGUMENT);

  return GRBEnv(concurrentenv);
}

void
GRBModel::discardConcurrentEnvs()
{
  GRBdiscardconcurrentenvs(Cmodel);
}

GRBEnv
GRBModel::getMultiobjEnv(int num)
{
  GRBenv *multiobjenv = GRBgetmultiobjenv(Cmodel, num);

  if (multiobjenv == NULL)
    throw GRBException("Failed to create multiobj env",
                       GRB_ERROR_INVALID_ARGUMENT);

  return GRBEnv(multiobjenv);
}

void
GRBModel::discardMultiobjEnvs()
{
  GRBdiscardmultiobjenvs(Cmodel);
}

int
GRBModel::get(GRB_IntParam param) const
{
  return getEnv().get(param);
}

double
GRBModel::get(GRB_DoubleParam param) const
{
  return getEnv().get(param);
}

string
GRBModel::get(GRB_StringParam param) const
{
  return getEnv().get(param);
}

void
GRBModel::set(GRB_IntParam param, int value)
{
  return getEnv().set(param, value);
}

void
GRBModel::set(GRB_DoubleParam param, double value)
{
  return getEnv().set(param, value);
}

void
GRBModel::set(GRB_StringParam param, const string& value)
{
  return getEnv().set(param, value);
}

void
GRBModel::set(const string& param, const string& value)
{
  return getEnv().set(param, value);
}

int
GRBModel::get(GRB_IntAttr attr) const
{
  int value;

  int error = GRBgetintattr(Cmodel, iattrname[attr], &value);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  return value;
}

double
GRBModel::get(GRB_DoubleAttr attr) const
{
  double value;

  int error = GRBgetdblattr(Cmodel, dattrname[attr], &value);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  return value;
}

string
GRBModel::get(GRB_StringAttr attr) const
{
  char *value;

  int error = GRBgetstrattr(Cmodel, sattrname[attr], &value);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (value == NULL) return string("");
  else               return string(value);
}

void
GRBModel::set(GRB_IntAttr attr, int value)
{
  int error = GRBsetintattr(Cmodel, iattrname[attr], value);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_DoubleAttr attr, double value)
{
  int error = GRBsetdblattr(Cmodel, dattrname[attr], value);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_StringAttr attr, const string& value)
{
  int error = GRBsetstrattr(Cmodel, sattrname[attr], (char*) value.c_str());
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

int*
GRBModel::get(GRB_IntAttr   attr,
              const GRBVar* xvars,
              int           len)
{
  if (len <= 0) return NULL;
  if (xvars == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, iattrname[attr], 1);
  int* val = new int[len];
  int* ind = setvarsind(xvars, len);
  int error = GRBgetintattrlist(Cmodel, iattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

char*
GRBModel::get(GRB_CharAttr  attr,
              const GRBVar* xvars,
              int           len)
{
  if (len <= 0) return NULL;
  if (xvars == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, cattrname[attr], 1);
  char* val = new char[len];
  int* ind = setvarsind(xvars, len);
  int error = GRBgetcharattrlist(Cmodel, cattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

double*
GRBModel::get(GRB_DoubleAttr  attr,
              const GRBVar*   xvars,
              int             len)
{
  if (len <= 0) return NULL;
  if (xvars == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, dattrname[attr], 1);
  double* val = new double[len];
  int* ind = setvarsind(xvars, len);
  int error = GRBgetdblattrlist(Cmodel, dattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

string*
GRBModel::get(GRB_StringAttr  attr,
              const GRBVar*   xvars,
              int             len)
{
  if (len <= 0) return NULL;
  if (xvars == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 1);
  char** val1 = new char*[len];
  int* ind = setvarsind(xvars, len);
  int error = GRBgetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  if (error) {
    delete[] val1;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }
  string* val = new string[len];
  for (int i = 0; i < len; i++) val[i] = string(val1[i]);
  delete[] val1;
  return val;
}

int*
GRBModel::get(GRB_IntAttr      attr,
              const GRBConstr* xconstrs,
              int              len)
{
  if (len <= 0) return NULL;
  if (xconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, iattrname[attr], 2);
  int* val = new int[len];
  int* ind = setconstrsind(xconstrs, len);
  int error = GRBgetintattrlist(Cmodel, iattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

char*
GRBModel::get(GRB_CharAttr     attr,
              const GRBConstr* xconstrs,
              int              len)
{
  if (len <= 0) return NULL;
  if (xconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, cattrname[attr], 2);
  char* val = new char[len];
  int* ind = setconstrsind(xconstrs, len);
  int error = GRBgetcharattrlist(Cmodel, cattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

double*
GRBModel::get(GRB_DoubleAttr     attr,
              const GRBConstr*   xconstrs,
              int                len)
{
  if (len <= 0) return NULL;
  if (xconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, dattrname[attr], 2);
  double* val = new double[len];
  int* ind = setconstrsind(xconstrs, len);
  int error = GRBgetdblattrlist(Cmodel, dattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

string*
GRBModel::get(GRB_StringAttr     attr,
              const GRBConstr*   xconstrs,
              int                len)
{
  if (len <= 0) return NULL;
  if (xconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 2);
  char** val1 = new char*[len];
  int* ind = setconstrsind(xconstrs, len);
  int error = GRBgetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  if (error) {
    delete[] val1;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }
  string* val = new string[len];
  for (int i = 0; i < len; i++) val[i] = string(val1[i]);
  delete[] val1;
  return val;
}

int*
GRBModel::get(GRB_IntAttr        attr,
              const GRBQConstr*  xqconstrs,
              int                len)
{
  if (len <= 0) return NULL;
  if (xqconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, iattrname[attr], 4);
  int* val = new int[len];
  int* ind = setqconstrsind(xqconstrs, len);
  int error = GRBgetintattrlist(Cmodel, iattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

char*
GRBModel::get(GRB_CharAttr      attr,
              const GRBQConstr* xqconstrs,
              int               len)
{
  if (len <= 0) return NULL;
  if (xqconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, cattrname[attr], 4);
  char* val = new char[len];
  int* ind = setqconstrsind(xqconstrs, len);
  int error = GRBgetcharattrlist(Cmodel, cattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

double*
GRBModel::get(GRB_DoubleAttr     attr,
              const GRBQConstr*  xqconstrs,
              int                len)
{
  if (len <= 0) return NULL;
  if (xqconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, dattrname[attr], 4);
  double* val = new double[len];
  int* ind = setqconstrsind(xqconstrs, len);
  int error = GRBgetdblattrlist(Cmodel, dattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

string*
GRBModel::get(GRB_StringAttr     attr,
              const GRBQConstr*  xqconstrs,
              int                len)
{
  if (len <= 0) return NULL;
  if (xqconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 4);
  char** val1 = new char*[len];
  int* ind = setqconstrsind(xqconstrs, len);
  int error = GRBgetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  if (error) {
    delete[] val1;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }
  string* val = new string[len];
  for (int i = 0; i < len; i++) val[i] = string(val1[i]);
  delete[] val1;
  return val;
}

int*
GRBModel::get(GRB_IntAttr          attr,
              const GRBGenConstr*  xgenconstrs,
              int                  len)
{
  if (len <= 0) return NULL;
  if (xgenconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, iattrname[attr], 5);
  int* val = new int[len];
  int* ind = setgenconstrsind(xgenconstrs, len);
  int error = GRBgetintattrlist(Cmodel, iattrname[attr], len, ind, val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  return val;
}

string*
GRBModel::get(GRB_StringAttr       attr,
              const GRBGenConstr*  xgenconstrs,
              int                  len)
{
  if (len <= 0) return NULL;
  if (xgenconstrs == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 5);
  char** val1 = new char*[len];
  int* ind = setgenconstrsind(xgenconstrs, len);
  int error = GRBgetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  if (error) {
    delete[] val1;
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }
  string* val = new string[len];
  for (int i = 0; i < len; i++) val[i] = string(val1[i]);
  delete[] val1;
  return val;
}

void
GRBModel::prefetchAttr(GRB_IntAttr attr)
{
  int error = GRBprefetchattr(Cmodel, iattrname[attr]);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::prefetchAttr(GRB_CharAttr attr)
{
  int error = GRBprefetchattr(Cmodel, cattrname[attr]);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::prefetchAttr(GRB_DoubleAttr attr)
{
  int error = GRBprefetchattr(Cmodel, dattrname[attr]);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::prefetchAttr(GRB_StringAttr attr)
{
  int error = GRBprefetchattr(Cmodel, sattrname[attr]);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

string
GRBModel::getJSONSolution(void)
{
  char *sval = NULL;
  int error  = 0;

  error = GRBgetjsonsolution(Cmodel, &sval);
  if (error) {
    assert(!sval);
    if (sval) GRBfree(sval);
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }

  string rval = string(sval);
  GRBfree(sval); // IMPORTANT: this is in the DLL heap on Windows,
                 // so use GRBfree
  return rval;
}

void
GRBModel::set(GRB_IntAttr     attr,
              const GRBVar*   xvars,
              const int*      val,
              int             len)
{
  if (len <= 0) return;
  if (xvars == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, iattrname[attr], 1);
  int* ind = setvarsind(xvars, len);
  int error = GRBsetintattrlist(Cmodel, iattrname[attr], len, ind,
                                (int*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_CharAttr    attr,
              const GRBVar*   xvars,
              const char*     val,
              int             len)
{
  if (len <= 0) return;
  if (xvars == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, cattrname[attr], 1);
  int* ind = setvarsind(xvars, len);
  int error = GRBsetcharattrlist(Cmodel, cattrname[attr], len, ind,
                                 (char*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_DoubleAttr  attr,
              const GRBVar*   xvars,
              const double*   val,
              int             len)
{
  if (len <= 0) return;
  if (xvars == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, dattrname[attr], 1);
  int* ind = setvarsind(xvars, len);
  int error = GRBsetdblattrlist(Cmodel, dattrname[attr], len, ind,
                                (double*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_StringAttr  attr,
              const GRBVar*   xvars,
              const string*   val,
              int             len)
{
  if (len <= 0) return;
  if (xvars == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 1);
  int* ind = setvarsind(xvars, len);
  int slen = 0;
  int i;
  for (i = 0; i < len; i++) slen += strlen(val[i].c_str())+1;
  char* store = new char[slen];
  char** val1 = new char*[len];
  slen = 0;
  for (i = 0; i < len; i++) {
    strcpy(store+slen, val[i].c_str());
    val1[i] = store+slen;
    slen += strlen(val[i].c_str());
    store[slen] = '\0';
    slen++;
  }
  int error = GRBsetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  delete[] store;
  delete[] val1;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_IntAttr        attr,
              const GRBConstr*   xconstrs,
              const int*         val,
              int                len)
{
  if (len <= 0) return;
  if (xconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, iattrname[attr], 2);
  int* ind = setconstrsind(xconstrs, len);
  int error = GRBsetintattrlist(Cmodel, iattrname[attr], len, ind,
                                (int*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_CharAttr       attr,
              const GRBConstr*   xconstrs,
              const char*        val,
              int                len)
{
  if (len <= 0) return;
  if (xconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, cattrname[attr], 2);
  int* ind = setconstrsind(xconstrs, len);
  int error = GRBsetcharattrlist(Cmodel, cattrname[attr], len, ind,
                                 (char*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_DoubleAttr     attr,
              const GRBConstr*   xconstrs,
              const double*      val,
              int                len)
{
  if (len <= 0) return;
  if (xconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, dattrname[attr], 2);
  int* ind = setconstrsind(xconstrs, len);
  int error = GRBsetdblattrlist(Cmodel, dattrname[attr], len, ind,
                                (double*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_StringAttr     attr,
              const GRBConstr*   xconstrs,
              const string*      val,
              int                len)
{
  if (len <= 0) return;
  if (xconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 2);
  int* ind = setconstrsind(xconstrs, len);
  int slen = 0;
  int i;
  for (i = 0; i < len; i++) slen += strlen(val[i].c_str())+1;
  char* store = new char[slen];
  char** val1 = new char*[len];
  slen = 0;
  for (i = 0; i < len; i++) {
    strcpy(store+slen, val[i].c_str());
    val1[i] = store+slen;
    slen += strlen(val[i].c_str());
    store[slen] = '\0';
    slen++;
  }
  int error = GRBsetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  delete[] store;
  delete[] val1;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_CharAttr       attr,
              const GRBQConstr*  xqconstrs,
              const char*        val,
              int                len)
{
  if (len <= 0) return;
  if (xqconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, cattrname[attr], 4);
  int* ind = setqconstrsind(xqconstrs, len);
  int error = GRBsetcharattrlist(Cmodel, cattrname[attr], len, ind,
                                 (char*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_DoubleAttr     attr,
              const GRBQConstr*  xqconstrs,
              const double*      val,
              int                len)
{
  if (len <= 0) return;
  if (xqconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, dattrname[attr], 4);
  int* ind = setqconstrsind(xqconstrs, len);
  int error = GRBsetdblattrlist(Cmodel, dattrname[attr], len, ind,
                                (double*) val);
  delete[] ind;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_StringAttr     attr,
              const GRBQConstr*  xqconstrs,
              const string*      val,
              int                len)
{
  if (len <= 0) return;
  if (xqconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 4);
  int* ind = setqconstrsind(xqconstrs, len);
  int slen = 0;
  int i;
  for (i = 0; i < len; i++) slen += strlen(val[i].c_str())+1;
  char* store = new char[slen];
  char** val1 = new char*[len];
  slen = 0;
  for (i = 0; i < len; i++) {
    strcpy(store+slen, val[i].c_str());
    val1[i] = store+slen;
    slen += strlen(val[i].c_str());
    store[slen] = '\0';
    slen++;
  }
  int error = GRBsetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  delete[] store;
  delete[] val1;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::set(GRB_StringAttr       attr,
              const GRBGenConstr*  xgenconstrs,
              const string*        val,
              int                  len)
{
  if (len <= 0) return;
  if (xgenconstrs == NULL || val == NULL) throw
    GRBException("Invalid arguments", GRB_ERROR_INVALID_ARGUMENT);
  checkattrsize(Cmodel, sattrname[attr], 5);
  int* ind = setgenconstrsind(xgenconstrs, len);
  int slen = 0;
  int i;
  for (i = 0; i < len; i++) slen += strlen(val[i].c_str())+1;
  char* store = new char[slen];
  char** val1 = new char*[len];
  slen = 0;
  for (i = 0; i < len; i++) {
    strcpy(store+slen, val[i].c_str());
    val1[i] = store+slen;
    slen += strlen(val[i].c_str());
    store[slen] = '\0';
    slen++;
  }
  int error = GRBsetstrattrlist(Cmodel, sattrname[attr], len, ind, val1);
  delete[] ind;
  delete[] store;
  delete[] val1;
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBModel::setCallback(GRBCallback* xcb)
{
  cb = xcb;
  if (xcb == NULL) {
    int error = GRBsetcallbackfunc(Cmodel, NULL, NULL);
    if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  } else {
    xcb->setcb(xcb, Cmodel, cols);
  }
}

int*
GRBModel::setvarsind(const GRBVar* vars, int size)
{
  int* ind = new int[size];
  for (int i1 = 0; i1 < size; i1++) {
    int col = vars[i1].getcolno();
    if (col < 0) {
      delete[] ind;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    ind[i1] = col;
  }
  return ind;
}

int*
GRBModel::setconstrsind(const GRBConstr* constrs, int size)
{
  int* ind = new int[size];
  for (int i1 = 0; i1 < size; i1++) {
    int row = constrs[i1].getrowno();
    if (row < 0) {
      delete[] ind;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    ind[i1] = row;
  }
  return ind;
}

int*
GRBModel::setqconstrsind(const GRBQConstr* qconstrs, int size)
{
  int* ind = new int[size];
  for (int i1 = 0; i1 < size; i1++) {
    int qc = qconstrs[i1].getindex();
    if (qc < 0) {
      delete[] ind;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    ind[i1] = qc;
  }
  return ind;
}

int*
GRBModel::setgenconstrsind(const GRBGenConstr* genconstrs, int size)
{
  int* ind = new int[size];
  for (int i1 = 0; i1 < size; i1++) {
    int genc = genconstrs[i1].getindex();
    if (genc < 0) {
      delete[] ind;
      throw GRBException("Not in the model", GRB_ERROR_NOT_IN_MODEL);
    }
    ind[i1] = genc;
  }
  return ind;
}
