CFLAGS+=-std=c99
LIBS=-lcrypto
COVERAGE_FLAGS=--coverage

all: clean
	$(CC) $(LIBS) $(CFLAGS) -c src/b64.c -DDEBUG=0 -DTEST=0
	$(CC) $(LIBS) $(CFLAGS) -c src/cws.c -DDEBUG=0 -DTEST=0
	ar rcv libcws.a b64.o cws.o

test: clean
	$(CC) -g -I src/ src/b64.c src/cws.c \
		  tests/test_cws.c \
		  $(CFLAGS) $(LIBS) -lcunit -o test_cws -DDEBUG=0 -DTEST=1
	      ./test_cws

coverage: clean
	$(CC) -g -I src/ src/b64.c src/cws.c \
		  tests/test_cws.c \
		  -lcunit $(CFLAGS) $(COVERAGE_FLAGS) $(LIBS) -o coverage_cws -DDEBUG=0 -DTEST=1
	      ./coverage_cws
		  lcov --directory . --capture --output-file app.info
		  genhtml --output-directory coverage/ app.info

clean:
	@rm -f *.o
	@rm -f *.a
	@rm -rf coverage/
	@rm -f  *.gcno *.gcda
	@rm -f app.info
	@rm -rf test_cws.*/
	@rm -rf coverage_cws.*/
