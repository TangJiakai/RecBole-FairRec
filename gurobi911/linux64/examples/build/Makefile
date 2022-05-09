# Copyright 2020, Gurobi Optimization, LLC

PLATFORM = linux64
TWOUP    = ..
KEEPTU   = ../..
INC      = $(TWOUP)/../include/
CC       = gcc
CPP      = g++
CARGS    = -m64 -g
CLIB     = -L$(TWOUP)/../lib -lgurobi91
CPPLIB   = -L$(TWOUP)/../lib -lgurobi_c++ -lgurobi91
JSRC     = $(TWOUP)/java
CLASSDIR = -classpath $(TWOUP)/../lib/gurobi.jar:.
JFLAG    = -d . $(CLASSDIR)
GRBAPP   = DOTNETCore2
DOTNETFRAMEWORK = --framework=netcoreapp2.1

all: batchmode_c genconstr_c poolsearch_c bilinear_c gc_pwl_c gc_pwl_func_c \
     callback_c dense_c diet_c facility_c feasopt_c fixanddive_c \
     lp_c lpmethod_c lpmod_c mip1_c mip2_c params_c piecewise_c qcp_c \
     qp_c sensitivity_c sos_c sudoku_c tsp_c tune_c multiobj_c multiscenario_c \
     workforce1_c workforce2_c workforce3_c workforce4_c workforce5_c \
     bilinear_c++ callback_c++ dense_c++ diet_c++ facility_c++ feasopt_c++ \
     fixanddive_c++ lp_c++ lpmethod_c++ lpmod_c++ mip1_c++ mip2_c++ \
     params_c++ piecewise_c++ qcp_c++ qp_c++ sensitivity_c++ sos_c++ \
     sudoku_c++ tsp_c++ tune_c++ workforce1_c++ workforce2_c++ \
     workforce3_c++ workforce4_c++ workforce5_c++ multiobj_c++ multiscenario_c++ \
     genconstr_c++ poolsearch_c++ gc_pwl_c++ gc_pwl_func_c++ \
     batchmode_c++ \
     Bilinear.class Callback.class Dense.class Diet.class Facility.class \
     Feasopt.class Fixanddive.class Lp.class Lpmethod.class Lpmod.class \
     Mip1.class Mip2.class Params.class Piecewise.class Qcp.class Qp.class \
     Sensitivity.class Sos.class Sudoku.class Tsp.class Tune.class \
     Workforce1.class Workforce2.class Workforce3.class Workforce4.class \
     Workforce5.class Multiobj.class Multiscenario.class Poolsearch.class \
     Genconstr.class GCPWL.class GCPWLFunc.class Batchmode.class

run: run_c run_c++ run_java run_python #run_dotnet

run_c: run_bilinear_c run_callback_c run_dense_c run_diet_c run_facility_c \
       run_feasopt_c run_fixanddive_c run_lp_c run_lpmethod_c run_lpmod_c \
       run_mip1_c run_mip2_c run_params_c run_piecewise_c run_qcp_c run_qp_c \
       run_sensitivity_c run_sos_c run_sudoku_c run_tsp_c run_tune_c \
       run_workforce1_c run_workforce2_c run_workforce3_c run_workforce4_c \
       run_workforce5_c run_multiobj_c run_multiscenario_c run_poolsearch_c \
       run_genconstr_c run_gc_pwl_c run_gc_pwl_func_c

run_c++: run_bilinear_c++ run_callback_c++ run_dense_c++ run_diet_c++ \
         run_facility_c++ run_feasopt_c++ run_fixanddive_c++ run_lp_c++ \
         run_lpmethod_c++ run_lpmod_c++ run_mip1_c++ run_mip2_c++ \
         run_params_c++ run_piecewise_c++ run_qcp_c++ run_qp_c++ \
         run_sensitivity_c++ run_sos_c++ run_sudoku_c++ run_tsp_c++ \
         run_tune_c++ run_workforce1_c++ run_workforce2_c++ \
         run_workforce3_c++ run_workforce4_c++ run_workforce5_c++ \
         run_multiscenario_c++ run_multiobj_c++ run_poolsearch_c++ \
	 run_genconstr_c++ run_gc_pwl_c++ run_gc_pwl_func_c++

