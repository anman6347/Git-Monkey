CXX = g++
CXXFLAGS = 
#TARGET_ADD = add
SRCS = MiniGit.cpp Add.cpp Init.cpp
#OBJS := $(OBJS:%.cpp=%.o)
OBJS = $(SRCS:%.cpp=%.o)
TARGET := $(SRCS:%.cpp=%.exe)

#comma:= ,
#empty:=
#space:= $(empty) $(empty)
#WINTARGET := $(subst $(space),$(comma),$(TARGET))


.PHONY: all clean run

all: $(TARGET)

%.exe: %.o
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

run: $(TARGET)
	cd ..\testspace && MiniGit add -A && cd ..\Xass
# del /Q .\*
#	del /Q ..\testspace\* 
#	copy *.exe ..\testspace
#	cd ..\testspace
#	.\MiniGit.exe init
#	cd ..\Xass

clean:
	del $(TARGET)
