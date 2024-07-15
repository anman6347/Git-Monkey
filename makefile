CXX = g++
CXXFLAGS = 
INCDIR = ".\\zlib-1.3.1"
LIBDIR = ".\\zlib-1.3.1"
SRCS = MiniGit.cpp Add.cpp Init.cpp
TARGET := $(SRCS:%.cpp=%.exe)
SRCS +=  Crypt.cpp
OBJS = $(SRCS:%.cpp=%.o)


#comma:= ,
#empty:=
#space:= $(empty) $(empty)
#WINTARGET := $(subst $(space),$(comma),$(TARGET))


.PHONY: all clean run

all: $(TARGET)

#%.exe: %.o
#	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I $(INCDIR) -c $^ -o $@

MiniGit.exe: MiniGit.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Init.exe: Init.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Add.exe: Add.o Crypt.o
	$(CXX) $(CXXFLAGS) -L $(LIBDIR) $^ -o $@ -lbcrypt -lz

run: $(TARGET)
	cd ..\testspace && ..\Xass\MiniGit.exe init && ..\Xass\MiniGit.exe add -A && cd ..\Xass

# del /Q .\*
#	del /Q ..\testspace\* 
#	copy *.exe ..\testspace
#	cd ..\testspace
#	.\MiniGit.exe init
#	cd ..\Xass

clean:
	del $(TARGET) $(OBJS)
	rmdir /s /q ..\testspace\.git
