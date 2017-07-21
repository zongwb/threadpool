#Copyright (C) 2015 Zong Wenbo

#TARGET = tp-test
#CC  = gcc
#AR = ar
#CPPC = clang++-3.5 -std=c++14
CPPC = g++ -std=c++14
CFLAGS = -g -D_GNU_SOURCE -Wall -Iinclude -I..  -D NUMCORES=10
LIBS= -lpthread 
ALL_LIBS=$(LIBS) 
EXT=cc

THREADLIB = -L.. -lthreadpool

.PHONY: default all clean
#.PRECIOUS: $(TARGET) $(OBJECTS) 

thread_test: thread_test.cc
	$(CPPC) $(CFLAGS) thread_test.cc -o $@ $(THREADLIB) $(ALL_LIBS) 

channel_test: channel_test.cc
	$(CPPC) $(CFLAGS) channel_test.cc -o $@ $(THREADLIB) $(ALL_LIBS) 

threadpool_test: threadpool_test.cc
	$(CPPC) $(CFLAGS) threadpool_test.cc -o $@ $(THREADLIB) $(ALL_LIBS) 

tb_test: tokenbucket_test.cc
	$(CPPC) $(CFLAGS) tokenbucket_test.cc -o $@ $(THREADLIB) $(ALL_LIBS) 

clean:
	-rm -f obj/*.o
	-rm -f *.o
	-rm -f thread_test threadpool_test tb_test
