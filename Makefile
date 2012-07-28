CFLAGS+=-std=c99
LIBS=-lcrypto
COVERAGE_FLAGS=--coverage

all:
	$(CC) $(LIBS) $(CFLAGS) -c src/b64.c -DDEBUG=0 -DTEST=0
	$(CC) $(LIBS) $(CFLAGS) -c src/websocket.c -DDEBUG=0 -DTEST=0
	ar rcv libwebsocket.a b64.o websocket.o

test:
	$(CC) -g -I src/ src/b64.c src/websocket.c \
		  tests/tests.c \
		  $(CFLAGS) $(LIBS) -lcunit -o test_cws -DDEBUG=0 -DTEST=1
	      ./test_cws

coverage:
	$(CC) -g -I src/ src/b64.c src/websocket.c \
		  tests/tests.c \
		  -lcunit $(CFLAGS) $(COVERAGE_FLAGS) $(LIBS) -o coverage_cws -DDEBUG=0 -DTEST=1
	      ./coverage_cws
		  lcov --directory . --capture --output-file app.info
		  genhtml --output-directory coverage/ app.info

clean:
	rm -f *.o
	rm -f *.a
	rm -rf coverage/
	rm -f  *.gcno *.gcda
	rm -f app.info
	rm -f  test_cws
	rm -f  coverage_cws
