CXX = g++
CXXFLAGS = -std=c++1y -fPIC
SOFLAGS = -shared -O3
INCLUDE = -I"./" 
LIB = -L"./lib/"
PYINCLUDE := `python3 -m pybind11 --includes`
PYEXT := `python3-config --extension-suffix`
PYBINDLIB = lib/cfgmanager$(shell python3-config --extension-suffix)
PYSITE := `python -c 'import site; print(site.getsitepackages()[0])'`

libdir = /usr/lib/root/

ROOT_LIB := `root-config --libs --glibs`
ROOT_FLAGS := `root-config --cflags --ldflags`

DEPS = 	interface/CfgManager.h interface/CfgManagerT.h Makefile

DEPS_OBJS = lib/CfgManager.o

all: $(DEPS_OBJS) lib/libCFGMan.so ${PYBINDLIB}

lib/%.o: src/%.cc $(DEPS)
	@echo " CXX $<"	
	@$ $(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDE) $(ROOT_FLAGS)

# lib/libCFGMan.cxx: interface/CfgManager.h interface/CfgManagerT.h interface/LinkDef.h 
# 	@$ rootcling -f $@ -rml libCFGMan.so -rmf lib/libCFGMan.rootmap $^
# 	@$ rootcling -f $@ -rml libCFGMan.so -rmf lib/libCFGMan.rootmap $^


${PYBINDLIB}: src/python_bindings.cc src/CfgManager.cc interface/CfgManager.h
	@echo " Python bindings"
	@$ $(CXX) $(CXXFLAGS) $(SOFLAGS) -o $@ $^ $(INCLUDE) $(PYINCLUDE)

lib/libCFGMan.so: lib/CfgManager.o 
	@echo " CXX $<"
	@$ $(CXX) $(CXXFLAGS) $(SOFLAGS) -shared -o $@ $^ $(INCLUDE) $(LIB)

bin/%: test/%.cpp lib/libCFGMan.so 
	@echo " CXX $<"
	@$ $(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS) $(LIB)

install:
	cp lib/libCFGMan.so $(libdir)
	cp lib/libCFGMan_rdict.pcm $(libdir)
	cp lib/libCFGMan.rootmap $(libdir)

uninstall:
	rm $(libdir)/libCFGMan.so
	rm $(libdir)/libCFGMan_rdict.pcm
	rm $(libdir)/libCFGMan.rootmap

clean:
	rm -fr tmp/*
	rm -fr lib/*
	rm -fr bin/*
