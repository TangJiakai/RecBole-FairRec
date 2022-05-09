// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_CALLBACK_H_
#define _CPP_CALLBACK_H_


class GRBCallback
{
  private:

    GRBmodel*   Cmodel;
    int         cols;
    void*       cbdata;
    double*     x;
    double*     newx;
    double*     relx;

    static int __stdcall xcb(GRBmodel *xmodel, void *xcbdata,
                             int xwhere, void *xuserdata);
    void        setcb(GRBCallback *cb, GRBmodel *xmodel, int xcols);
    void        addCutOrLazy(const GRBLinExpr& expr, char sense,
                             double rhs, bool isCut);

  public:

    friend void GRBModel::setCallback(GRBCallback* cb);
    friend void GRBModel::update();

    GRBCallback();
    virtual ~GRBCallback() {};

  protected:

    int where;
    virtual void callback() {};
    double getDoubleInfo(int what);
    int getIntInfo(int what);
    const std::string getStringInfo(int what) const;
    double getSolution(GRBVar v);
    double* getSolution(const GRBVar* xvars, int len);
    double getNodeRel(GRBVar v);
    double* getNodeRel(const GRBVar* xvars, int len);
    void setSolution(GRBVar v, double val);
    void setSolution(const GRBVar* xvars, const double* sol, int len);
    double useSolution();
    void addCut(const GRBTempConstr& tc);
    void addCut(const GRBLinExpr& expr, char sense, double rhs);
    void addLazy(const GRBTempConstr& tc);
    void addLazy(const GRBLinExpr& expr, char sense, double rhs);
    void abort();
    void stopOneMultiObj(int objnum);
};
#endif
