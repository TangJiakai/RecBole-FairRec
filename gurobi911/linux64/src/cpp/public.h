// Copyright (C) 2020, Gurobi Optimization, LLC
// All Rights Reserved
#include <iostream>
#include <vector>
#include <string>

extern "C" {
#include "gurobi_c.h"
}

#define GRB_ERROR_NOT_IN_MODEL            20001
#define GRB_ERROR_FAILED_TO_CREATE_MODEL  20002
#define GRB_ERROR_INTERNAL                20003

enum GRB_DoubleParam {
  GRB_DoubleParam_Cutoff,
  GRB_DoubleParam_IterationLimit,
  GRB_DoubleParam_NodeLimit,
  GRB_DoubleParam_TimeLimit,
  GRB_DoubleParam_FeasibilityTol,
  GRB_DoubleParam_IntFeasTol,
  GRB_DoubleParam_MarkowitzTol,
  GRB_DoubleParam_MIPGap,
  GRB_DoubleParam_MIPGapAbs,
  GRB_DoubleParam_OptimalityTol,
  GRB_DoubleParam_PerturbValue,
  GRB_DoubleParam_Heuristics,
  GRB_DoubleParam_ObjScale,
  GRB_DoubleParam_NodefileStart,
  GRB_DoubleParam_BarConvTol,
  GRB_DoubleParam_BarQCPConvTol,
  GRB_DoubleParam_PSDTol,
  GRB_DoubleParam_ImproveStartGap,
  GRB_DoubleParam_ImproveStartNodes,
  GRB_DoubleParam_ImproveStartTime,
  GRB_DoubleParam_FeasRelaxBigM,
  GRB_DoubleParam_TuneTimeLimit,
  GRB_DoubleParam_TuneCleanup,
  GRB_DoubleParam_PreSOS1BigM,
  GRB_DoubleParam_PreSOS2BigM,
  GRB_DoubleParam_PoolGap,
  GRB_DoubleParam_PoolGapAbs,
  GRB_DoubleParam_BestObjStop,
  GRB_DoubleParam_BestBdStop,
  GRB_DoubleParam_CSQueueTimeout,
  GRB_DoubleParam_FuncPieceError,
  GRB_DoubleParam_FuncPieceLength,
  GRB_DoubleParam_FuncPieceRatio,
  GRB_DoubleParam_FuncMaxVal,
  GRB_DoubleParam_NoRelHeurTime,
  GRB_DoubleParam_NoRelHeurWork
};

