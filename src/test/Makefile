# Helpers for producing multiple comparison graphs with nanobench+doctest
# ======================================================================
# Copyright (c) 2025 CS GROUP
#
# Licensed under the MIT License <http://opensource.org/licenses/MIT>.
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# ======================================================================
#
# Authors:
# - Luc Hermitte, initial author
#
# ======================================================================

# Note: the relevant builtin variables are the following:
#     LINK.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
#     LINK.cpp = $(LINK.cc)
#     COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
# They can be obtained with `make -p`

# Override "%: %.cpp" to ignore extra files like headers...
# -> using $< (1st prereq) instead of $^ (all prereq)
%: %.cpp
	$(LINK.cpp) $< $(LOADLIBES) $(LDLIBS) -o $@



# Check whether clang++ and g++ truly support __cpp_explicit_this_parameter
# If so, we require C++23. Otherwise, we fallback to whatever is the default
ifeq ($(subst clang++,,$(CXX)), $(CXX))
    # not clang
    HAS_EXPLICIT_THIS_PARAMETER = $(shell $(CXX) -dumpfullversion -dumpversion | awk -F. '$$1 >= 14')
else
    # clang!
    HAS_EXPLICIT_THIS_PARAMETER = $(shell $(CXX) -dumpfullversion -dumpversion | awk -F. '$$1 >= 20')
endif
ifeq ($(HAS_EXPLICIT_THIS_PARAMETER),)
	CXX_STD =
else
	CXX_STD = -std=c++23
endif

LEVEL1_DCACHE_SIZE = $(shell getconf -a | awk '/LEVEL1_DCACHE_SIZE/{print $$2;}')
LEVEL2_DCACHE_SIZE = $(shell getconf -a | awk '/LEVEL2_CACHE_SIZE/{print $$2;}')

TARGET_ARCH = -march=native
CPPFLAGS = -DLEVEL1_DCACHE_SIZE=$(LEVEL1_DCACHE_SIZE) -DLEVEL2_DCACHE_SIZE=$(LEVEL2_DCACHE_SIZE)
CXXFLAGS = -O3 $(CXX_STD) -g -DNDEBUG -Wall -Wextra -I../include
LDFLAGS  = -O3

LIB_HEADERS = \
	      ../include/nanobench_html_graph_doctest_main.hpp \
	      ../include/nanobench_html_graph_renderer.hpp \
	      ../include/rng.hpp


example_violin: $(LIB_HEADERS) Makefile
