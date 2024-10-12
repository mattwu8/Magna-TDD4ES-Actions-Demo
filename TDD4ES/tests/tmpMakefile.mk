# If you installed CppUTest from apt, leave CPPUTEST_HOME as is, otherwise, change it to the path where you installed CppUTest
CPPUTEST_HOME = ../../cpputest
CPPFLAGS += -I$(CPPUTEST_HOME)/include/

# CXXFLAGS += -include $(CPPUTEST_HOME)/include/CppUTest/MemoryLeakDetectorNewMacros.h
# CFLAGS += -include $(CPPUTEST_HOME)/include/CppUTest/MemoryLeakDetectorMallocMacros.h

LD_LIBRARIES = -L$(CPPUTEST_HOME)/cpputest_build/lib -lCppUTest -lCppUTestExt

# Flags
CPPFLAGS += -I../test_mocks	# Add flag to include mocking headers
CPPFLAGS += -I..			# Add flag to include parent directory
# CPPFLAGS += -I../../../
CPPUFLAGS += -DUNIT_TEST	# Test flag (to trigger certain preprocessor directives)
CPPFLAGS += --coverage -O0	# Add flags for code coverage

# Compiler. Use g++ which can compile both C and C++ files
CXX = g++

# Main test file
SRCS = all_tests.cpp

# Add tests
SRCS += matthew_test.cpp

# Output executable
TARGET = ez.testcase

# Build target
$(TARGET): $(SRCS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LD_LIBRARIES)

# Clean target
clean:
	rm -f $(TARGET)
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -r html/
	rm -r xml/