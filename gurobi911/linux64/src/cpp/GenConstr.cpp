// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"
#include "attrprivate.h"

GRBGenConstr::GRBGenConstr()
{
  genconRep = NULL;
}

GRBGenConstr::GRBGenConstr(GRBmodel* xmodel,
                           int       genconstr)
{
  genconRep = new GRBGenConstrRep();
  genconRep->Cmodel = xmodel;
  genconRep->num = genconstr;
}

void GRBGenConstr::remove()
{
  if (genconRep != NULL) {
    int j = genconRep->num;
    if      (j >=  0) genconRep->num = -3 -j;
    else if (j == -1) throw
      GRBException("not in model for removing", GRB_ERROR_NOT_IN_MODEL);
  }
  genconRep = NULL;
}

void
GRBGenConstr::setindex(int xnum)
{
  genconRep->num = xnum;
}

int
GRBGenConstr::getindex() const
{
  if (genconRep == NULL) return -2;
  else                   return genconRep->num;
}

int
GRBGenConstr::get(GRB_IntAttr attr) const
{
  int value;

  if (genconRep == NULL || genconRep->Cmodel == NULL || genconRep->num < 0)
    throw GRBException("General constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(genconRep->Cmodel, iattrname[attr], 5);
  int error = GRBgetintattrelement(genconRep->Cmodel, iattrname[attr],
                                   genconRep->num, &value);
  if (error) throw GRBException("GenConstr::get", error);

  return value;
}

double
GRBGenConstr::get(GRB_DoubleAttr attr) const
{
  double value;

  if (genconRep == NULL || genconRep->Cmodel == NULL || genconRep->num < 0)
    throw GRBException("General constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(genconRep->Cmodel, dattrname[attr], 5);
  int error = GRBgetdblattrelement(genconRep->Cmodel, dattrname[attr],
                                   genconRep->num, &value);
  if (error) throw GRBException("GenConstr::get", error);

  return value;
}

string
GRBGenConstr::get(GRB_StringAttr attr) const
{
  char *value;

  if (genconRep == NULL || genconRep->Cmodel == NULL || genconRep->num < 0)
    throw GRBException("General constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(genconRep->Cmodel, sattrname[attr], 5);
  int error = GRBgetstrattrelement(genconRep->Cmodel, sattrname[attr],
                                   genconRep->num, &value);
  if (error) throw GRBException("GenConstr::get", error);

  return string(value);
}

void
GRBGenConstr::set(GRB_IntAttr attr, int value)
{
  if (genconRep == NULL || genconRep->Cmodel == NULL || genconRep->num < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(genconRep->Cmodel, iattrname[attr], 5);
  int error = GRBsetintattrelement(genconRep->Cmodel, iattrname[attr],
                                   genconRep->num, value);
  if (error) throw GRBException("GenConstr::set", error);
}

void
GRBGenConstr::set(GRB_DoubleAttr attr, double value)
{
  if (genconRep == NULL || genconRep->Cmodel == NULL || genconRep->num < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(genconRep->Cmodel, dattrname[attr], 5);
  int error = GRBsetdblattrelement(genconRep->Cmodel, dattrname[attr],
                                   genconRep->num, value);
  if (error) throw GRBException("GenConstr::set", error);
}

void
GRBGenConstr::set(GRB_StringAttr attr, const string& value)
{
  if (genconRep == NULL || genconRep->Cmodel == NULL || genconRep->num < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(genconRep->Cmodel, sattrname[attr], 5);
  int error = GRBsetstrattrelement(genconRep->Cmodel, sattrname[attr],
                                   genconRep->num, (char *) value.c_str());
  if (error) throw GRBException("GenConstr::set", error);
}
