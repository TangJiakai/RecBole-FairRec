// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_MODEL_H_
#define _CPP_MODEL_H_


class GRBEnv;
class GRBCallback;

class GRBModel
{
  private:

    GRBmodel    *Cmodel;
    GRBenv      *Cenv;
    GRBCallback *cb;

    int rows;
    int cols;
    int numsos;
    int numqconstrs;
    int numgenconstrs;
    int newranges;

    int updatemode;

    std::vector<GRBVar> vars;
    std::vector<GRBConstr> constrs;
    std::vector<GRBSOS> sos;
    std::vector<GRBQConstr> qconstrs;
    std::vector<GRBGenConstr> genconstrs;

// only for gurobi_c++.h const GRBModel& operator=(const GRBModel &xmodel);

    GRBModel();
    int  getupdmode();
    void populate(bool emptyModel=false);
    int* setvarsind(const GRBVar* xvars, int size);
    int* setconstrsind(const GRBConstr* xconstrs, int size);
    int* setqconstrsind(const GRBQConstr* xqconstrs, int size);
    int* setgenconstrsind(const GRBGenConstr* xgenconstrs, int size);
    GRBConstr addConstr(const GRBLinExpr&  expr, char sense,
                        double lhs, double rhs, const std::string& cname);
    GRBConstr* addConstrs(const GRBLinExpr* expr, const char* sense,
                          const double* lhs, const double* rhs,
                          const std::string* name, int len);
    GRBQConstr addQConstr(const GRBQuadExpr&  expr, char sense,
                          const std::string& cname);
    double feasRelaxP(int type, bool minrelax, int vlen, int clen,
                    const GRBVar* vars, const double* lbpen,
                    const double* ubpen, const GRBConstr* constrs,
                    const double* rhspen);

  public:

    GRBModel(const GRBEnv& env);
    GRBModel(const GRBEnv& env, const std::string& filename);
    GRBModel(const GRBModel& xmodel);

    ~GRBModel();

    void read(const std::string& filename);
    void write(const std::string& filename);

    void sync();

    GRBModel relax();
    GRBModel fixedModel();
    GRBModel presolve();
    GRBModel feasibility();
    GRBModel linearize();
    GRBModel singleScenarioModel();

    double feasRelax(int relaxobjtype, bool minrelax, bool vrelax, bool crelax);
    double feasRelax(int relaxobjtype, bool minrelax, int vlen,
                   const GRBVar* vars, const double* lbpen,
                   const double* ubpen, int clen, const GRBConstr* constrs,
                   const double* rhspen);

    void update();
    void optimize();
    std::string optimizeBatch();
    void optimizeasync();
    void computeIIS();
    void tune();
    void reset(int clearall = 0);
    void check();
    void terminate();

    void getTuneResult(int i);

    GRBQuadExpr getObjective() const;
    GRBLinExpr getObjective(int index) const;
    int  getPWLObj(GRBVar v, double *x, double *y) const;
    void setObjective(GRBLinExpr obje, int sense=0);
    void setObjective(GRBQuadExpr obje, int sense=0);
    void setObjectiveN(GRBLinExpr obj, int index, int priority=0,
                       double weight=1, double abstol=0, double reltol=0,
                       std::string name="");
    void setPWLObj(GRBVar v, int points, double *x, double *y);

    GRBVar getVar(int i) const;
    GRBVar* getVars() const;
    GRBVar getVarByName(const std::string& name);
    GRBConstr getConstr(int i) const;
    GRBConstr* getConstrs() const;
    GRBConstr getConstrByName(const std::string& name);
    GRBSOS* getSOSs() const;
    GRBQConstr* getQConstrs() const;
    GRBGenConstr* getGenConstrs() const;

    GRBVar addVar(double lb, double ub, double obj, char vtype,
                  std::string vname="");
    GRBVar addVar(double lb, double ub, double obj, char vtype,
                  int nonzeros, const GRBConstr* xconstrs,
                  const double* coeffs=NULL, std::string name="");
    GRBVar addVar(double lb, double ub, double obj, char vtype,
                  const GRBColumn& col, std::string name="");
    GRBVar* addVars(int cnt, char type=GRB_CONTINUOUS);
    GRBVar* addVars(const double* lb, const double* ub,
                    const double* obj, const char* type,
                    const std::string* name, int len);
    GRBVar* addVars(const double* lb, const double *ub,
                    const double* obj, const char* type,
                    const std::string* name, const GRBColumn*
                    col, int len);

