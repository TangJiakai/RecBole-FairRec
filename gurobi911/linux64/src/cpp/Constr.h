// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_CONSTR_H_
#define _CPP_CONSTR_H_


class GRBConRep  // private one
{
  private:
    GRBmodel*  Cmodel;
    int        row_no;
  public:
    friend class GRBConstr;
};

class GRBConstr
{
  private:

    GRBConRep* conRep;

    GRBConstr(GRBmodel* xmodel, int xrow_no);
    void setrowno(int xrow_no);
    int  getrowno() const;
    void remove();

  public:

    friend class GRBModel;
    friend class GRBColumn;

    GRBConstr();
    int index() const;
    int get(GRB_IntAttr attr) const;
    char get(GRB_CharAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int value);
    void set(GRB_CharAttr attr, char value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);

    bool sameAs(GRBConstr c2);
};
#endif