run_java: Bilinear Callback Dense Diet Facility Feasopt Fixanddive Lp \
          Lpmethod Lpmod Mip1 Mip2 Params Piecewise Qcp Qp Sensitivity \
          Sos Sudoku Tsp Tune Workforce1 Workforce2 Workforce3 Workforce4 \
          Workforce5 Multiobj Multiscenario Poolsearch Genconstr GCPWL GCPWLFunc

run_python: run_bilinear_python run_callback_python run_dense_python \
        run_diet_python run_diet2_python run_diet3_python run_diet4_python \
        run_facility_python run_feasopt_python run_fixanddive_python \
        run_lp_python run_lpmethod_python run_lpmod_python run_mip1_python \
        run_mip1_remote_python run_mip2_python run_netflow_python \
        run_params_python run_piecewise_python run_qcp_python run_qp_python \
        run_sensitivity_python run_sos_python run_sudoku_python \
        run_tsp_python run_tune_python run_workforce1_python \
        run_workforce2_python run_workforce3_python \
        run_workforce4_python run_workforce5_python run_genconstr_python \
        run_poolsearch_python run_multiobj_python run_multiscenario_python \
	run_custom_python run_gc_pwl_python run_gc_pwl_func_python

# these tests need scipy and numpy packages installed
run_python3: run_python run_matrix1_python run_matrix2_python

run_dotnet: run_bilinear_cs run_callback_cs run_dense_cs run_diet_cs \
        run_facility_cs run_feasopt_cs run_fixanddive_cs run_lp_cs \
        run_lpmethod_cs run_lpmod_cs run_mip1_cs run_mip2_cs run_params_cs \
        run_piecewise_cs run_qcp_cs run_qp_cs run_sensitivity_cs \
        run_sos_cs run_sudoku_cs run_tsp_cs run_tune_cs \
        run_workforce1_cs run_workforce2_cs run_workforce3_cs \
        run_workforce4_cs run_workforce5_cs run_genconstr_cs \
        run_poolsearch_cs run_multiobj_cs run_multiscenario_cs \
	run_gc_pwl_cs run_gc_pwl_func_cs



%_c: $(TWOUP)/c/%_c.c
	$(CC) $(CARGS) -o $@ $< -I$(INC) $(CLIB) -lm

%_c++: $(TWOUP)/c++/%_c++.cpp
	$(CPP) $(CARGS) -o $@ $< -I$(INC) $(CPPLIB) -lm

%.class: $(JSRC)/%.java
	javac $(JFLAG) $<

run_batchmode_c: batchmode_c
	./batchmode_c $(TWOUP)/data/p0033.mps
run_bilinear_c: bilinear_c
	./bilinear_c
run_callback_c: callback_c
	./callback_c $(TWOUP)/data/p0033.mps
run_dense_c: dense_c
	./dense_c
run_diet_c: diet_c
	./diet_c
run_facility_c: facility_c
	./facility_c
run_feasopt_c: feasopt_c
	./feasopt_c $(TWOUP)/data/klein1.mps
run_fixanddive_c: fixanddive_c
	./fixanddive_c $(TWOUP)/data/stein9.mps
run_lp_c: lp_c
	./lp_c $(TWOUP)/data/klein1.mps
run_lpmethod_c: lpmethod_c
	./lpmethod_c $(TWOUP)/data/afiro.mps
run_lpmod_c: lpmod_c
	./lpmod_c $(TWOUP)/data/afiro.mps
run_mip1_c: mip1_c
	./mip1_c
run_mip2_c: mip2_c
	./mip2_c $(TWOUP)/data/stein9.mps
run_params_c: params_c
	./params_c $(TWOUP)/data/misc07.mps
run_piecewise_c: piecewise_c
	./piecewise_c
run_qcp_c: qcp_c
	./qcp_c
run_qp_c: qp_c
	./qp_c
run_sensitivity_c: sensitivity_c
	./sensitivity_c $(TWOUP)/data/p0033.lp
run_sos_c: sos_c
	./sos_c
run_sudoku_c: sudoku_c
	./sudoku_c $(TWOUP)/data/sudoku1
run_tsp_c: tsp_c
	./tsp_c 50
run_tune_c: tune_c
	./tune_c $(TWOUP)/data/p0033.mps
run_workforce1_c: workforce1_c
	./workforce1_c