enum GRB_IntParam {
  GRB_IntParam_SolutionLimit,
  GRB_IntParam_Method,
  GRB_IntParam_ScaleFlag,
  GRB_IntParam_SimplexPricing,
  GRB_IntParam_Quad,
  GRB_IntParam_NormAdjust,
  GRB_IntParam_Sifting,
  GRB_IntParam_SiftMethod,
  GRB_IntParam_SubMIPNodes,
  GRB_IntParam_VarBranch,
  GRB_IntParam_Cuts,
  GRB_IntParam_CliqueCuts,
  GRB_IntParam_CoverCuts,
  GRB_IntParam_FlowCoverCuts,
  GRB_IntParam_FlowPathCuts,
  GRB_IntParam_GUBCoverCuts,
  GRB_IntParam_ImpliedCuts,
  GRB_IntParam_ProjImpliedCuts,
  GRB_IntParam_MIPSepCuts,
  GRB_IntParam_MIRCuts,
  GRB_IntParam_StrongCGCuts,
  GRB_IntParam_ModKCuts,
  GRB_IntParam_ZeroHalfCuts,
  GRB_IntParam_NetworkCuts,
  GRB_IntParam_SubMIPCuts,
  GRB_IntParam_InfProofCuts,
  GRB_IntParam_RelaxLiftCuts,
  GRB_IntParam_RLTCuts,
  GRB_IntParam_BQPCuts,
  GRB_IntParam_PSDCuts,
  GRB_IntParam_CutAggPasses,
  GRB_IntParam_CutPasses,
  GRB_IntParam_GomoryPasses,
  GRB_IntParam_NodeMethod,
  GRB_IntParam_Presolve,
  GRB_IntParam_Aggregate,
  GRB_IntParam_IISMethod,
  GRB_IntParam_PreCrush,
  GRB_IntParam_PreDepRow,
  GRB_IntParam_PrePasses,
  GRB_IntParam_DisplayInterval,
  GRB_IntParam_OutputFlag,
  GRB_IntParam_Threads,
  GRB_IntParam_BarIterLimit,
  GRB_IntParam_Crossover,
  GRB_IntParam_CrossoverBasis,
  GRB_IntParam_BarCorrectors,
  GRB_IntParam_BarOrder,
  GRB_IntParam_PumpPasses,
  GRB_IntParam_RINS,
  GRB_IntParam_Symmetry,
  GRB_IntParam_MIPFocus,
  GRB_IntParam_NumericFocus,
  GRB_IntParam_AggFill,
  GRB_IntParam_PreDual,
  GRB_IntParam_SolutionNumber,
  GRB_IntParam_MinRelNodes,
  GRB_IntParam_ZeroObjNodes,
  GRB_IntParam_BranchDir,
  GRB_IntParam_DegenMoves,
  GRB_IntParam_InfUnbdInfo,
  GRB_IntParam_DualReductions,
  GRB_IntParam_BarHomogeneous,
  GRB_IntParam_PreQLinearize,
  GRB_IntParam_MIQCPMethod,
  GRB_IntParam_NonConvex,
  GRB_IntParam_QCPDual,
  GRB_IntParam_LogToConsole,
  GRB_IntParam_PreSparsify,
  GRB_IntParam_PreMIQCPForm,
  GRB_IntParam_Seed,
  GRB_IntParam_ConcurrentMIP,
  GRB_IntParam_ConcurrentJobs,
  GRB_IntParam_DistributedMIPJobs,
  GRB_IntParam_LazyConstraints,
  GRB_IntParam_TuneResults,
  GRB_IntParam_TuneTrials,
  GRB_IntParam_TuneOutput,
  GRB_IntParam_TuneJobs,
  GRB_IntParam_TuneCriterion,
  GRB_IntParam_Disconnected,
  GRB_IntParam_UpdateMode,
  GRB_IntParam_Record,
  GRB_IntParam_ObjNumber,
  GRB_IntParam_MultiObjMethod,
  GRB_IntParam_MultiObjPre,
  GRB_IntParam_PoolSolutions,
  GRB_IntParam_PoolSearchMode,
  GRB_IntParam_ScenarioNumber,
  GRB_IntParam_StartNumber,
  GRB_IntParam_StartNodeLimit,
  GRB_IntParam_IgnoreNames,
  GRB_IntParam_PartitionPlace,
  GRB_IntParam_CSPriority,
  GRB_IntParam_CSTLSInsecure,
  GRB_IntParam_CSIdleTimeout,
  GRB_IntParam_ServerTimeout,
  GRB_IntParam_TSPort,
  GRB_IntParam_JSONSolDetail,
  GRB_IntParam_CSBatchMode,
  GRB_IntParam_FuncPieces,
  GRB_IntParam_CSClientLog,
  GRB_IntParam_IntegralityFocus
};

enum GRB_StringParam {
  GRB_StringParam_LogFile,
  GRB_StringParam_NodefileDir,
  GRB_StringParam_ResultFile,
  GRB_StringParam_WorkerPool,
  GRB_StringParam_WorkerPassword,
  GRB_StringParam_ComputeServer,
  GRB_StringParam_ServerPassword,
  GRB_StringParam_CSRouter,
  GRB_StringParam_CSGroup,
  GRB_StringParam_TokenServer,
  GRB_StringParam_CloudAccessID,
  GRB_StringParam_CloudSecretKey,
  GRB_StringParam_CloudPool,
  GRB_StringParam_CloudHost,
  GRB_StringParam_JobID,
  GRB_StringParam_CSManager,
  GRB_StringParam_CSAuthToken,
  GRB_StringParam_CSAPIAccessID,
  GRB_StringParam_CSAPISecret,
  GRB_StringParam_UserName,
  GRB_StringParam_CSAppName,
  GRB_StringParam_SolFiles,
  GRB_StringParam_Dummy
};

