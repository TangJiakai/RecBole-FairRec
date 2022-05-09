// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_COLUMN_H_
#define _CPP_COLUMN_H_


class GRBColumn
{
  private:

    std::vector<double> coeffs;
    std::vector<GRBConstr> constrs;

  public:

    unsigned int size(void) const;
    GRBConstr getConstr(int i) const;
    double getCoeff(int i) const;

    void addTerm(double coeff, GRBConstr constr);
    void addTerms(const double* coeff, const GRBConstr* constr, int cnt);
    void remove(int i);
    bool remove(GRBConstr c);

    void clear();
};
#endif
