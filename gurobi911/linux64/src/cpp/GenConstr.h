// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_GENCONSTR_H_
#define _CPP_GENCONSTR_H_


class GRBGenConstrRep // private one
{
  private:
    GRBmodel*  Cmodel;
    int        num;
  public:
    friend class GRBGenConstr;
};

class GRBGenConstr
{
  private:

    GRBGenConstrRep* genconRep;

    GRBGenConstr(GRBmodel* xmodel, int genc);
    void setindex(int genc);
    int  getindex() const;
    void remove();

  public:

    friend class GRBModel;

    GRBGenConstr();
    int get(GRB_IntAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);
};
#endif