enum GRB_IntAttr {
  GRB_IntAttr_NumConstrs,
  GRB_IntAttr_NumVars,
  GRB_IntAttr_NumSOS,
  GRB_IntAttr_NumQConstrs,
  GRB_IntAttr_NumGenConstrs,
  GRB_IntAttr_NumNZs,
  GRB_IntAttr_NumQNZs,
  GRB_IntAttr_NumQCNZs,
  GRB_IntAttr_NumIntVars,
  GRB_IntAttr_NumBinVars,
  GRB_IntAttr_NumPWLObjVars,
  GRB_IntAttr_ModelSense,
  GRB_IntAttr_IsMIP,
  GRB_IntAttr_IsQP,
  GRB_IntAttr_IsQCP,
  GRB_IntAttr_IsMultiObj,
  GRB_IntAttr_Status,
  GRB_IntAttr_SolCount,
  GRB_IntAttr_BarIterCount,
  GRB_IntAttr_VBasis,
  GRB_IntAttr_CBasis,
  GRB_IntAttr_PWLObjCvx,
  GRB_IntAttr_BranchPriority,
  GRB_IntAttr_VarPreStat,
  GRB_IntAttr_BoundVioIndex,
  GRB_IntAttr_BoundSVioIndex,
  GRB_IntAttr_ConstrVioIndex,
  GRB_IntAttr_ConstrSVioIndex,
  GRB_IntAttr_ConstrResidualIndex,
  GRB_IntAttr_ConstrSResidualIndex,
  GRB_IntAttr_DualVioIndex,
  GRB_IntAttr_DualSVioIndex,
  GRB_IntAttr_DualResidualIndex,
  GRB_IntAttr_DualSResidualIndex,
  GRB_IntAttr_ComplVioIndex,
  GRB_IntAttr_IntVioIndex,
  GRB_IntAttr_IISMinimal,
  GRB_IntAttr_IISLB,
  GRB_IntAttr_IISUB,
  GRB_IntAttr_IISConstr,
  GRB_IntAttr_IISSOS,
  GRB_IntAttr_IISQConstr,
  GRB_IntAttr_IISGenConstr,
  GRB_IntAttr_TuneResultCount,
  GRB_IntAttr_Lazy,
  GRB_IntAttr_VarHintPri,
  GRB_IntAttr_ObjNPriority,
  GRB_IntAttr_NumObj,
  GRB_IntAttr_GenConstrType,
  GRB_IntAttr_NumStart,
  GRB_IntAttr_Partition,
  GRB_IntAttr_LicenseExpiration,
  GRB_IntAttr_NumScenarios,
  GRB_IntAttr_FuncPieces,
  GRB_IntAttr_BatchErrorCode,
  GRB_IntAttr_BatchStatus,
  GRB_IntAttr_Fingerprint
};

enum GRB_CharAttr {
  GRB_CharAttr_VType,
  GRB_CharAttr_Sense,
  GRB_CharAttr_QCSense
};

