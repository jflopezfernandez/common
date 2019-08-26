
vpath %.1 man
vpath %.c src
vpath %.h include

PREFIX   = /usr/local
BINDIR   = /bin

CP       = cp -f -u
RM       = rm -f
INSTALL  = install
INSTFLAGS= --strip --target-directory=$(PREFIX)$(BINDIR)

OBJS     = $(patsubst %.c,%.o,$(notdir $(wildcard src/*.c)))

MANPAGE  = $(wildcard man/*.1)

CC       = gcc
CFLAGS   = -std=c99 -Wall -Wextra -Wpedantic -O3 -mtune=intel -march=sandybridge
CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE -D_XOPEN_SOURCE=700
LDFLAGS  = -O
LIBS     = -lm

TARGET   = common

all: $(TARGET)

.PHONY: help
help:
	@echo -e "Available targets: "
	@echo -e "    all            "
	@echo -e "    documentation  "
	@echo -e "    pdf            "
	@echo -e "    view-manpage   "

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include    -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I include -c -o $@ $^

documentation: pdf

pdf: $(MANPAGE)
	groff -Tpdf -man $^ > doc/pdf/common.pdf

.PHONY: clean-pdf
clean-pdf:
	$(RM) $(wildcard doc/pdf/*.pdf)

view-manpage: $(MANPAGE)
	groff -Tascii -man $(MANPAGE) | less

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: install
install: $(TARGET)
	$(INSTALL) $(INSTFLAGS) $^

.PHONY: uninstall
uninstall:
	$(RM) $(PREFIX)$(BINDIR)/$(TARGET)
