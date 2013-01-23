
DIRS = kernel tools

all modules install modules_install:
	for d in $(DIRS); do $(MAKE) -C $$d $@ || exit 1; done

clean:
	for d in $(DIRS) sdb-lib; do $(MAKE) -C $$d $@ || exit 1; done
