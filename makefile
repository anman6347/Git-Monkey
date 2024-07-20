CXX = g++
CXXFLAGS = 
INCDIR = ".\\zlib-1.3.1"
LIBDIR = ".\\zlib-1.3.1"
SRCS = MiniGit.cpp Add.cpp Init.cpp Commit.cpp
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

# -lws2_32 is required windows 10 ?
Add.exe: Add.o Crypt.o
	$(CXX) $(CXXFLAGS) -L $(LIBDIR) $^ -o $@ -lbcrypt -lz -lws2_32

Commit.exe: Commit.o Crypt.o
	$(CXX) $(CXXFLAGS) -L $(LIBDIR) $^ -o $@ -lbcrypt -lz -lws2_32

run: $(TARGET)
	cd ..\test_tmp_dir1 && ..\Xass\MiniGit.exe init && ..\Xass\MiniGit.exe add -A && ..\Xass\MiniGit.exe commit -m "test commit 1" && cd ..\Xass

clean:
	del $(TARGET) $(OBJS)
	rmdir /s /q ..\testspace\.git
