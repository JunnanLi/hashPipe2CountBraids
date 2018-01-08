Object = analysisPcap.o common.o hash.o readPcap.o taskCounterBraids.o taskBloomFilter.o taskHashPipe.o testHashPipe.o
EXE = test

test: $(Object)
	gcc -o $(EXE) $(Object)

analysisPcap.o: analysisPcap.c analysisPcap.h common.h taskHashPipe.h
	gcc -c analysisPcap.c

hash.o: hash.c hash.h common.h
	gcc -c hash.c

common.o: common.c common.h
	gcc -c common.c

readPcap.o: readPcap.c readPcap.h common.h
	gcc -c readPcap.c

taskCounterBraids.o: taskCounterBraids.c taskCounterBraids.h common.h hash.h
	gcc -c taskCounterBraids.c

taskBloomFilter.o: taskBloomFilter.c taskBloomFilter.h common.h hash.h
	gcc -c taskBloomFilter.c

taskHashPipe.o: taskHashPipe.c taskHashPipe.h common.h hash.h
	gcc -c taskHashPipe.c

testHashPipe.o: testHashPipe.c common.h hash.h analysisPcap.h taskCounterBraids.h taskHashPipe.h readPcap.h taskBloomFilter.h
	gcc -c testHashPipe.c

clean:
	rm $(EXE) $(Object)
