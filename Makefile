DEBUG_FLAG=-g
OPTIMIZATION_FLAG=-O0
WARNING_FLAG = -Wall

formulartests: formulartests.cpp ogame.hpp ogame.o
	g++ -std=c++14 $(DEBUG_FLAG) $(WARNING_FLAG) formulartests.cpp ogame.o -o formulartests
	
dplistcalculator: dplistcalculator.cpp ogame.hpp ogame.o util.hpp
	g++ -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) -fopenmp dplistcalculator.cpp ogame.o -o dplistcalculator

accountupgrade: accountupgrade.cpp ogame.hpp ogame.o account.hpp account.o parallel_permutation.hpp util.hpp
	g++ -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) accountupgrade.cpp ogame.o account.o -fopenmp -o accountupgrade
	
ogame.o: ogame.cpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) ogame.cpp -o ogame.o
	
account.o: account.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) account.cpp -o account.o
	
clean:
	rm -rf dplistcalculator accountupgrade ogame.o account.o formulartest
	