    GRBConstr addConstr(const GRBLinExpr& expr1, char sense,
                        const GRBLinExpr& expr2,
                        std::string name="");
    GRBConstr addConstr(const GRBLinExpr& expr, char sense, GRBVar v,
                        std::string name="");
    GRBConstr addConstr(GRBVar v1, char sense, GRBVar v2,
                        std::string name="");
    GRBConstr addConstr(GRBVar v, char sense, double rhs,
                        std::string name="");
    GRBConstr addConstr(const GRBLinExpr& expr, char sense, double rhs,
                        std::string name="");
    GRBConstr addConstr(const GRBTempConstr& tc, std::string name="");
    GRBConstr addRange(const GRBLinExpr& expr, double lower, double upper,
                       std::string name="");
    GRBConstr* addConstrs(int cnt);
    GRBConstr* addConstrs(const GRBLinExpr* expr, const char* sense,
                          const double* rhs, const std::string* name,
                          int len);
    GRBConstr* addRanges(const GRBLinExpr* expr, const double* lower,
                         const double* upper, const std::string* name,
                         int len);
    GRBSOS addSOS(const GRBVar* xvars, const double* weight, int len, int type);
    GRBQConstr addQConstr(const GRBQuadExpr& expr1, char sense,
                          const GRBQuadExpr& expr2,
                          std::string name="");
    GRBQConstr addQConstr(const GRBTempConstr& tc, std::string name="");
    GRBQConstr addQConstr(const GRBQuadExpr&  expr, char sense, double rhs,
                          std::string name="");
    GRBGenConstr addGenConstrMax(GRBVar resvar, const GRBVar* xvars,
                                 int len, double constant=-GRB_INFINITY, std::string name="");
    GRBGenConstr addGenConstrMin(GRBVar resvar, const GRBVar* xvars,
                                 int len, double constant=GRB_INFINITY, std::string name="");
    GRBGenConstr addGenConstrAbs(GRBVar resvar, GRBVar argvar,
                                 std::string name="");
    GRBGenConstr addGenConstrAnd(GRBVar resvar, const GRBVar* xvars,
                                 int len, std::string name="");
    GRBGenConstr addGenConstrOr(GRBVar resvar, const GRBVar* xvars,
                                int len, std::string name="");
    GRBGenConstr addGenConstrIndicator(GRBVar binvar, int binval,
                                       const GRBLinExpr& expr, char sense, double rhs,
                                       std::string name="");
    GRBGenConstr addGenConstrIndicator(GRBVar binvar, int binval,
                                       const GRBTempConstr& constr,
                                       std::string name="");
    GRBGenConstr addGenConstrPWL(GRBVar xvar, GRBVar yvar, int npts, const double* xpts,
                                 const double* ypts, std::string name="");
    GRBGenConstr addGenConstrPoly(GRBVar xvar, GRBVar yvar, int plen, const double* p,
                                  std::string name="", std::string options="");
    GRBGenConstr addGenConstrExp(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrExpA(GRBVar xvar, GRBVar yvar, double a, std::string name="",
                                  std::string options="");
    GRBGenConstr addGenConstrLog(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrLogA(GRBVar xvar, GRBVar yvar, double a, std::string name="",
                                  std::string options="");
    GRBGenConstr addGenConstrPow(GRBVar xvar, GRBVar yvar, double a, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrSin(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrCos(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrTan(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");

    void remove(GRBVar v);
    void remove(GRBConstr c);
    void remove(GRBSOS xsos);
    void remove(GRBQConstr xqconstr);
    void remove(GRBGenConstr xgenconstr);

    void chgCoeff(GRBConstr c, GRBVar v, double val);
    void chgCoeffs(const GRBConstr* xconstrs, const GRBVar* xvars,
                   const double* val, int len);
    void chgCoeffs(const GRBConstr* xconstrs, const GRBVar* xvars,
                   const double* val, size_t len);
    double getCoeff(GRBConstr c, GRBVar v) const;
    GRBColumn getCol(GRBVar v);
    GRBLinExpr getRow(GRBConstr c);
    int getSOS(GRBSOS xsos, GRBVar* xvars, double* weight, int* typeP);
    void getGenConstrMax(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                         int* lenP, double* constantP);
    void getGenConstrMin(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                         int* lenP, double* constantP);
    void getGenConstrAbs(GRBGenConstr genc, GRBVar* resvarP, GRBVar* argvarP);
    void getGenConstrAnd(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                         int* lenP);
    void getGenConstrOr(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                        int* lenP);
    void getGenConstrIndicator(GRBGenConstr genc, GRBVar* binvarP, int* binvalP,
                               GRBLinExpr* exprP, char* senseP, double* rhsP);
    void getGenConstrPWL(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, int* nptsP,
                         double* xpts, double* ypts);
    void getGenConstrPoly(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, int* plenP, double* p);
    void getGenConstrExp(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrExpA(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, double* aP);
    void getGenConstrLog(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrLogA(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, double* aP);
    void getGenConstrPow(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, double* aP);
    void getGenConstrSin(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrCos(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrTan(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);

    GRBQuadExpr getQCRow(GRBQConstr c);
    GRBEnv getEnv() const;
    GRBEnv getConcurrentEnv(int num);
    void discardConcurrentEnvs();
    GRBEnv getMultiobjEnv(int num);
    void discardMultiobjEnvs();

    // Parameters

    int    get(GRB_IntParam param) const;
    double get(GRB_DoubleParam param) const;
    std::string get(GRB_StringParam param) const;

    void set(GRB_IntParam param, int val);
    void set(GRB_DoubleParam param, double val);
    void set(GRB_StringParam param, const std::string& val);
    void set(const std::string& param, const std::string& val);

    // Attributes

    int    get(GRB_IntAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int val);
    void set(GRB_DoubleAttr attr, double val);
    void set(GRB_StringAttr attr, const std::string& val);

    int*    get(GRB_IntAttr    attr, const GRBVar* xvars, int len);
    char*   get(GRB_CharAttr   attr, const GRBVar* xvars, int len);
    double* get(GRB_DoubleAttr attr, const GRBVar* xvars, int len);
    std::string* get(GRB_StringAttr attr, const GRBVar* xvars, int len);

    int*    get(GRB_IntAttr    attr, const GRBConstr* xconstrs, int len);
    char*   get(GRB_CharAttr   attr, const GRBConstr* xconstrs, int len);
    double* get(GRB_DoubleAttr attr, const GRBConstr* xconstrs, int len);
    std::string* get(GRB_StringAttr attr, const GRBConstr* xconstrs, int len);

    int*    get(GRB_IntAttr    attr, const GRBQConstr* xqconstrs, int len);
    char*   get(GRB_CharAttr   attr, const GRBQConstr* xqconstrs, int len);
    double* get(GRB_DoubleAttr attr, const GRBQConstr* xqconstrs, int len);
    std::string* get(GRB_StringAttr attr, const GRBQConstr* xqconstrs, int len);

    int*    get(GRB_IntAttr    attr, const GRBGenConstr* xgenconstrs, int len);
    std::string* get(GRB_StringAttr attr, const GRBGenConstr* xgenconstrs, int len);

    void prefetchAttr(GRB_IntAttr    attr);
    void prefetchAttr(GRB_CharAttr   attr);
    void prefetchAttr(GRB_DoubleAttr attr);
    void prefetchAttr(GRB_StringAttr attr);

    std::string getJSONSolution(void);

    void set(GRB_IntAttr    attr, const GRBVar* xvars,
             const int*    val, int len);
    void set(GRB_CharAttr   attr, const GRBVar* xvars,
             const char*   val, int len);
    void set(GRB_DoubleAttr attr, const GRBVar* xvars,
             const double* val, int len);
    void set(GRB_StringAttr attr, const GRBVar* xvars,
             const std::string* val, int len);

    void set(GRB_IntAttr    attr, const GRBConstr* xconstrs,
             const int*    val, int len);
    void set(GRB_CharAttr   attr, const GRBConstr* xconstrs,
             const char*   val, int len);
    void set(GRB_DoubleAttr attr, const GRBConstr* xconstrs,
             const double* val, int len);
    void set(GRB_StringAttr attr, const GRBConstr* xconstrs,
             const std::string* val, int len);

    void set(GRB_CharAttr   attr, const GRBQConstr* xconstrs,
             const char*   val, int len);
    void set(GRB_DoubleAttr attr, const GRBQConstr* xconstrs,
             const double* val, int len);
    void set(GRB_StringAttr attr, const GRBQConstr* xconstrs,
             const std::string* val, int len);

    void set(GRB_StringAttr attr, const GRBGenConstr* xqconstrs,
             const std::string* val, int len);

    void setCallback(GRBCallback* xcb);
};
#endif
