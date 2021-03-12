CC?=
CFLAGS?=
LDFLAGS?=

SRCFILE=$(wildcard *.c)
OBJFILE=$(patsubst %.c,../build/$(TARGET)/%.o,$(SRCFILE))
DEPFILE=$(OBJFILE:%.o=%.d)

TARGET?=

all: ../build/libs/$(TARGET).so

ifneq "$(MAKECMDGOALS)" "clean"
  -include $(DEPFILE)
endif

../build/libs/$(TARGET).so: $(OBJFILE)
	@mkdir -p $(@D)
	$(CC) -Wl,-soname,$(TARGET).so.1 $(OBJFILE) $(LDFLAGS) -o $@

../build/$(TARGET)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	@rm -f $(OBJFILE) $(DEPFILE) ../build/libs/$(TARGET).so