all: MCMC_Example
MADAI_PREFIX = ${HOME}/local

WARNING_FLAGS = -Wall -Wextra -pedantic -Wno-long-long -Wno-deprecated
CXXFLAGS = -O3 ${WARNING_FLAGS} -I"${MADAI_PREFIX}/include"
LDFLAGS = -L"${MADAI_PREFIX}/lib/madai" -lDistributionSampling -lmadaisys
%.o : %.cxx
	$(CXX) -c $(CXXFLAGS) $< -o $@
% : %.o
	$(CXX) $< $(LDFLAGS) -o $@
.PHONY : all
