LIB = libDDD.a

SRCS = DED.cpp DDD.cpp Hom.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

CXX = g++
#CXXFLAGS= -O9  -Wall #-pg
CXXFLAGS= -g  -Wall -D DEBUG_EGAL #-pg 

all: $(LIB)

$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	ranlib $(LIB)

# implicit rules

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $<

clean:
	rm -f $(LIB) *.o *.d *~

%.d: %.cpp
	@$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(INCLUDES) $< \
                | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
                [ -s $@ ] || rm -f $@'

-include $(DEPS)        












