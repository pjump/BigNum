#Author: Petr Skocik
#skocipet@fit.cvut.cz

#allowed commands: rm, gcc, g++, mkdir, doxygen, cp, mv, cd, ar, make
#OPTIONS
EXE=bignum
CXXFLAGS= -Wall -pedantic -Wno-long-long -O0 -ggdb -std=c++0x -DSTYPE=u64 
#-DUSE_READLINE
#Add the above option along with adding -lreadline to CLIBS if you want readline support when running the program interactively
DOXY=Doxyfile
CLIBS=
#-lreadline
CXX=g++
RM=rm -rf

#DIRECTORIES:
#Sources
SRCDIR=src
DOCDIR=doc
BINDIR=bin
#NOTE: $() references established vars
#Objects
OBJDIR=$(BINDIR)/objs
#Dependencies
DEPDIR=$(SRCDIR)/.deps

#An object and a dependency file will be created out of each and every *.cpp in $(SRCDIR)
SRCS=$(wildcard $(SRCDIR)/*.cpp)
#Two ways to change suffixes below:
#% in substitutions = * (the wildcard)
#OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
#DEPS = $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$(OBJS))
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)


#----------------------------------------------------------------------------------
#PHONY TARGETS (make won't get confused if the corresponding files exist)
.PHONY: all compile run clean deps

all: compile doc
compile: $(EXE)
run: $(EXE)
	./$(EXE) test 
clean:
	$(RM) $(BINDIR) $(DOCDIR) $(EXE) $(DEPDIR)
deps: $(DEPS)
#@ suppresses echoing of the line that's prefixed with it
doc: $(DOXY) $(SRCDIR)/* 
	@mkdir -p doc
	@doxygen $(DOXY) \&>/dev/null

#REAL TARGETS ----------------------------------------------------------------------------------

#Automatic Variables:
#$@ = The file of the target of the rule
#$* = stem of the target in an implicit rule (target - suffix)
#$< = file name of the first prerequisite
$(OBJDIR) $(DEPDIR):
	mkdir -p $@

#generate dependencies into src/.deps (must exist)
$(DEPDIR)/%.d: $(SRCDIR)/%.cpp | $(DEPDIR)
	$(CXX) $(CXXFLAGS) -MM -MF $@ -MT $(OBJDIR)/$*.o $<

# pull in dependency info for *existing* .o files
-include $(DEPS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $(SRCDIR)/$*.cpp -o $@

$(EXE): $(OBJS) 
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(CLIBS)
