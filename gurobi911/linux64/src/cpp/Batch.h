// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_BATCH_H_
#define _CPP_BATCH_H_


class GRBEnv;

class GRBBatch
{
  private:

    GRBenv   *Cenv;
    GRBbatch *Cbatch;

    GRBBatch();

    void set(const char *attrname);

  public:

    // constructor
    GRBBatch(const GRBEnv& env, const std::string& batchID);

    // destructor
    ~GRBBatch();

    // Attributes
    int         get(GRB_IntAttr    attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr    attr, int         val);
    void set(GRB_DoubleAttr attr, double      val);
    void set(GRB_StringAttr attr, std::string val);

    // Control functions
    void abort(void);
    void discard(void);
    void retry(void);
    void update(void);

    // Output functions
    std::string getJSONSolution(void);
    void        writeJSONSolution(std::string filename);
};
#endif
