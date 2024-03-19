CC := gcc
CFLAGS := -Wall -O3 -std=c11 -fopenmp
LDFLAGS := -lgmp -lm

SRCDIR := src
INCDIR := include
BINDIR := bin
TARGET := program

SRCFILES := $(wildcard $(SRCDIR)/*.c)
OBJFILES := $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%.o,$(SRCFILES))

$(BINDIR)/$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BINDIR)/*.o $(BINDIR)/$(TARGET)
