CC=g++ -g -Wall -std=c++17

# List of source files for your thread library
# THREAD_SOURCES=file1.cpp file2.cpp
THREAD_SOURCES=cpu.cpp thread.cpp mutex.cpp cv.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

all: libcpu.o libthread.o app app2 app3 app4 app5 app6 app7 app8 app9 app10 app11 app12

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	./autotag.sh
	ld -r -o $@ ${THREAD_OBJS}

# Compile an application program
app: test1.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app2: test2.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app3: test3.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app4: test4.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread	

app5: test5.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app6: test6.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app7: test7.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app8: test8.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app9: test9.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app10: test10.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app11: test11.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

app12: test12.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${THREAD_OBJS} libthread.o app* core*
