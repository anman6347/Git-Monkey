CXX = g++
CXXFLAGS = 
INCDIR1 = .\\zlib-1.3.1
INCDIR2 = .\\curl-8.8.0_3-win64-mingw\\include
INCDIR3 = .\\jsoncpp-1.9.0\\include
INCDIR4 = .\\jsoncpp-1.9.0\\build\\include\\json
INCDIR5 = .\\jsoncpp-1.9.0\\build\\src\\lib_json
LIBDIR1 = .\\zlib-1.3.1 
LIBDIR2 = .\\curl-8.8.0_3-win64-mingw\\lib 
LIBDIR3 = .\\jsoncpp-1.9.0\\build\\src\\lib_json
SRCS = MiniGit.cpp Add.cpp Init.cpp Commit.cpp Wrapper.cpp Upload.cpp
TARGET := $(SRCS:%.cpp=%.exe)
SRCS +=  Crypt.cpp Index.cpp FileTree.cpp CommitObj.cpp Compress.cpp
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
	$(CXX) $(CXXFLAGS) -I $(INCDIR1) -I $(INCDIR2) -I $(INCDIR3) -I $(INCDIR4) -I $(INCDIR5) -L $(LIBDIR1) -L $(LIBDIR2) -L $(LIBDIR3) -c $^ -o $@ -lcurl -lssl -lcrypto -lws2_32 -lbcrypt -ljsoncpp

Wrapper.exe : Wrapper.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Upload.exe : Upload.o
	$(CXX) $(CXXFLAGS) -I $(INCDIR2) -I $(INCDIR3) -I $(INCDIR4) -I $(INCDIR5) -L $(LIBDIR1) -L $(LIBDIR2) -L $(LIBDIR3) $^ -o $@ -lcurl -lssl -lcrypto -lws2_32 -lbcrypt -ljsoncpp

MiniGit.exe: MiniGit.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Init.exe: Init.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# -lws2_32 is required windows 10 ?
Add.exe: Add.o Crypt.o Compress.o
	$(CXX) $(CXXFLAGS) -L $(LIBDIR1) $^ -o $@ -lbcrypt -lz -lws2_32

Commit.exe: Commit.o Crypt.o Index.o FileTree.o CommitObj.o Compress.o
	$(CXX) $(CXXFLAGS) -L $(LIBDIR1) $^ -o $@ -lbcrypt -lz -lws2_32

#run: $(TARGET)
#	cd ..\test_tmp_dir2 && ..\Xass\MiniGit.exe init && ..\Xass\MiniGit.exe add -A && ..\Xass\MiniGit.exe commit -m "test commit 1" && echo Hi! >> aabbcc.txt && ..\Xass\MiniGit.exe add -A && ..\Xass\MiniGit.exe commit -m "test commit 2" && cd ..\Xass

clean:
	del $(TARGET) $(OBJS)
#	rmdir /s /q ..\test_tmp_dir2\.git
