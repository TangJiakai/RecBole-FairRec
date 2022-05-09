// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"
#include "attrprivate.h"

GRBVar::GRBVar()
{
  varRep = NULL;
}

GRBVar::GRBVar(GRBmodel* xmodel,
               int       xcol_no)
{
  varRep = new GRBVarRep();
  varRep->Cmodel = xmodel;
  varRep->col_no = xcol_no;
}


void GRBVar::remove()
{
  if (varRep != NULL) {
    int j = varRep->col_no;
    if      (j >=  0) varRep->col_no = -3 -j;
    else if (j == -1) throw
      GRBException("Not in model for removing", GRB_ERROR_NOT_IN_MODEL);
  }
  varRep = NULL;
}

void
GRBVar::setcolno(int xcol_no)
{
  varRep->col_no = xcol_no;
}

int
GRBVar::getcolno() const
{
  if (varRep == NULL) return -2;
  else                return varRep->col_no;
}

int
GRBVar::index() const
{
  return getcolno();
}

int
GRBVar::get(GRB_IntAttr attr) const
{
  int value;

  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, iattrname[attr], 1);
  int error = GRBgetintattrelement(varRep->Cmodel, iattrname[attr],
                                   varRep->col_no, &value);
  if (error) throw GRBException("Var::get", error);

  return value;
}

char
GRBVar::get(GRB_CharAttr attr) const
{
  char value;

  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, cattrname[attr], 1);
  int error = GRBgetcharattrelement(varRep->Cmodel, cattrname[attr],
                                    varRep->col_no, &value);
  if (error) throw GRBException("Var::get", error);

  return value;
}

double
GRBVar::get(GRB_DoubleAttr attr) const
{
  double    value;

  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, dattrname[attr], 1);
  int error = GRBgetdblattrelement(varRep->Cmodel, dattrname[attr],
                                   varRep->col_no, &value);
  if (error) throw GRBException("Var::get", error);

  return value;
}

string
GRBVar::get(GRB_StringAttr attr) const
{
  char     *value;

  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, sattrname[attr], 1);
  int error = GRBgetstrattrelement(varRep->Cmodel, sattrname[attr],
                                   varRep->col_no, &value);
  if (error) throw GRBException("Var::get", error);

  return string(value);
}

void
GRBVar::set(GRB_IntAttr attr, int value)
{
  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, iattrname[attr], 1);
  int error = GRBsetintattrelement(varRep->Cmodel, iattrname[attr],
                                   varRep->col_no, value);
  if (error) throw GRBException("Var::set", error);
}

void
GRBVar::set(GRB_DoubleAttr attr, double value)
{
  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, dattrname[attr], 1);
  int error = GRBsetdblattrelement(varRep->Cmodel, dattrname[attr],
                                   varRep->col_no, value);
  if (error) throw GRBException("Var::set", error);
}

void
GRBVar::set(GRB_CharAttr attr, char value)
{
  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, cattrname[attr], 1);
  int error = GRBsetcharattrelement(varRep->Cmodel, cattrname[attr],
                                    varRep->col_no, value);
  if (error) throw GRBException("Var::set", error);
}

void
GRBVar::set(GRB_StringAttr attr, const string& value)
{
  if (varRep == NULL || varRep->Cmodel == NULL || varRep->col_no < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(varRep->Cmodel, sattrname[attr], 1);
  int error = GRBsetstrattrelement(varRep->Cmodel, sattrname[attr],
                                   varRep->col_no, (char *) value.c_str());
  if (error) throw GRBException("Var::set", error);
}

bool
GRBVar::sameAs(GRBVar v2)
{
  return (varRep == v2.varRep);
}
