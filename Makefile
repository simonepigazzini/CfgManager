CXX = g++
CXXFLAGS = -std=c++1y -fPIC
SOFLAGS = -shared -O3
INCLUDE = -I"./" 
LIB = -L"./lib/"

ROOT_LIB := `root-config --libs --glibs`
ROOT_FLAGS := `root-config --cflags --ldflags`

DEPS = 	interface/CfgManager.h interface/CfgManagerT.h

DEPS_OBJS = lib/CfgManager.o

all: $(DEPS_OBJS) lib/LinkDef.cxx lib/libCFGMan.so lib/CfgManagerDict.so bin/test

lib/%.o: src/%.cc $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS)

lib/libCFGMan.so: lib/CfgManager.o
	$(CXX) -shared -o $@ $< 

lib/LinkDef.cxx: interface/CfgManager.h interface/CfgManagerT.h interface/LinkDef.h 
	rootcling -f $@ -c $^

lib/CfgManagerDict.so: lib/LinkDef.cxx $(DEPS_OBJS)
	$(CXX) $(CXXFLAGS) $(SOFLAGS) -o $@ $^ $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS) $(LIB)

bin/%: test/%.cpp lib/libCFGMan.so lib/CfgManagerDict.so
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS) $(LIB)

clean:
	rm -fr tmp/*
	rm -fr lib/*
	rm -fr bin/*
