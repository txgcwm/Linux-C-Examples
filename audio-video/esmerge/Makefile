SUBDIRSCLEAN = tstools esmerge
SUBDIRS = tstools esmerge

all:
	@set -e; \
	for i in $(SUBDIRS); \
		do $(MAKE) $(MFLAGS) -C $$i ; done

clean:
	@for i in $(SUBDIRSCLEAN); \
		do $(MAKE) $(MFLAGS) -C $$i clean; done
