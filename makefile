CXX = g++
CXXFLAGS = 
INCDIR = "C:\\Program Files\\OpenSSL-Win64\\include"
LIBDIR = "C:\\Program Files\\OpenSSL-Win64\\lib"
#LIBS = -lssl -lcrypto
LIBS = "C:\\Program Files\\OpenSSL-Win64\\lib\\VC\\x64\\MT\\libcrypto.lib" 
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
	$(CXX) $(CXXFLAGS) -c $^ -o $@

MiniGit.exe: MiniGit.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Init.exe: Init.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Add.exe: Add.o Crypt.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbcrypt -lz

run: $(TARGET)
	cd ..\testspace && MiniGit add -A && cd ..\Xass
# del /Q .\*
#	del /Q ..\testspace\* 
#	copy *.exe ..\testspace
#	cd ..\testspace
#	.\MiniGit.exe init
#	cd ..\Xass

clean:
	del $(TARGET) $(OBJS)