run_workforce2_c: workforce2_c
	./workforce2_c
run_workforce3_c: workforce3_c
	./workforce3_c
run_workforce4_c: workforce4_c
	./workforce4_c
run_workforce5_c: workforce5_c
	./workforce5_c
run_multiobj_c: multiobj_c
	./multiobj_c
run_multiscenario_c: multiscenario_c
	./multiscenario_c
run_poolsearch_c: poolsearch_c
	./poolsearch_c
run_genconstr_c: genconstr_c
	./genconstr_c
run_gc_pwl_c: gc_pwl_c
	./gc_pwl_c
run_gc_pwl_func_c: gc_pwl_func_c
	./gc_pwl_func_c

run_batchmode_c++: batchmode_c++
	./batchmode_c++ $(TWOUP)/data/p0033.mps
run_bilinear_c++: bilinear_c++
	./bilinear_c++
run_callback_c++: callback_c++
	./callback_c++ $(TWOUP)/data/p0033.mps
run_dense_c++: dense_c++
	./dense_c++
run_diet_c++: diet_c++
	./diet_c++
run_facility_c++: facility_c++
	./facility_c++
run_feasopt_c++: feasopt_c++
	./feasopt_c++ $(TWOUP)/data/klein1.mps
run_fixanddive_c++: fixanddive_c++
	./fixanddive_c++ $(TWOUP)/data/stein9.mps
run_lp_c++: lp_c++
	./lp_c++ $(TWOUP)/data/klein1.mps
run_lpmethod_c++: lpmethod_c++
	./lpmethod_c++ $(TWOUP)/data/afiro.mps
run_lpmod_c++: lpmod_c++
	./lpmod_c++ $(TWOUP)/data/afiro.mps
run_mip1_c++: mip1_c++
	./mip1_c++
run_mip2_c++: mip2_c++
	./mip2_c++ $(TWOUP)/data/stein9.mps
run_params_c++: params_c++
	./params_c++ $(TWOUP)/data/misc07.mps
run_piecewise_c++: piecewise_c++
	./piecewise_c++
run_qcp_c++: qcp_c++
	./qcp_c++
run_qp_c++: qp_c++
	./qp_c++
run_sensitivity_c++: sensitivity_c++
	./sensitivity_c++ $(TWOUP)/data/p0033.lp
run_sos_c++: sos_c++
	./sos_c++
run_sudoku_c++: sudoku_c++
	./sudoku_c++ < $(TWOUP)/data/sudoku1
run_tsp_c++: tsp_c++
	./tsp_c++ 50
run_tune_c++: tune_c++
	./tune_c++ $(TWOUP)/data/p0033.mps
run_workforce1_c++: workforce1_c++
	./workforce1_c++
run_workforce2_c++: workforce2_c++
	./workforce2_c++
run_workforce3_c++: workforce3_c++
	./workforce3_c++
run_workforce4_c++: workforce4_c++
	./workforce4_c++
run_workforce5_c++: workforce5_c++
	./workforce5_c++
run_multiobj_c++: multiobj_c++
	./multiobj_c++
run_multiscenario_c++: multiscenario_c++
	./multiscenario_c++
run_poolsearch_c++: poolsearch_c++
	./poolsearch_c++
run_genconstr_c++: genconstr_c++
	./genconstr_c++
run_gc_pwl_c++: gc_pwl_c++
	./gc_pwl_c++
run_gc_pwl_func_c++: gc_pwl_func_c++
	./gc_pwl_func_c++

Bilinear: Bilinear.class
	java $(CLASSDIR): Bilinear
Callback: Callback.class
	java $(CLASSDIR): Callback $(TWOUP)/data/p0033.mps
Dense: Dense.class
	java $(CLASSDIR): Dense
Diet: Diet.class
	java $(CLASSDIR): Diet
Facility: Facility.class
	java $(CLASSDIR): Facility
Feasopt: Feasopt.class
	java $(CLASSDIR): Feasopt $(TWOUP)/data/klein1.mps
Fixanddive: Fixanddive.class
	java $(CLASSDIR): Fixanddive $(TWOUP)/data/stein9.mps
Lp: Lp.class
	java $(CLASSDIR): Lp $(TWOUP)/data/klein1.mps
