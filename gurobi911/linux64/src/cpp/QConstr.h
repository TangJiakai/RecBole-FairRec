// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_QCONSTR_H_
#define _CPP_QCONSTR_H_


class GRBQConstrRep // private one
{
  private:
    GRBmodel*  Cmodel;
    int        num;
  public:
    friend class GRBQConstr;
};

class GRBQConstr
{
  private:

    GRBQConstrRep* qconRep;

    GRBQConstr(GRBmodel* xmodel, int qc);
    void setindex(int qc);
    int  getindex() const;
    void remove();

  public:

    friend class GRBModel;

    GRBQConstr();
    char get(GRB_CharAttr attr) const;
    int get(GRB_IntAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_CharAttr attr, char value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);
};
#endif
