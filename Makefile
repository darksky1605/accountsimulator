DEBUG_FLAG=-g
OPTIMIZATION_FLAG=$(if $(OPT),$(OPT),-O3)
WARNING_FLAG = -Wall -Wextra -Wpedantic

CFLAGS = $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) -fopenmp

formulartests: dirs formulartests.cpp ogame.hpp build/ogame.o
	g++ -std=c++14 $(DEBUG_FLAG) $(WARNING_FLAG) formulartests.cpp build/ogame.o -o formulartests
	
dplistcalculator: dplistcalculator.cpp ogame.hpp build/ogame.o util.hpp
	g++ -std=c++14 $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) $(WARNING_FLAG) -fopenmp dplistcalculator.cpp build/ogame.o -o dplistcalculator

accountupgrade: dirs build/main_accountupgrade.o build/ogame.o build/account.o build/serialization.o build/io.o
	g++ -std=c++14 build/main_accountupgrade.o build/ogame.o build/account.o build/serialization.o build/io.o -fopenmp -o accountupgrade

build/main_accountupgrade.o: main_accountupgrade.cpp ogame.hpp account.hpp parallel_permutation.hpp util.hpp serialization.hpp io.hpp
	g++ -c -std=c++14 $(CFLAGS) main_accountupgrade.cpp -fopenmp -o build/main_accountupgrade.o
	
build/ogame.o: ogame.cpp ogame.hpp
	g++ -c -std=c++14 $(CFLAGS) ogame.cpp -o build/ogame.o
	
build/account.o: account.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(CFLAGS) account.cpp -o build/account.o

build/serialization.o: serialization.hpp serialization.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(CFLAGS) serialization.cpp -o build/serialization.o

build/io.o: io.hpp io.cpp account.hpp ogame.hpp
	g++ -c -std=c++14 $(CFLAGS) io.cpp -o build/io.o

clean:
	rm -rf dplistcalculator accountupgrade build/ formulartest
	
dirs:
	mkdir -p build
