// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include <string.h>
#include <cstdlib>
#include "Common.h"
#include "attrprivate.h"

void
checkattrsize(GRBmodel*   Cmodel,
              const char* attrname,
              int         size)
{
  int i;
  int error = GRBgetattrinfo(Cmodel, attrname, NULL, &i, NULL);
  if (error == 0 && i != size) error = GRB_ERROR_INVALID_ARGUMENT;
  if (error) throw GRBException("Not right attribute", error);
}
