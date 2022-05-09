// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"
#include "attrprivate.h"

GRBConstr::GRBConstr()
{
  conRep = NULL;
}

GRBConstr::GRBConstr(GRBmodel* xmodel,
                     int       xrow_no)
{
  conRep = new GRBConRep();
  conRep->Cmodel = xmodel;
  conRep->row_no = xrow_no;
}


void GRBConstr::remove()
{
  if (conRep != NULL) {
    int j = conRep->row_no;
    if      (j >=  0) conRep->row_no = -3 -j;
    else if (j == -1) throw
      GRBException("not in model for removing", GRB_ERROR_NOT_IN_MODEL);
  }
  conRep->Cmodel = NULL;
}

void
GRBConstr::setrowno(int xrow_no)
{
  conRep->row_no = xrow_no;
}

int
GRBConstr::getrowno() const
{
  if (conRep == NULL) return -2;
  else                return conRep->row_no;
}

int
GRBConstr::index() const
{
  return getrowno();
}

int
GRBConstr::get(GRB_IntAttr attr) const
{
  int value;

  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, iattrname[attr], 2);
  int error = GRBgetintattrelement(conRep->Cmodel, iattrname[attr],
                                   conRep->row_no, &value);
  if (error) throw GRBException("Constr::get", error);

  return value;
}

char
GRBConstr::get(GRB_CharAttr attr) const
{
  char value;

  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, cattrname[attr], 2);
  int error = GRBgetcharattrelement(conRep->Cmodel, cattrname[attr],
                                    conRep->row_no, &value);
  if (error) throw GRBException("Constr::get", error);

  return value;
}

double
GRBConstr::get(GRB_DoubleAttr attr) const
{
  double    value;

  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, dattrname[attr], 2);
  int error = GRBgetdblattrelement(conRep->Cmodel, dattrname[attr],
                                   conRep->row_no, &value);
  if (error) throw GRBException("Constr::get", error);

  return value;
}

string
GRBConstr::get(GRB_StringAttr attr) const
{
  char     *value;

  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, sattrname[attr], 2);
  int error = GRBgetstrattrelement(conRep->Cmodel, sattrname[attr],
                                   conRep->row_no, &value);
  if (error) throw GRBException("Constr::get", error);

  return string(value);
}

void
GRBConstr::set(GRB_IntAttr attr,
               int         value)
{
  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, iattrname[attr], 2);
  int error = GRBsetintattrelement(conRep->Cmodel, iattrname[attr],
                                   conRep->row_no, value);
  if (error) throw GRBException("Constr::set", error);
}

void
GRBConstr::set(GRB_DoubleAttr attr,
               double         value)
{
  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, dattrname[attr], 2);
  int error = GRBsetdblattrelement(conRep->Cmodel, dattrname[attr],
                                   conRep->row_no, value);
  if (error) throw GRBException("Constr::set", error);
}

void
GRBConstr::set(GRB_CharAttr attr,
               char         value)
{
  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, cattrname[attr], 2);
  int error = GRBsetcharattrelement(conRep->Cmodel, cattrname[attr],
                                    conRep->row_no, value);
  if (error) throw GRBException("Constr::set", error);
}

void
GRBConstr::set(GRB_StringAttr attr,
               const string&  value)
{
  if (conRep == NULL || conRep->Cmodel == NULL || conRep->row_no < 0)
    throw GRBException("Constraint not in model", GRB_ERROR_NOT_IN_MODEL);
  checkattrsize(conRep->Cmodel, sattrname[attr], 2);
  int error = GRBsetstrattrelement(conRep->Cmodel, sattrname[attr],
                                   conRep->row_no, (char *) value.c_str());
  if (error) throw GRBException("Constr::set", error);
}

bool
GRBConstr::sameAs(GRBConstr c2)
{
  return (conRep == c2.conRep);
}
