/* Copyright 2020, Gurobi Optimization, LLC */

/* Use parameters that are associated with a model.

   A MIP is solved for a few seconds with different sets of parameters.
   The one with the smallest MIP gap is selected, and the optimization
   is resumed until the optimal solution is found.
*/

#include "gurobi_c++.h"
using namespace std;

int
main(int argc,
     char *argv[])
{
  if (argc < 2)
  {
    cout << "Usage: params_c++ filename" << endl;
    return 1;
  }

  GRBEnv* env = 0;
  GRBModel *bestModel = 0, *m = 0;
  try
  {
    // Read model and verify that it is a MIP
    env = new GRBEnv();
    m = new GRBModel(*env, argv[1]);
    if (m->get(GRB_IntAttr_IsMIP) == 0)
    {
      cout << "The model is not an integer program" << endl;
      return 1;
    }

    // Set a 2 second time limit
    m->set(GRB_DoubleParam_TimeLimit, 2);

    // Now solve the model with different values of MIPFocus
    bestModel = new GRBModel(*m);
    bestModel->optimize();
    for (int i = 1; i <= 3; ++i)
    {
      m->reset();
      m->set(GRB_IntParam_MIPFocus, i);
      m->optimize();
      if (bestModel->get(GRB_DoubleAttr_MIPGap) >
                  m->get(GRB_DoubleAttr_MIPGap))
      {
        swap(bestModel, m);
      }
    }

    // Finally, delete the extra model, reset the time limit and
    // continue to solve the best model to optimality
    delete m;
    m = 0;
    bestModel->set(GRB_DoubleParam_TimeLimit, GRB_INFINITY);
    bestModel->optimize();
    cout << "Solved with MIPFocus: " <<
    bestModel->get(GRB_IntParam_MIPFocus) << endl;

  }
  catch (GRBException e)
  {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  }
  catch (...)
  {
    cout << "Error during optimization" << endl;
  }

  delete bestModel;
  delete m;
  delete env;
  return 0;
}
