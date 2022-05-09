// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_VAR_H_
#define _CPP_VAR_H_


class GRBVarRep // private one
{
  private:
    GRBmodel*  Cmodel;
    int        col_no;
  public:
    friend class GRBVar;
};

class GRBVar
{
  private:

    GRBVarRep* varRep;

    GRBVar(GRBmodel* xmodel, int xcol_no);
    void setcolno(int xcol_no);
    int  getcolno() const;
    void remove();

  public:

    friend class GRBModel;
    friend class GRBLinExpr;
    friend class GRBQuadExpr;
    friend class GRBCallback;

    GRBVar();
    int index() const;
    int get(GRB_IntAttr attr) const;
    char get(GRB_CharAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int value);
    void set(GRB_CharAttr attr, char value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);

    bool sameAs(GRBVar v2);
};
#endif