Lpmethod: Lpmethod.class
	java $(CLASSDIR): Lpmethod $(TWOUP)/data/afiro.mps
Lpmod: Lpmod.class
	java $(CLASSDIR): Lpmod $(TWOUP)/data/afiro.mps
Mip1: Mip1.class
	java $(CLASSDIR): Mip1
Mip2: Mip2.class
	java $(CLASSDIR): Mip2 $(TWOUP)/data/stein9.mps
Params: Params.class
	java $(CLASSDIR): Params $(TWOUP)/data/misc07.mps
Piecewise: Piecewise.class
	java $(CLASSDIR): Piecewise
Qcp: Qcp.class
	java $(CLASSDIR): Qcp
Qp: Qp.class
	java $(CLASSDIR): Qp
Sensitivity: Sensitivity.class
	java $(CLASSDIR): Sensitivity $(TWOUP)/data/p0033.lp
Sos: Sos.class
	java $(CLASSDIR): Sos
Sudoku: Sudoku.class
	java $(CLASSDIR): Sudoku $(TWOUP)/data/sudoku1
Tsp: Tsp.class
	java $(CLASSDIR): Tsp 50
Tune: Tune.class
	java $(CLASSDIR): Tune $(TWOUP)/data/p0033.mps
Workforce1: Workforce1.class
	java $(CLASSDIR): Workforce1
Workforce2: Workforce2.class
	java $(CLASSDIR): Workforce2
Workforce3: Workforce3.class
	java $(CLASSDIR): Workforce3
Workforce4: Workforce4.class
	java $(CLASSDIR): Workforce4
Workforce5: Workforce5.class
	java $(CLASSDIR): Workforce5
Multiobj: Multiobj.class
	java $(CLASSDIR): Multiobj
Multiscenario: Multiscenario.class
	java $(CLASSDIR): Multiscenario
Poolsearch: Poolsearch.class
	java $(CLASSDIR): Poolsearch
Genconstr: Genconstr.class
	java $(CLASSDIR): Genconstr
GCPWL: GCPWL.class
	java $(CLASSDIR): GCPWL
GCPWLFunc: GCPWLFunc.class
	java $(CLASSDIR): GCPWLFunc
Batchmode: Batchmode.class
	java $(CLASSDIR): Batchmode $(TWOUP)/data/p0033.mps

run_bilinear_python:
	gurobi.sh $(TWOUP)/python/bilinear.py
run_custom_python:
	gurobi.sh $(TWOUP)/python/custom.py $(TWOUP)/data/p0033.mps
run_callback_python:
	gurobi.sh $(TWOUP)/python/callback.py $(TWOUP)/data/p0033.mps
run_dense_python:
	gurobi.sh $(TWOUP)/python/dense.py
run_diet_python:
	gurobi.sh $(TWOUP)/python/diet.py
run_diet2_python:
	gurobi.sh $(TWOUP)/python/diet2.py
run_diet3_python:
	gurobi.sh $(TWOUP)/python/diet3.py
run_diet4_python:
	gurobi.sh $(TWOUP)/python/diet4.py
run_facility_python:
	gurobi.sh $(TWOUP)/python/facility.py
run_feasopt_python:
	gurobi.sh $(TWOUP)/python/feasopt.py $(TWOUP)/data/klein1.mps
run_fixanddive_python:
	gurobi.sh $(TWOUP)/python/fixanddive.py $(TWOUP)/data/stein9.mps
run_lp_python:
	gurobi.sh $(TWOUP)/python/lp.py $(TWOUP)/data/klein1.mps
run_lpmethod_python:
	gurobi.sh $(TWOUP)/python/lpmethod.py $(TWOUP)/data/afiro.mps
run_lpmod_python:
	gurobi.sh $(TWOUP)/python/lpmod.py $(TWOUP)/data/afiro.mps
run_matrix1_python:
	gurobi.sh $(TWOUP)/python/matrix1.py
run_matrix2_python:
	gurobi.sh $(TWOUP)/python/matrix2.py
run_mip1_python:
	gurobi.sh $(TWOUP)/python/mip1.py
run_mip1_remote_python:
	gurobi.sh $(TWOUP)/python/mip1_remote.py
run_mip2_python:
	gurobi.sh $(TWOUP)/python/mip2.py $(TWOUP)/data/stein9.mps
