DIRS = spl xfs-interface xsm_dev

all:
	set -e; for d in $(DIRS); do $(MAKE) -C $$d ; done
test:
	./scripts/run_tests.sh
exec:
	./scripts/exec.sh
clean:
	@cd spl && make clean
	@cd xfs-interface && make clean
	@cd xsm_dev && make clean
	rm -r build
