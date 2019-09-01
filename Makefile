
SHELL    = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

vpath %.1 man
vpath %.c src
vpath %.h include
vpath %.c tests

PREFIX   = /usr/local
BINDIR   = /bin

CP       = cp -f -u
RM       = rm -f
INSTALL  = install
INSTFLAGS= --strip --target-directory=$(PREFIX)$(BINDIR)

NM       = nm
NMOPTS   = --extern-only --defined-only -v --print-file-name

STRIP    = strip
STRIPOPTS= --strip-all

SRCS     = $(wildcard src/*.c)
OBJS     = $(patsubst %.c,%.o,$(notdir $(SRCS)))
ASMLISTS = $(patsubst %.c,%.asm,$(notdir $(SRCS)))

MANPAGE  = $(wildcard man/*.1)

MAPFILE  = map.file

CC       = gcc
#CFLAGS   = -std=c99 -Wall -Wextra -Wpedantic -mtune=intel -march=sandybridge -g -ggdb -O0
CFLAGS   = -std=c99 -Wall -Wextra -Wpedantic -Ofast -mtune=intel \
           -march=sandybridge -fmerge-all-constants -fmodulo-sched \
           -fmodulo-sched-allow-regmoves -fgcse-sm -fgcse-las \
           -fselective-scheduling -fsel-sched-pipelining \
           -fsel-sched-pipelining-outer-loops -fsemantic-interposition \
           -fipa-pta -fisolate-erroneous-paths-attribute \
           -ftree-parallelize-loops=16 -ftree-vrp \
           -fvariable-expansion-in-unroller -flto -fwhole-program \
           -funsafe-math-optimizations -fassociative-math -freciprocal-math \
           -ffinite-math-only -fno-trapping-math -fno-signaling-nans \
           -fno-fp-int-builtin-inexact -fsingle-precision-constant \
           -fcx-fortran-rules -fbranch-target-load-optimize \
           -fbranch-target-load-optimize2 -fbtr-bb-exclusive -fstdarg-opt \
           --param l1-cache-size=64 --param l2-cache-size=256 \
           --param allow-store-data-races=1 -fno-asynchronous-unwind-tables \
           -fno-gnu-unique -fshort-enums \
           -fno-strict-volatile-bitfields 
CPPFLAGS = -D_POSIX_C_SOURCE -D_POSIX_THREADS -D_GNU_SOURCE -D_XOPEN_SOURCE=700
LDFLAGS  = -O
LIBS     = -lm -pthread

GROFF    = groff
GROFFOPTS= -Tascii -man
GROFFPDF = -Tpdf -man

PAGER    = less

GPROF    = gprof
GPROFOPTS= 
PROFILE  = gmon.out
PROFAGGR = gmon.sum

GCOV     = gcov
GCOVOPTS = --all-blocks --branch-probabilities --branch-counts \
           --display-progress --function-summaries --human-readable \
           --use-colors --long-file-names --use-hotness-colors -s src

TARGET   = common

TESTSRCS = $(wildcard tests/*.c)
TESTOBJS = $(patsubst %.c,%.o,$(notdir $(TESTSRCS)))
TESTS    = $(basename $(TESTOBJS))

all: release

release: $(TARGET)

.PHONY: debug
debug: $(TARGET)

.PHONY: memcheck
memcheck: $(TARGET)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --expensive-definedness-checks=yes --leak-resolution=high ./$(TARGET) data/a data/b

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include    -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include -c -o $@ $^

assembly-listings: $(ASMLISTS)

%.asm: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include -S -o $@ $^ -masm=intel

.PHONY: clean-assembly-listings
clean-assembly-listings:
	$(RM) $(ASMLISTS)

.PHONY: mapfile
mapfile: $(TARGET)
	$(NM) $(NMOPTS) $^ > $(MAPFILE)

.PHONY: clean-mapfile
clean-mapfile:
	$(RM) $(MAPFILE)

.PHONY: profile
profile: gprof gcov

gprof: $(PROFILE)
	$(GPROF) $(GPROFOPTS) -s $(PROFAGGR) $(TARGET)

$(PROFILE): $(TARGET)
	./$(TARGET) data/ultra-huge data/ultra-huge2

.PHONY: gcov
gcov: $(TARGET)
	$(GCOV) $(GCOVOPTS) $(TARGET) $(notdir $(SRCS))
	$(foreach SRC,$(SRCS),$(shell $(GCOV) $(SRC)))

.PHONY: clean-gcov
clean-gcov:
	$(RM) *.{gcov,gcno,gcda,gz}

.PHONY: clean-profile
clean-profile: clean
	$(RM) $(PROFILE)

.PHONY: reprofile
reprofile: clean-profile |  profile

.PHONY: test
test: tests

tests: $(TESTS)

check-str: check-str.o str.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include    -o $@ $^ $(LDFLAGS) -lcheck

check-str.o: check-str.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include -c -o $@ $^ $(LDFLAGS)

check-file-exists: check-file-exists.o file.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include    -o $@ $^ $(LDFLAGS) -lcheck

check-file-exists.o: check-file-exists.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include -c -o $@ $^ $(LDFLAGS)

.PHONY: check
check: tests
	@./check-file-exists
	@./check-str

.PHONY: clean-tests
clean-tests: 
	$(RM) $(TESTS) $(TESTOBJS)

.PHONY: documentation
documentation: pdf

pdf: $(MANPAGE)
	$(GROFF) $(GROFFPDF) $^ > doc/pdf/$(TARGET).pdf

.PHONY: clean-pdf
clean-pdf:
	$(RM) $(wildcard doc/pdf/*.pdf)

view-manpage: $(MANPAGE)
	$(GROFF) $(GROFFOPTS) $(MANPAGE) | $(PAGER)

.PHONY: clean
clean:
	$(RM) $(wildcard vgcore.*) $(OBJS) $(TARGET)

.PHONY: clean-all
clean-all: clean clean-mapfile clean-gcov clean-profile clean-tests clean-assembly-listings

.PHONY: install
install: $(TARGET)
	$(INSTALL) $(INSTFLAGS) $^

.PHONY: uninstall
uninstall:
	$(RM) $(PREFIX)$(BINDIR)/$(TARGET)

.PHONY: help
help:
	@echo -e "Available targets:  "
	@echo -e "    all             "
	@echo -e "    documentation   "
	@echo -e "    pdf             "
	@echo -e "    view-manpage    "
	@echo -e "    profile         "
	@echo -e "    reprofile       "
	@echo -e "    assembly-listings"
