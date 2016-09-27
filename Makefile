CXX = g++
CXXFLAGS = -std=c++1y -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
SOFLAGS = -shared -O3
INCLUDE = -I"./" 
LIB = -L"./lib/"
libdir = /usr/lib/root/

ROOT_LIB := `root-config --libs --glibs`
ROOT_FLAGS := `root-config --cflags --ldflags`

DEPS = 	interface/CfgManager.h interface/CfgManagerT.h

DEPS_OBJS = lib/CfgManager.o

all: $(DEPS_OBJS) lib/LinkDef.cxx lib/libCFGMan.so lib/CfgManagerDict.so bin/test

lib/%.o: src/%.cc $(DEPS)
	@echo " CXX $<"	
	@$ $(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS)

lib/libCFGMan.so: lib/CfgManager.o
	@echo " CXX $<"
	@$ $(CXX) -shared -o $@ $< 

lib/LinkDef.cxx: interface/CfgManager.h interface/CfgManagerT.h interface/LinkDef.h 
	@$ rootcling -f $@ -c $^
	@$ rootcling -f $@ -c $^

lib/CfgManagerDict.so: lib/LinkDef.cxx $(DEPS_OBJS)
	@echo " CXX $<"
	@$ $(CXX) $(CXXFLAGS) $(SOFLAGS) -o $@ $^ $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS) $(LIB)

bin/%: test/%.cpp lib/libCFGMan.so lib/CfgManagerDict.so
	@echo " CXX $<"
	@$ $(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDE) $(ROOT_LIB) $(ROOT_FLAGS) $(LIB)

install:
	cp lib/CfgManagerDict.so $(libdir)
	cp lib/libCFGMan.so $(libdir)
	cp lib/LinkDef_rdict.pcm $(libdir)

uninstall:
	rm $(libdir)/CfgManagerDict.so
	rm $(libdir)/libCFGMan.so
	rm $(libdir)/LinkDef_rdict.pcm

clean:
	rm -fr tmp/*
	rm -fr lib/*
	rm -fr bin/*
