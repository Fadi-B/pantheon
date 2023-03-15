AM_CPPFLAGS = -I../protobufs -I../network -I../crypto -I../extern_utils/eigen-3.3.9
AM_CXXFLAGS = -std=c++14 -w $(WARNING_CXXFLAGS) $(PICKY_CXXFLAGS) $(HARDEN_CFLAGS) $(MISC_CXXFLAGS)

noinst_LIBRARIES = libsprout.a

libsprout_a_SOURCES = process.cc  processforecaster.cc  receiver.cc  sampledfunction.cc kalman_filter.cc
