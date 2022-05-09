// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#ifndef _CPP_EXCEPTION_H_
#define _CPP_EXCEPTION_H_


class GRBException
{
  private:

    std::string msg;
    int error;

  public:

    GRBException(int errcode = 0);
    GRBException(std::string errmsg, int errcode = 0);

    const std::string getMessage() const;
    int getErrorCode() const;
};
#endif
