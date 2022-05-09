// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"
#include "attrprivate.h"

GRBQConstr::GRBQConstr()
{
  qconRep = NULL;
}

GRBQConstr::GRBQConstr(GRBmodel* xmodel,
                       int       qconstr)
{
  qconRep = new GRBQConstrRep();
  qconRep->Cmodel = xmodel;
  qconRep->num = qconstr;
}

void GRBQConstr::remove()
{
  if (qconRep != NULL) {
    int j = qconRep->num;
    if      (j >=  0) qconRep->num = -3 -j;
    else if (j == -1) throw
      GRBException("not in model for removing", GRB_ERROR_NOT_IN_MODEL);
  }
  qconRep = NULL;
}

void
GRBQConstr::setindex(int xnum)
{
  qconRep->num = xnum;
}

int
GRBQConstr::getindex() const
{
  if (qconRep == NULL) return -2;
  else                 return qconRep->num;
}

char
GRBQConstr::get(GRB_CharAttr attr) const
{
  char value;

  if (qconRep == NULL || qconRep->Cmodel == NULL || qconRep->num < 0)
    throw GRBException("QConstr not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(qconRep->Cmodel, cattrname[attr], 4);
  int error = GRBgetcharattrelement(qconRep->Cmodel, cattrname[attr],
                                    qconRep->num, &value);
  if (error) throw GRBException("QConstr::get", error);

  return value;
}

int
GRBQConstr::get(GRB_IntAttr attr) const
{
  int value;

  if (qconRep == NULL || qconRep->Cmodel == NULL || qconRep->num < 0)
    throw GRBException("QConstr not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(qconRep->Cmodel, iattrname[attr], 4);
  int error = GRBgetintattrelement(qconRep->Cmodel, iattrname[attr],
                                   qconRep->num, &value);
  if (error) throw GRBException("QConstr::get", error);

  return value;
}

double
GRBQConstr::get(GRB_DoubleAttr attr) const
{
  double    value;

  if (qconRep == NULL || qconRep->Cmodel == NULL || qconRep->num < 0)
    throw GRBException("QConstr not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(qconRep->Cmodel, dattrname[attr], 4);
  int error = GRBgetdblattrelement(qconRep->Cmodel, dattrname[attr],
                                   qconRep->num, &value);
  if (error) throw GRBException("QConstr::get", error);

  return value;
}

string
GRBQConstr::get(GRB_StringAttr attr) const
{
  char *value;

  if (qconRep == NULL || qconRep->Cmodel == NULL || qconRep->num < 0)
    throw GRBException("QConstr not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(qconRep->Cmodel, sattrname[attr], 4);
  int error = GRBgetstrattrelement(qconRep->Cmodel, sattrname[attr],
                                   qconRep->num, &value);
  if (error) throw GRBException("QConstr::get", error);

  return string(value);
}

void
GRBQConstr::set(GRB_DoubleAttr attr, double value)
{
  if (qconRep == NULL || qconRep->Cmodel == NULL || qconRep->num < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(qconRep->Cmodel, dattrname[attr], 4);
  int error = GRBsetdblattrelement(qconRep->Cmodel, dattrname[attr],
                                   qconRep->num, value);
  if (error) throw GRBException("QConstr::set", error);
}

void
GRBQConstr::set(GRB_CharAttr attr, char value)
{
  if (qconRep == NULL || qconRep->Cmodel == NULL || qconRep->num < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(qconRep->Cmodel, cattrname[attr], 4);
  int error = GRBsetcharattrelement(qconRep->Cmodel, cattrname[attr],
                                    qconRep->num, value);
  if (error) throw GRBException("QConstr::set", error);
}

void
GRBQConstr::set(GRB_StringAttr attr, const string& value)
{
  if (qconRep == NULL || qconRep->Cmodel == NULL || qconRep->num < 0)
    throw GRBException("Variable not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(qconRep->Cmodel, sattrname[attr], 4);
  int error = GRBsetstrattrelement(qconRep->Cmodel, sattrname[attr],
                                   qconRep->num, (char *) value.c_str());
  if (error) throw GRBException("QConstr::set", error);
}