run_netflow_python:
	gurobi.sh $(TWOUP)/python/netflow.py
run_params_python:
	gurobi.sh $(TWOUP)/python/params.py $(TWOUP)/data/misc07.mps
run_piecewise_python:
	gurobi.sh $(TWOUP)/python/piecewise.py
run_qcp_python:
	gurobi.sh $(TWOUP)/python/qcp.py
run_qp_python:
	gurobi.sh $(TWOUP)/python/qp.py
run_sensitivity_python:
	gurobi.sh $(TWOUP)/python/sensitivity.py $(TWOUP)/data/p0033.lp
run_sos_python:
	gurobi.sh $(TWOUP)/python/sos.py
run_sudoku_python:
	gurobi.sh $(TWOUP)/python/sudoku.py $(TWOUP)/data/sudoku1
run_tsp_python:
	gurobi.sh $(TWOUP)/python/tsp.py 50
run_tune_python:
	gurobi.sh $(TWOUP)/python/tune.py $(TWOUP)/data/p0033.mps
run_workforce1_python:
	gurobi.sh $(TWOUP)/python/workforce1.py
run_workforce2_python:
	gurobi.sh $(TWOUP)/python/workforce2.py
run_workforce3_python:
	gurobi.sh $(TWOUP)/python/workforce3.py
run_workforce4_python:
	gurobi.sh $(TWOUP)/python/workforce4.py
run_workforce5_python:
	gurobi.sh $(TWOUP)/python/workforce5.py
run_genconstr_python:
	gurobi.sh $(TWOUP)/python/genconstr.py
run_poolsearch_python:
	gurobi.sh $(TWOUP)/python/poolsearch.py
run_multiobj_python:
	gurobi.sh $(TWOUP)/python/multiobj.py
run_multiscenario_python:
	gurobi.sh $(TWOUP)/python/multiscenario.py
run_gc_pwl_python:
	gurobi.sh $(TWOUP)/python/gc_pwl.py
run_gc_pwl_func_python:
	gurobi.sh $(TWOUP)/python/gc_pwl_func.py

run_batchmode_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/batchmode_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/p0033.mps; \
	cd -
run_bilinear_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/bilinear_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_callback_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/callback_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/p0033.mps; \
	cd -
run_dense_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/dense_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_diet_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/diet_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_facility_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/facility_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_feasopt_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/feasopt_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/klein1.mps; \
	cd -
run_fixanddive_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/fixanddive_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/stein9.mps; \
	cd -
run_lp_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/lp_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/klein1.mps; \
	cd -
run_lpmethod_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/lpmethod_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/afiro.mps; \
	cd -
run_lpmod_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/lpmod_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/afiro.mps; \
	cd -
run_mip1_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/mip1_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_mip2_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/mip2_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/stein9.mps; \
	cd -
run_params_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/params_cs.cs . ; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/misc07.mps; \
	cd -
run_piecewise_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/piecewise_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_qcp_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/qcp_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_qp_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/qp_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_sensitivity_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/sensitivity_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/p0033.lp; \
	cd -
run_sos_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/sos_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_sudoku_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/sudoku_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/sudoku1; \
	cd -
run_tsp_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/tsp_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) 50; \
	cd -
run_tune_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/tune_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK) $(KEEPTU)/data/p0033.mps; \
	cd -
run_workforce1_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/workforce1_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_workforce2_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/workforce2_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_workforce3_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/workforce3_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_workforce4_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/workforce4_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_workforce5_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/workforce5_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_genconstr_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/genconstr_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_poolsearch_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/poolsearch_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_multiobj_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/multiobj_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_multiscenario_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/multiscenario_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_gc_pwl_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/gc_pwl_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -
run_gc_pwl_func_cs:
	cd $(GRBAPP); \
	rm -f *.cs; \
	cp $(KEEPTU)/c#/gc_pwl_func_cs.cs .; \
	dotnet run $(DOTNETFRAMEWORK); \
	cd -

clean:
	rm -rf *.o *_c *_c++ *.class *.log *.rlp *.lp *.bas *.ilp *.mps *.prm; \
	if [ -d $(GRBAPP) ]; then \
		cd $(GRBAPP); \
		find . ! \( -name dotnetcore2.csproj -o -name . \) -exec rm -rf {} +; \
	fi
