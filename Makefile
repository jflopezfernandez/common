
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

SRCS     = $(wildcard src/*.c)
OBJS     = $(patsubst %.c,%.o,$(notdir $(SRCS)))

MANPAGE  = $(wildcard man/*.1)

MAPFILE  = map.file

CC       = gcc
# gcov
# CFLAGS   = -std=c99 -Wall -Wextra -Wpedantic -mtune=intel -march=sandybridge \
#            -g -pg --coverage -fsave-optimization-record \
#            -fdiagnostics-color=always -fprofile \
#            -fprofile-arcs -fprofile-correction -fprofile-generate \
#            -fprofile-report -fprofile-use -fprofile-values -freorder-blocks \
#            -freorder-blocks-algorithm=stc -freorder-blocks-and-partition \
#            -freorder-functions -frename-registers -frerun-cse-after-loop \
#            -ggdb -ginline-points -grecord-gcc-switches -gdescribe-dies \
#            -gvariable-location-views -p -gstatement-frontiers \
#            -fdiagnostics-show-option \
#            -ftest-coverage -ftime-report -ftime-report-details -ftracer \
#            -ftrampolines -fvar-tracking -fvar-tracking-assignments \
#            -fvar-tracking-uninit -fverbose-asm
# gprof
# CFLAGS   = -std=c99 -Wall -Wextra -Wpedantic -O3 -mtune=intel -march=sandybridge -g -pg
# Release
CFLAGS   = -std=c99 -Wall -Wextra -Wpedantic -g -ggdb -O0
CPPFLAGS = -D_POSIX_C_SOURCE -D_POSIX_THREADS -D_GNU_SOURCE -D_XOPEN_SOURCE=700
LDFLAGS  = -O
LIBS     = -lm -pthread

GROFF    = groff
GROFFOPTS= -Tascii -man
GROFFPDF = -Tpdf -man

PAGER    = less

GPROF    = gprof
GPROFOPTS= # -b -C --file-info -p -q -a -c -z
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

all: notify-building-all | release documentation
	
.PHONY: notify-building-all
notify-building-all:
	@echo -e "Building release executable and documentation..."

.PHONY: release
release: $(TARGET)

.PHONY: debug
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include    -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include -c -o $@ $^

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
	$(RM) $(OBJS) $(TARGET)

.PHONY: clean-all
clean-all: clean clean-mapfile clean-gcov clean-profile clean-tests

.PHONY: install
install: $(TARGET)
	$(INSTALL) $(INSTFLAGS) $^

.PHONY: uninstall
uninstall:
	$(RM) $(PREFIX)$(BINDIR)/$(TARGET)

.PHONY: help
help:
	@echo -e "Available targets: "
	@echo -e "    all            "
	@echo -e "    documentation  "
	@echo -e "    pdf            "
	@echo -e "    view-manpage   "
	@echo -e "    profile        "
	@echo -e "    reprofile      "