enum GRB_DoubleAttr {
  GRB_DoubleAttr_Runtime,
  GRB_DoubleAttr_ObjCon,
  GRB_DoubleAttr_LB,
  GRB_DoubleAttr_UB,
  GRB_DoubleAttr_Obj,
  GRB_DoubleAttr_Start,
  GRB_DoubleAttr_PreFixVal,
  GRB_DoubleAttr_RHS,
  GRB_DoubleAttr_QCRHS,
  GRB_DoubleAttr_MaxCoeff,
  GRB_DoubleAttr_MinCoeff,
  GRB_DoubleAttr_MaxBound,
  GRB_DoubleAttr_MinBound,
  GRB_DoubleAttr_MaxObjCoeff,
  GRB_DoubleAttr_MinObjCoeff,
  GRB_DoubleAttr_MaxRHS,
  GRB_DoubleAttr_MinRHS,
  GRB_DoubleAttr_MaxQCRHS,
  GRB_DoubleAttr_MinQCRHS,
  GRB_DoubleAttr_MaxQCCoeff,
  GRB_DoubleAttr_MinQCCoeff,
  GRB_DoubleAttr_MaxQCLCoeff,
  GRB_DoubleAttr_MinQCLCoeff,
  GRB_DoubleAttr_MaxQObjCoeff,
  GRB_DoubleAttr_MinQObjCoeff,
  GRB_DoubleAttr_ObjVal,
  GRB_DoubleAttr_ObjBound,
  GRB_DoubleAttr_ObjBoundC,
  GRB_DoubleAttr_MIPGap,
  GRB_DoubleAttr_IterCount,
  GRB_DoubleAttr_NodeCount,
  GRB_DoubleAttr_X,
  GRB_DoubleAttr_RC,
  GRB_DoubleAttr_Pi,
  GRB_DoubleAttr_QCPi,
  GRB_DoubleAttr_Slack,
  GRB_DoubleAttr_QCSlack,
  GRB_DoubleAttr_BoundVio,
  GRB_DoubleAttr_BoundSVio,
  GRB_DoubleAttr_BoundVioSum,
  GRB_DoubleAttr_BoundSVioSum,
  GRB_DoubleAttr_ConstrVio,
  GRB_DoubleAttr_ConstrSVio,
  GRB_DoubleAttr_ConstrVioSum,
  GRB_DoubleAttr_ConstrSVioSum,
  GRB_DoubleAttr_ConstrResidual,
  GRB_DoubleAttr_ConstrSResidual,
  GRB_DoubleAttr_ConstrResidualSum,
  GRB_DoubleAttr_ConstrSResidualSum,
  GRB_DoubleAttr_DualVio,
  GRB_DoubleAttr_DualSVio,
  GRB_DoubleAttr_DualVioSum,
  GRB_DoubleAttr_DualSVioSum,
  GRB_DoubleAttr_DualResidual,
  GRB_DoubleAttr_DualSResidual,
  GRB_DoubleAttr_DualResidualSum,
  GRB_DoubleAttr_DualSResidualSum,
  GRB_DoubleAttr_ComplVio,
  GRB_DoubleAttr_ComplVioSum,
  GRB_DoubleAttr_IntVio,
  GRB_DoubleAttr_IntVioSum,
  GRB_DoubleAttr_Kappa,
  GRB_DoubleAttr_KappaExact,
  GRB_DoubleAttr_SAObjLow,
  GRB_DoubleAttr_SAObjUp,
  GRB_DoubleAttr_SALBLow,
  GRB_DoubleAttr_SALBUp,
  GRB_DoubleAttr_SARHSLow,
  GRB_DoubleAttr_SAUBLow,
  GRB_DoubleAttr_SAUBUp,
  GRB_DoubleAttr_SARHSUp,
  GRB_DoubleAttr_Xn,
  GRB_DoubleAttr_FarkasProof,
  GRB_DoubleAttr_FarkasDual,
  GRB_DoubleAttr_UnbdRay,
  GRB_DoubleAttr_PStart,
  GRB_DoubleAttr_DStart,
  GRB_DoubleAttr_BarX,
  GRB_DoubleAttr_VarHintVal,
  GRB_DoubleAttr_ObjN,
  GRB_DoubleAttr_ObjNCon,
  GRB_DoubleAttr_ObjNWeight,
  GRB_DoubleAttr_ObjNRelTol,
  GRB_DoubleAttr_ObjNAbsTol,
  GRB_DoubleAttr_ObjNVal,
  GRB_DoubleAttr_PoolObjBound,
  GRB_DoubleAttr_PoolObjVal,
  GRB_DoubleAttr_ScenNLB,
  GRB_DoubleAttr_ScenNUB,
  GRB_DoubleAttr_ScenNObj,
  GRB_DoubleAttr_ScenNRHS,
  GRB_DoubleAttr_ScenNX,
  GRB_DoubleAttr_ScenNObjBound,
  GRB_DoubleAttr_ScenNObjVal,
  GRB_DoubleAttr_FuncPieceError,
  GRB_DoubleAttr_FuncPieceLength,
  GRB_DoubleAttr_FuncPieceRatio,
  GRB_DoubleAttr_DNumNZs
};

