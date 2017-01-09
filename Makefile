CXX = g++
CXXFLAGS = -std=c++1y -fPIC
SOFLAGS = -shared -O3
INCLUDE = -I"./" 
LIB = -L"./lib/"
libdir = /usr/lib/root/

ROOT_LIB := `root-config --libs --glibs`
ROOT_FLAGS := `root-config --cflags --ldflags`

DEPS = 	interface/CfgManager.h interface/CfgManagerT.h Makefile

DEPS_OBJS = lib/CfgManager.o

all: $(DEPS_OBJS) lib/libCFGMan.cxx lib/libCFGMan.so bin/test

lib/%.o: src/%.cc $(DEPS)
	@echo " CXX $<"	
	@$ $(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDE) $(ROOT_FLAGS)

lib/libCFGMan.cxx: interface/CfgManager.h interface/CfgManagerT.h interface/LinkDef.h 
	@$ rootcling -f $@ -rml libCFGMan.so -rmf lib/libCFGMan.rootmap $^
	@$ rootcling -f $@ -rml libCFGMan.so -rmf lib/libCFGMan.rootmap $^

lib/libCFGMan.so: lib/libCFGMan.cxx lib/CfgManager.o $(DEPS_OBJS)
	@echo " CXX $<"
	@$ $(CXX) $(CXXFLAGS) $(SOFLAGS) -shared -o $@ $^ $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS) $(LIB)

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
