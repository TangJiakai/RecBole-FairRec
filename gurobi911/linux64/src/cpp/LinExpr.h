// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_LINEXPR_H_
#define _CPP_LINEXPR_H_


class GRBLinExpr: public GRBExpr
{
  private:

    double constant;
    std::vector<double> coeffs;
    std::vector<GRBVar> vars;
    void multAdd(double m, const GRBLinExpr& expr);

  public:

    GRBLinExpr(double constant=0.0);
    GRBLinExpr(GRBVar var, double coeff=1.0);

    friend class GRBQuadExpr;

    friend std::ostream& operator<<(std::ostream &stream, GRBLinExpr expr);
    friend GRBLinExpr operator+(const GRBLinExpr& x, const GRBLinExpr& y);
    friend GRBLinExpr operator+(const GRBLinExpr& x);
    friend GRBLinExpr operator+(GRBVar x, GRBVar y);
    friend GRBLinExpr operator+(GRBVar x, double a);
    friend GRBLinExpr operator+(double a, GRBVar x);
    friend GRBLinExpr operator-(const GRBLinExpr& x, const GRBLinExpr& y);
    friend GRBLinExpr operator-(const GRBLinExpr& x);
    friend GRBLinExpr operator-(GRBVar x);
    friend GRBLinExpr operator-(GRBVar x, GRBVar y);
    friend GRBLinExpr operator-(GRBVar x, double a);
    friend GRBLinExpr operator-(double a, GRBVar x);
    friend GRBLinExpr operator*(double a, GRBVar x);
    friend GRBLinExpr operator*(GRBVar x, double a);
    friend GRBLinExpr operator*(const GRBLinExpr& x, double a);
    friend GRBLinExpr operator*(double a, const GRBLinExpr& x);
    friend GRBLinExpr operator/(GRBVar x, double a);
    friend GRBLinExpr operator/(const GRBLinExpr& x, double a);

    unsigned int size(void) const;
    GRBVar getVar(int i) const;
    double getCoeff(int i) const;
    double getConstant() const;
    double getValue() const;

    void addTerms(const double* coeff, const GRBVar* var, int cnt);
    GRBLinExpr operator=(const GRBLinExpr& rhs);
    void operator+=(const GRBLinExpr& expr);
    void operator-=(const GRBLinExpr& expr);
    void operator*=(double mult);
    void operator/=(double a);
    GRBLinExpr operator+(const GRBLinExpr& rhs);
    GRBLinExpr operator-(const GRBLinExpr& rhs);
    void remove(int i);
    bool remove(GRBVar v);

    void clear();
};
#endif
