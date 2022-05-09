// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include <string.h>
#include <cstdlib>
#include <cassert>
#include "Common.h"
#include "attrprivate.h"

GRBBatch::GRBBatch()
{
  Cbatch = NULL;
  Cenv   = NULL;
}

GRBBatch::GRBBatch(const GRBEnv& env,
                   const string& batchID)
{
  Cenv = env.env;

  int error = GRBgetbatch(Cenv, batchID.c_str(), &Cbatch);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  // Environment gets copied
  Cenv = GRBgetbatchenv(Cbatch);
}

GRBBatch::~GRBBatch()
{
  GRBfreebatch(Cbatch);
}

int
GRBBatch::get(GRB_IntAttr attr) const
{
  int value;

  int error = GRBgetbatchintattr(Cbatch, iattrname[attr], &value);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  return value;
}

std::string
GRBBatch::get(GRB_StringAttr attr) const
{
  char *value;

  int error = GRBgetbatchstrattr(Cbatch, sattrname[attr], &value);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (value == NULL) return string("");
  else               return string(value);
}

void
GRBBatch::set(const char *attrname)
{
  unsigned attrflags = 0;

  int error = GRBgetbatchattrflags(Cbatch, attrname, &attrflags);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);

  if (attrflags & 16) {
    string message = "Attribute ";
    message += string(attrname) + " cannot be set";
    throw GRBException(message, GRB_ERROR_INVALID_ARGUMENT);
  }

  // Currently, no batch attribute is settable, we error out
  throw GRBException(string("Unexpected internal error"),
                     GRB_ERROR_INVALID_ARGUMENT);
}

void
GRBBatch::set(GRB_IntAttr attr, int val)
{
  set(iattrname[attr]);
}

void
GRBBatch::set(GRB_DoubleAttr attr, double val)
{
  set(dattrname[attr]);
}

void
GRBBatch::set(GRB_StringAttr attr, std::string val)
{
  set(sattrname[attr]);
}

void
GRBBatch::abort(void)
{
  int error = GRBabortbatch(Cbatch);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBBatch::discard(void)
{
  int error = GRBdiscardbatch(Cbatch);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBBatch::retry(void)
{
  int error = GRBretrybatch(Cbatch);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

void
GRBBatch::update(void)
{
  int error = GRBupdatebatch(Cbatch);
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}

string
GRBBatch::getJSONSolution(void)
{
  char *sval = NULL;

  int error = GRBgetbatchjsonsolution(Cbatch, &sval);
  if (error) {
    assert(!sval);
    if (sval) GRBfree(sval);
    throw GRBException(string(GRBgeterrormsg(Cenv)), error);
  }

  string rval = string(sval);
  GRBfree(sval); // IMPORTANT: this is in the DLL heap on Windows,
                 // so use GRBfree
  return rval;
}

void
GRBBatch::writeJSONSolution(string filename)
{
  int error = GRBwritebatchjsonsolution(Cbatch, filename.c_str());
  if (error) throw GRBException(string(GRBgeterrormsg(Cenv)), error);
}
