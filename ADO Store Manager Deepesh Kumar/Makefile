.PHONY: all clean

all: test_storemanager

test_storemanager: test_assign1_1.c storage_mgr.c dberror.c
	gcc -o test_storemanager test_assign1_1.c storage_mgr.c dberror.c

clean:
ifeq ($(OS),Windows_NT)
	del /Q test_storemanager.exe 2>nul || exit 0
else
	rm -f test_storemanager
endif