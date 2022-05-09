// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved

#include "Common.h"
#include "parprivate.h"

#define APITYPE_CPP 1

GRBEnv::GRBEnv(GRBenv* Cenv)
{
  env = Cenv;
  envP = NULL;
}

GRBEnv::GRBEnv(const bool empty)
{
  int error;
  if (empty) {
    error = GRBemptyenvadv(&env, APITYPE_CPP, GRB_VERSION_MAJOR,
                           GRB_VERSION_MINOR, GRB_VERSION_TECHNICAL,
                           NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  } else {
    error = GRBloadenvadv(&env, NULL, APITYPE_CPP, GRB_VERSION_MAJOR,
                          GRB_VERSION_MINOR, GRB_VERSION_TECHNICAL,
                          NULL, NULL, NULL, NULL, -9999999, -1,
                          NULL, NULL, NULL, NULL, NULL);
  }

  if (error) throw GRBException(getErrorMsg(), error);
  envP = &env;
}

GRBEnv::GRBEnv(const char* logfilename)
{
  int error = GRBloadenvadv(&env, logfilename, APITYPE_CPP, GRB_VERSION_MAJOR,
                            GRB_VERSION_MINOR, GRB_VERSION_TECHNICAL,
                            NULL, NULL, NULL, NULL, -9999999, -1,
                            NULL, NULL, NULL, NULL, NULL);
  if (error) throw GRBException(getErrorMsg(), error);
  envP = &env;
}

GRBEnv::GRBEnv(const string& logfilename)
{
  int error = GRBloadenvadv(&env, logfilename.c_str(), APITYPE_CPP,
                            GRB_VERSION_MAJOR, GRB_VERSION_MINOR,
                            GRB_VERSION_TECHNICAL,
                            NULL, NULL, NULL, NULL, -9999999, -1,
                            NULL, NULL, NULL, NULL, NULL);
  if (error) throw GRBException(getErrorMsg(), error);
  envP = &env;
}

GRBEnv::GRBEnv(const string& logfilename,
               const string& computeserver,
               const string& router,
               const string& password,
               const string& group,
               int           CStlsInsecure,
               int           priority,
               double        timeout)
{
  int error = GRBloadclientenvadv(&env, logfilename.c_str(),
                                  computeserver.c_str(), router.c_str(),
                                  password.c_str(), group.c_str(),
                                  CStlsInsecure, priority, timeout, APITYPE_CPP,
                                  GRB_VERSION_MAJOR, GRB_VERSION_MINOR,
                                  GRB_VERSION_TECHNICAL, NULL, NULL);
  if (error) throw GRBException(getErrorMsg(), error);
  envP = &env;
}

GRBEnv::GRBEnv(const string& logfilename,
               const string& accessID,
               const string& secretKey,
               const string& pool,
               int           priority)
{
  int error = GRBloadcloudenvadv(&env, logfilename.c_str(), accessID.c_str(),
                                 secretKey.c_str(), pool.c_str(), priority,
                                 APITYPE_CPP,
                                 GRB_VERSION_MAJOR, GRB_VERSION_MINOR,
                                 GRB_VERSION_TECHNICAL, NULL, NULL);
  if (error) throw GRBException(getErrorMsg(), error);
  envP = &env;
}

GRBEnv::~GRBEnv()
{
  if (envP != NULL && envP == &env) {
    GRBfreeenv(env);
    env = NULL;
  }
}

void
GRBEnv::message(const string& msg)
{
  GRBmsg(env, msg.c_str());
}

void
GRBEnv::start()
{
  int error = GRBstartenv(env);
  if (error) throw GRBException(getErrorMsg(), error);
}

int
GRBEnv::get(GRB_IntParam param) const
{
  int value;
  int error = GRBgetintparam(env, iparname[param], &value);
  if (error) throw GRBException(getErrorMsg(), error);
  return value;
}

double
GRBEnv::get(GRB_DoubleParam param) const
{
  double value;
  int error = GRBgetdblparam(env, dparname[param], &value);
  if (error) throw GRBException(getErrorMsg(), error);
  return value;
}

string
GRBEnv::get(GRB_StringParam param) const
{
  char value[GRB_MAX_STRLEN];
  int  requiredlen;
  int  error = 0;

  error = GRBgetlongstrparam(env, sparname[param], value,
                             GRB_MAX_STRLEN, &requiredlen);
  if (error) throw GRBException(getErrorMsg(), error);

  if (requiredlen < GRB_MAX_STRLEN)
    return string(value);

  char *longvalue = new char[requiredlen+1];

  error = GRBgetlongstrparam(env, sparname[param], longvalue,
                             requiredlen+1, &requiredlen);
  if (error) throw GRBException(getErrorMsg(), error);

  string val = string(longvalue);
  delete[] longvalue;

  return val;
}

void
GRBEnv::set(GRB_IntParam param, int newvalue)
{
  int error = GRBsetintparam(env, iparname[param], newvalue);
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::set(GRB_DoubleParam param, double newvalue)
{
  int error = GRBsetdblparam(env, dparname[param], newvalue);
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::set(GRB_StringParam param, const string& newvalue)
{
  int error = GRBsetstrparam(env, sparname[param], newvalue.c_str());
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::set(const string& paramname, const string& newvalue)
{
  int error = GRBsetparam(env, paramname.c_str(), newvalue.c_str());
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::getParamInfo(GRB_DoubleParam param, double* valP,
                     double* minP, double* maxP, double* defP)
{
  int error = GRBgetdblparaminfo(env, dparname[param], valP, minP,
                                 maxP, defP);
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::getParamInfo(GRB_IntParam param, int* valP, int* minP,
                     int* maxP, int* defP)
{
  int error = GRBgetintparaminfo(env, iparname[param], valP, minP,
                                 maxP, defP);
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::getParamInfo(GRB_StringParam param, string& value,
                     string& defvalue)
{
  char cvalue[GRB_MAX_STRLEN];
  char cdefvalue[GRB_MAX_STRLEN];
  int error = GRBgetstrparaminfo(env, sparname[param],
                                 cvalue, cdefvalue);
  if (error) throw GRBException(getErrorMsg(), error);
  value    = string(cvalue);
  defvalue = string(cdefvalue);
}

void
GRBEnv::resetParams()
{
  int error = GRBresetparams(env);
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::writeParams(const string& paramfile)
{
  int error = GRBwriteparams(env, paramfile.c_str());
  if (error) throw GRBException(getErrorMsg(), error);
}

void
GRBEnv::readParams(const string& paramfile)
{
  int error = GRBreadparams(env, paramfile.c_str());
  if (error) throw GRBException(getErrorMsg(), error);
}

const string
GRBEnv::getErrorMsg() const
{
  const char* str = GRBgeterrormsg(env);
  return string(str);
}
