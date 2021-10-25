.SUFFIXES:
default:

PRIVATE_FILES=Lab.key.ipynb admin .git

.PHONY: create-labs
create-labs:
	cse142 lab delete -f compiler-bench
	cse142 lab delete -f compiler
	cse142 lab create --name "Lab 2: The Compiler (Benchmark)" --short-name "compiler-bench" --docker-image stevenjswanson/cse142l-swanson-runner:latest --execution-time-limit 0:05:00 --total-time-limit 1:00:00 --due-date 2021-10-26T23:59:59 --starter-repo https://github.com/CSE142/fa21-CSE142L-compiler-starter.git --starter-branch main
	cse142 lab create --name "Lab 2: The Compiler" --short-name "compiler" --docker-image stevenjswanson/cse142l-swanson-runner:latest --execution-time-limit 0:05:00 --total-time-limit 1:00:00 --due-date 2021-10-26T23:59:59


COMPILER=gcc-8
include $(ARCHLAB_ROOT)/cse141.make

STUDENT_EDITABLE_FILES=sum.cpp config.make


ifeq ($(DEVEL_MODE),yes)
        # DEVEL_MODE is set to yes, when they run their code locally.
        # This means that libarchlab can't access the performance
        # counters, so you shouldn't ask for any or their executabel
        # won't run.
	CMD_LINE_ARGS=--stat runtime=ARCHLAB_WALL_TIME $(EXTRA_OPTIONS)
else
        # This is what gets run on the server.
	CMD_LINE_ARGS=--stat-set default.cfg $(EXTRA_OPTIONS)
endif




.PHONY: autograde
autograde: sum.exe regressions.json
	./sum.exe --stats autograde.csv --stat=ET=ARCHLAB_WALL_TIME --function sum_of_locations_solution sum_of_locations  --stat-set bench.cfg --MHz 3500

$(BUILD)sum.o: Makefile config.make
sum.exe: $(BUILD)sum.o $(BUILD)sum_main.o $(BUILD)sum_baseline.o

$(BUILD)sum.o: OPTIMIZE=$(SUM_OPTIMIZE)

$(BUILD)microbench.o: OPTIMIZE=$(MICROBENCH_OPTIMIZE)
$(BUILD)microbench.s: OPTIMIZE=$(MICROBENCH_OPTIMIZE)

CZOO_OPTIMIZE=-O3 -fkeep-inline-functions #-funroll-all-loops
SORT_OPTIMIZE=-O3 -fkeep-inline-functions #-funroll-all-loops

hello_fiddle.exe: $(BUILD)hello_fiddle.o

$(BUILD)find_min.o: OPTIMIZE=-O3
$(BUILD)find_min.s: OPTIMIZE=-O3
find_min.exe:   $(BUILD)find_min.o $(BUILD)main.o 


$(BUILD)sort.o: Makefile
$(BUILD)sort_O3.o: Makefile


$(BUILD)czoo.o: Makefile
$(BUILD)czoo_O3.o: Makefile

$(BUILD)czoo.s: Makefile
$(BUILD)czoo_O3.s: Makefile

czoo.exe: $(BUILD)czoo.o $(BUILD)main.o 
czoo_O3.exe: $(BUILD)czoo_O3.o $(BUILD)main.o

virt.exe: $(BUILD)virt.o $(BUILD)main.o

sort.exe: $(BUILD)sort.o $(BUILD)main.o 
sort_O1.exe: $(BUILD)sort_O1.o $(BUILD)main.o
sort_O2.exe: $(BUILD)sort_O2.o $(BUILD)main.o
sort_O3.exe: $(BUILD)sort_O3.o $(BUILD)main.o
sort_Og.exe: $(BUILD)sort_Og.o $(BUILD)main.o

czoo_O3.cpp : czoo.cpp
	cp $^ $@

sort_O1.cpp : sort.cpp
	cp $^ $@
sort_O2.cpp : sort.cpp
	cp $^ $@
sort_O3.cpp : sort.cpp
	cp $^ $@
sort_Og.cpp : sort.cpp
	cp $^ $@



$(BUILD)czoo_O3.o: OPTIMIZE=$(CZOO_OPTIMIZE)
$(BUILD)czoo_O3.s: OPTIMIZE=$(CZOO_OPTIMIZE)

$(BUILD)sort_Og.o: OPTIMIZE=-Og
$(BUILD)sort_Og.s: OPTIMIZE=-Og

$(BUILD)sort_O1.o: OPTIMIZE=-O1
$(BUILD)sort_O1.s: OPTIMIZE=-O1

$(BUILD)sort_O2.o: OPTIMIZE=-O2
$(BUILD)sort_O2.s: OPTIMIZE=-O2

$(BUILD)sort_O3.o: OPTIMIZE=-O3
$(BUILD)sort_O3.s: OPTIMIZE=-O3

if_ex.exe: $(BUILD)if_ex.o
loop_if.exe: $(BUILD)loop_if.o
cg1.exe: $(BUILD)cg1.o
if_else_if_else.exe: $(BUILD)if_else_if_else.o
fiddle_sort.exe: $(BUILD)fiddle_sort.o
mangle.exe: $(BUILD)mangle.o
reg_assign.exe: $(BUILD)reg_assign.o
CSE.exe: $(BUILD)CSE.o
SR.exe: $(BUILD)SR.o
SR2.exe: $(BUILD)SR2.o
unroll1.exe: $(BUILD)unroll1.o
unroll-not.exe: $(BUILD)unroll-not.o
classes.exe: $(BUILD)classes.o
CP.exe: $(BUILD)CP.o
CP2.exe: $(BUILD)CP2.o
LICM.exe: $(BUILD)LICM.o
prologue.exe: $(BUILD)prologue.o
inline1.exe: $(BUILD)inline1.o
inline2.exe: $(BUILD)inline2.o
sort_revisited.exe: $(BUILD)sort_revisited.o
mimic_1.exe: $(BUILD)mimic_1.o
loop_func.exe: $(BUILD)loop_func.o

#-include admin/solution.make

run_tests.exe: $(BUILD)sum.o $(BUILD)sum_baseline.o

regressions.json: run_tests.exe
	./run_tests.exe --gtest_output=json:$@


#$(BUILD)config.make: $(LAB_SUBMISSION_DIR)/config.make
#	mkdir -p $(BUILD)
#	cp $^ $@

#$(BUILD)sum.cpp: $(LAB_SUBMISSION_DIR)/sum.cpp
#	mkdir -p $(BUILD)
#	cp $^ $@

#-include $(BUILD)config.make
include $(DJR_JOB_ROOT)/$(LAB_SUBMISSION_DIR)/config.make
