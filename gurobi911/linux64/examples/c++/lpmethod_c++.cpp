/* Copyright 2020, Gurobi Optimization, LLC */

/* Solve a model with different values of the Method parameter;
   show which value gives the shortest solve time. */

#include "gurobi_c++.h"
using namespace std;

int
main(int   argc,
     char *argv[])
{

  if (argc < 2)
  {
    cout << "Usage: lpmethod_c++ filename" << endl;
    return 1;
  }

  try {
    // Read model
    GRBEnv env = GRBEnv();
    GRBModel m = GRBModel(env, argv[1]);

    // Solve the model with different values of Method
    int    bestMethod = -1;
    double bestTime = m.get(GRB_DoubleParam_TimeLimit);
    for (int i = 0; i <= 2; ++i) {
      m.reset();
      m.set(GRB_IntParam_Method, i);
      m.optimize();
      if (m.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
        bestTime = m.get(GRB_DoubleAttr_Runtime);
        bestMethod = i;
        // Reduce the TimeLimit parameter to save time
        // with other methods
        m.set(GRB_DoubleParam_TimeLimit, bestTime);
      }
    }

    // Report which method was fastest
    if (bestMethod == -1) {
      cout << "Unable to solve this model" << endl;
    } else {
      cout << "Solved in " << bestTime
        << " seconds with Method: " << bestMethod << endl;
    }
  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }

  return 0;
}
