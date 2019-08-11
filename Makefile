DEBUG_FLAG=-g
OPTIMIZATION_FLAG=$(if $(OPT),$(OPT),-O0)
WARNING_FLAG = -Wall

formulartests: formulartests.cpp ogame.hpp ogame.o
	g++ -std=c++14 $(DEBUG_FLAG) $(WARNING_FLAG) formulartests.cpp ogame.o -o formulartests
	
dplistcalculator: dplistcalculator.cpp ogame.hpp ogame.o util.hpp
	g++ -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) -fopenmp dplistcalculator.cpp ogame.o -o dplistcalculator

accountupgrade: main_accountupgrade.o ogame.o account.o serialization.o
	g++ -std=c++14 main_accountupgrade.o ogame.o account.o serialization.o -fopenmp -o accountupgrade

main_accountupgrade.o: main_accountupgrade.cpp ogame.hpp account.hpp parallel_permutation.hpp util.hpp serialization.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) main_accountupgrade.cpp -fopenmp -o main_accountupgrade.o
	
ogame.o: ogame.cpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) ogame.cpp -o ogame.o
	
account.o: account.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) account.cpp -o account.o

serialization.o: serialization.hpp serialization.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) serialization.cpp -o serialization.o
	
clean:
	rm -rf dplistcalculator accountupgrade main_accountupgrade.o ogame.o account.o serialization.o formulartest
	
