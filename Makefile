

cpp_files = src/Potentials.cpp src/FastMultipole.cpp



.phony: all clean
all: lib/libFastMultipole_dbg.so lib/libFastMultipole.so fmtest.o fmtest_dbg.o
clean:
	rm lib/* *.o


lib/libFastMultipole.so: $(cpp_files) src/*.hpp
	g++ $(cpp_files) -shared -o lib/libFastMultipole.so -Isrc -Ofast -std=c++20 -fPIC -pthread

lib/libFastMultipole_dbg.so: $(cpp_files) src/*.hpp
	g++ $(cpp_files) -shared -o lib/libFastMultipole_dbg.so -Isrc -std=c++20 -fPIC -ggdb -D DEBUG -D TIMINGS -pthread

fmtest.o: $(cpp_files) src/*.hpp src/FMTest.cpp
	g++ $(cpp_files) src/FMTest.cpp -o fmtest.o -Isrc -std=c++20 -Ofast -fPIC -pthread
fmtest_dbg.o: $(cpp_files) src/*.hpp src/FMTest.cpp
	g++ $(cpp_files) src/FMTest.cpp -o fmtest_dbg.o -Isrc -std=c++20 -fPIC -ggdb -D DEBUG -D TIMINGS -pthread
