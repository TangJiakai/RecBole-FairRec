// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"
#include "attrprivate.h"

GRBSOS::GRBSOS()
{ 
  sosRep = NULL;
}

GRBSOS::GRBSOS(GRBmodel* xmodel,
               int       sos)
{
  sosRep = new GRBSOSRep();
  sosRep->Cmodel = xmodel;
  sosRep->num = sos;
}

void GRBSOS::remove()
{
  if (sosRep != NULL) {
    int j = sosRep->num;
    if      (j >=  0) sosRep->num = -3 -j;
    else if (j == -1) throw
      GRBException("not in model for removing", GRB_ERROR_NOT_IN_MODEL);
  }
  sosRep = NULL;
}

void
GRBSOS::setindex(int xnum)
{
  sosRep->num = xnum;
}

int
GRBSOS::getindex() const
{
  if (sosRep == NULL) return -2;
  else                return sosRep->num;
}

int
GRBSOS::get(GRB_IntAttr attr) const
{
  int value;

  if (sosRep == NULL || sosRep->Cmodel == NULL || sosRep->num < 0)
    throw GRBException("SOS set not in model", GRB_ERROR_NOT_IN_MODEL);

  /* dummy code to avoid warning */
  if (sosRep->num == -1) 
    std::cout << cattrname[0] << dattrname[0] << sattrname[0] << std::endl;

  checkattrsize(sosRep->Cmodel, iattrname[attr], 3);
  int error = GRBgetintattrelement(sosRep->Cmodel, iattrname[attr],
                                   sosRep->num, &value);
  if (error) throw GRBException("SOS::get", error);

  return value;
}
  

