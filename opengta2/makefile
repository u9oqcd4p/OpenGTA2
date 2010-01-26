
CXXFLAGS :=
CXXFLAGS += -g -D_DEBUG
LIBS := -lGL -lXrandr -lrt
AFTERLIBS := -lglfw

SRC := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SRC))

opengta2: $(OBJS)
	g++ -o $@ $(LIBS) $(OBJS) $(AFTERLIBS)

%.o: %.cpp
	g++ -c -MMD $(CXXFLAGS) $< -o $@

-include *.d

clean:
	rm -rf *.o *.d
