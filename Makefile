# Make shure to define rule for each target
SHELL := bash

CXX := g++
CXXFLAGS := -Wall -fno-elide-constructors
include txx.mk

# Determine the object files
OBJ :=

# Use submodules instead of recursion
MODULES := init dfg
# Include the description for each module
include $(patsubst %, %/module.mk, $(MODULES))

int: $(OBJ)
	echo $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGETS)
	rm -rf $(addsuffix /*.o, $(MODULES))
	rm -rf $(addsuffix /*.d, $(MODULES))

.PHONY: clean $(TARGETS)

# Dependencies
include $(OBJ:.o=.d)

%.d: %.cc
	./depend.sh `dirname $*` $(CXXFLAGS) $< > $@

