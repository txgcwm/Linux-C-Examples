#
#libdvbpsi.a
#

CFLAGS += -I ./

LIBTSTOOLS=libtstools.a
LIBTSTOOLS_SHARED=libtstools.so

# All of our non-program object modules
OBJS = 	accessunit.o \
		avs.o \
		ac3.o \
		adts.o \
		bitdata.o \
		es.o \
		filter.o \
		fmtx.o \
		h222.o \
		h262.o \
		audio.o \
		l2audio.o \
		misc.o \
		nalunit.o \
		ps.o \
		pes.o \
		pidint.o \
		reverse.o \
	 	ts.o \
		tswrite.o

all: $(LIBTSTOOLS)

$(OBJS):  %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBTSTOOLS) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBTSTOOLS) $(OBJS)

$(OBJS): Makefile

clean:
	$(RM) *.[oa] $(LIBTSTOOLS)* $(LIBTSTOOLS_SHARED)*

