DEBUG_FLAG=-g
OPTIMIZATION_FLAG=$(if $(OPT),$(OPT),-O0)
WARNING_FLAG = -Wall -Wextra -Wpedantic

formulartests: formulartests.cpp ogame.hpp build/ogame.o
	g++ -std=c++14 $(DEBUG_FLAG) $(WARNING_FLAG) formulartests.cpp build/ogame.o -o formulartests
	
dplistcalculator: dplistcalculator.cpp ogame.hpp build/ogame.o util.hpp
	g++ -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) -fopenmp dplistcalculator.cpp build/ogame.o -o dplistcalculator

accountupgrade: build/main_accountupgrade.o build/ogame.o build/account.o build/serialization.o
	g++ -std=c++14 build/main_accountupgrade.o build/ogame.o build/account.o build/serialization.o -fopenmp -o accountupgrade

build/main_accountupgrade.o: dirs main_accountupgrade.cpp ogame.hpp account.hpp parallel_permutation.hpp util.hpp serialization.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) main_accountupgrade.cpp -fopenmp -o build/main_accountupgrade.o
	
build/ogame.o: dirs ogame.cpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) ogame.cpp -o build/ogame.o
	
build/account.o: dirs account.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) account.cpp -o build/account.o

build/serialization.o: dirs serialization.hpp serialization.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) serialization.cpp -o build/serialization.o
	
clean:
	rm -rf dplistcalculator accountupgrade build/ formulartest
	
dirs:
	mkdir -p build
