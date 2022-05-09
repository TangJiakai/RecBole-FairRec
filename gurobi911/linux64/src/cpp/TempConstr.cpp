// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include "Common.h"

GRBTempConstr
operator<=(GRBQuadExpr x, GRBQuadExpr y)
{
  GRBTempConstr tc = GRBTempConstr();
  tc.expr = x - y;
  tc.sense = GRB_LESS_EQUAL;
  return tc;
}
  
GRBTempConstr
operator>=(GRBQuadExpr x, GRBQuadExpr y)
{
  GRBTempConstr tc = GRBTempConstr();
  tc.expr = x - y;
  tc.sense = GRB_GREATER_EQUAL;
  return tc;
}

GRBTempConstr
operator==(GRBQuadExpr x, GRBQuadExpr y)
{
  GRBTempConstr tc = GRBTempConstr();
  tc.expr = x - y;
  tc.sense = GRB_EQUAL;
  return tc;
}
