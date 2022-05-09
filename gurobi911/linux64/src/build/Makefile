C++      = g++
C++FLAGS = -m64 -fPIC -O

C++OBJS = Env.o Model.o attrprivate.o Var.o Constr.o LinExpr.o QuadExpr.o \
          Exception.o Callback.o Column.o SOS.o QConstr.o GenConstr.o \
          TempConstr.o Batch.o

c++: libgurobi_c++.a

%.o: ../cpp/%.cpp ../cpp/%.h
	$(C++) $(C++FLAGS) -I../../include -c $<

libgurobi_c++.a: $(C++OBJS)
	ar rv libgurobi_c++.a $(C++OBJS)

clean:
	rm -f *.o libgurobi_c++.a
