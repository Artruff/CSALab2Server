# Makefile to build the project

CC = g++
CFLAGS = -Wall -I hv
LDFLAGS = -L lib -lCatch2
LIBS = -lhv -lsqlite3
OUTDIR = build/
TESTDIR = test/
PACKAGEDIR = package/usr/bin/

all: clean CSALab2 Test

CSALab2: CSALab2.o MyServer.o
	mkdir -p $(OUTDIR)
	mkdir -p $(PACKAGEDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUTDIR)CSALab2 CSALab2.o MyServer.o $(LIBS)
	cp $(OUTDIR)CSALab2 $(PACKAGEDIR)CSALab2

CSALab2.o: src/CSALab2.cpp
	$(CC) $(CFLAGS) -c src/CSALab2.cpp

MyServer.o: src/MyServer.cpp
	$(CC) $(CFLAGS) -c src/MyServer.cpp

Test: Test.o MyServer.o
	mkdir -p $(TESTDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)Test Test.o MyServer.o $(LIBS)

Test.o: src/Test.cpp
	$(CC) $(CFLAGS) -c src/Test.cpp

clean:
	rm -f *.o *.deb $(OUTDIR)CSALab2 $(PACKAGEDIR)CSALab2 $(TESTDIR)Test
	rm -rf $(OUTDIR) $(TESTDIR)