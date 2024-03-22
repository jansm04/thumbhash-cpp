EXE = th

OBJS_EXE = lodepng.o main.o thumbhash.o

CXX = g++
CXXFLAGS = -std=c++1y -c -g -O0 -Wall -Wextra -pedantic
LD = g++
LDFLAGS = -std=c++1y -lpthread -lm

all : th

$(EXE) : $(OBJS_EXE)
	$(LD) $(OBJS_EXE) $(LDFLAGS) -o $(EXE)

#object files
lodepng.o : util/lodepng/lodepng.cpp util/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) util/lodepng/lodepng.cpp -o lodepng.o

thumbhash.o : src/thumbhash.cpp src/thumbhash.h
	$(CXX) $(CXXFLAGS) src/thumbhash.cpp -o thumbhash.o

main.o : examples/main.cpp util/lodepng/lodepng.h src/thumbhash.h
	$(CXX) $(CXXFLAGS) examples/main.cpp -o main.o

clean :
	-rm -f *.o $(EXE) examples/images-output/*.png