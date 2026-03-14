CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c99 -I../src -Wno-unused-function

TEST_TARGETS = test_auth test_service_request test_tax_record

.PHONY: all clean

all: $(TEST_TARGETS)
	@echo ""
	@echo "Running all tests..."
	@echo ""
	@./test_auth
	@./test_service_request
	@./test_tax_record

test_auth: test_auth.c ../src/constants.h ../src/utils.h ../src/auth.h
	$(CC) $(CFLAGS) -o $@ $<

test_service_request: test_service_request.c \
                      ../src/constants.h ../src/utils.h ../src/service_request.h
	$(CC) $(CFLAGS) -o $@ $<

test_tax_record: test_tax_record.c \
                 ../src/constants.h ../src/utils.h ../src/tax_record.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TEST_TARGETS)
	rm -rf test_data
