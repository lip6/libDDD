SRC = tst1.cpp tst2.cpp tst3.cpp tst3.cpp tst4.cpp tst5.cpp tst6.cpp tst7.cpp tst8.cpp tst9.cpp
RES = $(SRC:.cpp=.res)
EXE = $(SRC:.cpp=.exe)
DIR = ../src

all: $(EXE)

%.exe: %.cpp
	g++ -g $< -I$(DIR) -L$(DIR) -lDDD -o $@

clean:
	rm -f $(LIB) *.exe *.res *~


