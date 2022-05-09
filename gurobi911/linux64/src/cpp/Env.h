// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_ENV_H_
#define _CPP_ENV_H_


class GRBEnv
{
  private:

    GRBenv*  env;
    GRBenv** envP;

// only for gurobi_c++.h const GRBEnv& operator=(const GRBEnv &xenv);

    GRBEnv(GRBenv *Cenv);

  public:

    friend class GRBModel;
    friend class GRBBatch;

    GRBEnv(const bool empty = false);
    GRBEnv(const char* logfilename);
    GRBEnv(const std::string& logfilename);
    GRBEnv(const std::string& logfilename, const std::string& computeserver,
           const std::string& router, const std::string& password,
           const std::string& group, int CStlsInsecure, int priority,
           double timeout);
    GRBEnv(const std::string& logfilename, const std::string& accessID,
           const std::string& secretKey, const std::string& pool,
           int priority);
    GRBEnv(const std::string&, const std::string&, const std::string&, int, const std::string&);
    GRBEnv(const std::string&, const std::string&, const std::string&, int, const std::string&,
           void* (__stdcall *)(MALLOCCB_ARGS),
           void* (__stdcall *)(CALLOCCB_ARGS),
           void* (__stdcall *)(REALLOCCB_ARGS),
           void  (__stdcall *)(FREECB_ARGS),
           int   (__stdcall *)(THREADCREATECB_ARGS),
           void  (__stdcall *)(THREADJOINCB_ARGS),
           void*);
    ~GRBEnv();
    void start();
    void message(const std::string& msg);
    int get(GRB_IntParam param) const;
    double get(GRB_DoubleParam param) const;
    std::string get(GRB_StringParam param) const;
    void set(GRB_IntParam param, int newvalue);
    void set(GRB_DoubleParam param, double newvalue);
    void set(GRB_StringParam param, const std::string& newvalue);
    void set(const std::string& paramname, const std::string& newvalue);
    void getParamInfo(GRB_DoubleParam param, double* valP,
                      double* minP, double* maxP, double* defP);
    void getParamInfo(GRB_IntParam param, int* valP, int* minP,
                      int* maxP, int* defP);
    void getParamInfo(GRB_StringParam param, std::string& value,
                      std::string& defvalue);
    void resetParams();
    void writeParams(const std::string& paramfile);
    void readParams(const std::string& paramfile);
    const std::string getErrorMsg() const;
};
#endif