enum GRB_StringAttr {
  GRB_StringAttr_ModelName,
  GRB_StringAttr_VarName,
  GRB_StringAttr_ConstrName,
  GRB_StringAttr_QCName,
  GRB_StringAttr_GenConstrName,
  GRB_StringAttr_ObjNName,
  GRB_StringAttr_Server,
  GRB_StringAttr_JobID,
  GRB_StringAttr_ScenNName,
  GRB_StringAttr_BatchID,
  GRB_StringAttr_VTag,
  GRB_StringAttr_CTag,
  GRB_StringAttr_QCTag,
  GRB_StringAttr_BatchErrorMessage
};

class GRBVar;
class GRBExpr;
class GRBLinExpr;
class GRBQuadExpr;
class GRBConstr;
class GRBModel;
class GRBEnv;
class GRBException;
class GRBCallback;
class GRBSOS;
class GRBQConstr;
class GRBGenConstr;
class GRBColumn;
class GRBTempConstr;

std::ostream& operator<<(std::ostream &stream, GRBLinExpr expr);
GRBLinExpr operator+(const GRBLinExpr& x, const GRBLinExpr& y);
GRBLinExpr operator-(const GRBLinExpr& x, const GRBLinExpr& y);
GRBLinExpr operator+(const GRBLinExpr& x);
GRBLinExpr operator+(GRBVar x, GRBVar y);
GRBLinExpr operator+(GRBVar x, double a);
GRBLinExpr operator+(double a, GRBVar x);
GRBLinExpr operator-(const GRBLinExpr& x);
GRBLinExpr operator-(GRBVar x);
GRBLinExpr operator-(GRBVar x, GRBVar y);
GRBLinExpr operator-(GRBVar x, double a);
GRBLinExpr operator-(double a, GRBVar x);
GRBLinExpr operator*(double a, GRBVar x);
GRBLinExpr operator*(GRBVar x, double a);
GRBLinExpr operator*(const GRBLinExpr& x, double a);
GRBLinExpr operator*(double a, const GRBLinExpr& x);
GRBLinExpr operator/(GRBVar x, double a);
GRBLinExpr operator/(const GRBLinExpr& x, double a);

std::ostream& operator<<(std::ostream &stream, GRBQuadExpr expr);
GRBQuadExpr operator+(const GRBQuadExpr& x, const GRBQuadExpr& y);
GRBQuadExpr operator-(const GRBQuadExpr& x, const GRBQuadExpr& y);
GRBQuadExpr operator+(const GRBQuadExpr& x);
GRBQuadExpr operator-(const GRBQuadExpr& x);
GRBQuadExpr operator*(const GRBQuadExpr& x, double a);
GRBQuadExpr operator*(double a, const GRBQuadExpr& x);
GRBQuadExpr operator*(GRBVar x, GRBVar y);
GRBQuadExpr operator*(GRBVar x, const GRBLinExpr& y);
GRBQuadExpr operator*(const GRBLinExpr& y, GRBVar x);
GRBQuadExpr operator*(const GRBLinExpr& x, const GRBLinExpr& y);
GRBQuadExpr operator/(const GRBQuadExpr& x, double a);

GRBTempConstr operator<=(GRBQuadExpr x, GRBQuadExpr y);
GRBTempConstr operator>=(GRBQuadExpr x, GRBQuadExpr y);
GRBTempConstr operator==(GRBQuadExpr x, GRBQuadExpr y);
