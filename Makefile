CFLAGS=-lcrypto -std=c99

COVERAGE_FLAGS=--coverage

all:
	$(CC) src/b64.c src/websocket.c $(CFLAGS) -shared -o cws.so -DDEBUG=0 -DTEST=0

test:
	$(CC) -g src/b64.c src/websocket.c \
		  src/tests.c \
		  $(CFLAGS) -lcunit -o test_cws -DDEBUG=0 -DTEST=1
	      ./test_cws

coverage:
	$(CC) -g src/b64.c src/websocket.c \
		  src/tests.c \
		  -lcunit $(CFLAGS) $(COVERAGE_FLAGS) -o coverage_cws -DDEBUG=0 -DTEST=1
	      ./coverage_cws
		  lcov --directory . --capture --output-file app.info
		  genhtml --output-directory coverage/ app.info

clean:
	rm -rf coverage/
	rm -f  *.gcno *.gcda
	rm -f app.info
	rm -f  test_cws
	rm -f  coverage_cws
