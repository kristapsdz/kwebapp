.SUFFIXES: .dot .svg .1 .1.html .5 .5.html
.PHONY: tests

include Makefile.configure

VERSION_MAJOR	 = 0
VERSION_MINOR	 = 8
VERSION_BUILD	 = 4
VERSION		:= $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_BUILD)
LIBOBJS		 = comments.o \
		   compats.o \
		   config.o \
		   linker.o \
		   parser.o \
		   printer.o \
		   protos.o \
		   writer.o
OBJS		 = audit.o \
		   main.o \
		   header.o \
		   javascript.o \
		   source.o \
		   sql.o \
		   xliff.o
HTMLS		 = archive.html \
		   index.html \
		   ort.1.html \
		   ort-audit.1.html \
		   ort-audit-gv.1.html \
		   ort-audit-json.1.html \
		   ort-c-header.1.html \
		   ort-c-source.1.html \
		   ort-javascript.1.html \
		   ort-sql.1.html \
		   ort-sqldiff.1.html \
		   ort-xliff.1.html \
		   ort.5.html
WWWDIR		 = /var/www/vhosts/kristaps.bsd.lv/htdocs/openradtool
MAN1S		 = ort.1 \
		   ort-audit.1 \
		   ort-audit-gv.1 \
		   ort-audit-json.1 \
		   ort-c-header.1 \
		   ort-c-source.1 \
		   ort-javascript.1 \
		   ort-sql.1 \
		   ort-sqldiff.1 \
		   ort-xliff.1
DOTAREXEC	 = configure
DOTAR		 = audit.c \
		   audit.css \
		   audit.html \
		   audit.js \
		   b64_ntop.c \
		   comments.c \
		   compats.c \
		   config.c \
		   extern.h \
		   gensalt.c \
		   header.c \
		   javascript.c \
		   jsmn.c \
		   $(MAN1S) \
		   ort.5 \
		   ort.h \
		   linker.c \
		   Makefile \
		   main.c \
		   parser.c \
		   printer.c \
		   protos.c \
		   source.c \
		   sql.c \
		   tests.c \
		   test.c \
		   writer.c \
		   xliff.c
XMLS		 = test.xml.xml \
		   versions.xml
IHTMLS		 = audit-example.txt.html \
		   audit-out.js \
		   db.txt.html \
		   db.fr.xml.html \
		   db.h.html \
		   db.c.html \
		   db.sql.html \
		   db.old.txt.html \
		   db.update.sql.html \
		   db.js.html \
		   db.ts.html \
		   db.trans.txt.html \
		   test.c.html
BINS		 = ort \
		   ort-audit \
		   ort-audit-gv \
		   ort-audit-json \
		   ort-c-header \
		   ort-c-source \
		   ort-javascript \
		   ort-sql \
		   ort-sqldiff \
		   ort-xliff
IMAGES		 = index.svg \
		   index2.svg \
		   index-fig0.svg \
		   index-fig1.svg \
		   index-fig2.svg \
		   index-fig3.svg \
		   index-fig4.svg \
		   index-fig5.svg \
		   index-fig6.svg \
		   index-fig7.svg
DIFFARGS	 = -I '^[/ ]\*.*' -I '^\# define KWBP_.*' -w

# FreeBSD's make doesn't support CPPFLAGS.
# CFLAGS += $(CPPFLAGS)

all: $(BINS)

afl::
	$(MAKE) clean
	$(MAKE) all CC=afl-gcc
	cp $(BINS) afl

ort: main.o libort.a
	$(CC) -o $@ main.o libort.a

libort.a: $(LIBOBJS)
	$(AR) rs $@ $(LIBOBJS)

ort-c-source: source.o libort.a
	$(CC) -o $@ source.o libort.a

ort-c-header: header.o libort.a
	$(CC) -o $@ header.o libort.a

ort-javascript: javascript.o libort.a
	$(CC) -o $@ javascript.o libort.a

ort-sql: sql.o libort.a
	$(CC) -o $@ sql.o libort.a

ort-sqldiff: sql.o libort.a
	$(CC) -o $@ sql.o libort.a

ort-audit: audit.o libort.a
	$(CC) -o $@ audit.o libort.a

ort-audit-gv: audit.o libort.a
	$(CC) -o $@ audit.o libort.a

ort-audit-json: audit.o libort.a
	$(CC) -o $@ audit.o libort.a

ort-xliff: xliff.o libort.a
	$(CC) -o $@ xliff.o libort.a $(LDFLAGS) -lexpat

www: $(IMAGES) $(HTMLS) openradtool.tar.gz openradtool.tar.gz.sha512 atom.xml

installwww: www
	mkdir -p $(WWWDIR)
	mkdir -p $(WWWDIR)/snapshots
	$(INSTALL_DATA) *.html *.css *.js $(IMAGES) atom.xml $(WWWDIR)
	$(INSTALL_DATA) openradtool.tar.gz openradtool.tar.gz.sha512 $(WWWDIR)/snapshots
	$(INSTALL_DATA) openradtool.tar.gz $(WWWDIR)/snapshots/openradtool-$(VERSION).tar.gz
	$(INSTALL_DATA) openradtool.tar.gz.sha512 $(WWWDIR)/snapshots/openradtool-$(VERSION).tar.gz.sha512

# Something to run before doing a release: just makes sure that we have
# the given version documented and that make and make install works
# when run on the distributed tarball.
# Also checks that our manpages are nice.

distcheck: openradtool.tar.gz.sha512 openradtool.tar.gz
	mandoc -Tlint -Wwarning $(MAN1S)
	newest=`grep "<h3>" versions.xml | head -n1 | sed 's![ 	]*!!g'` ; \
	       [ "$$newest" == "<h3>$(VERSION)</h3>" ] || \
		{ echo "Version $(VERSION) not newest in versions.xml" 1>&2 ; exit 1 ; }
	rm -rf .distcheck
	sha512 -C openradtool.tar.gz.sha512 openradtool.tar.gz
	mkdir -p .distcheck
	tar -zvxpf openradtool.tar.gz -C .distcheck
	( cd .distcheck/openradtool-$(VERSION) && ./configure PREFIX=prefix \
		CPPFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS)" LDADD="$(LDADD)" )
	( cd .distcheck/openradtool-$(VERSION) && make )
	( cd .distcheck/openradtool-$(VERSION) && make install )
	( cd .distcheck/openradtool-$(VERSION)/prefix && find . -type f )
	rm -rf .distcheck

version.h: Makefile
	( echo "#define VERSION \"$(VERSION)\"" ; \
	  echo "#define VSTAMP `echo $$((($(VERSION_BUILD)+1)+($(VERSION_MINOR)+1)*100+($(VERSION_MAJOR)+1)*10000))`" ; ) >$@

header.o source.o: version.h

paths.h: Makefile
	( echo "#define PATH_GENSALT \"$(SHAREDIR)/openradtool/gensalt.c\"" ; \
	  echo "#define PATH_B64_NTOP \"$(SHAREDIR)/openradtool/b64_ntop.c\"" ; \
	  echo "#define PATH_JSMN \"$(SHAREDIR)/openradtool/jsmn.c\"" ; ) >$@

source.o: paths.h

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	mkdir -p $(DESTDIR)$(MANDIR)/man5
	mkdir -p $(DESTDIR)$(SHAREDIR)/openradtool
	$(INSTALL_MAN) $(MAN1S) $(DESTDIR)$(MANDIR)/man1
	$(INSTALL_MAN) ort.5 $(DESTDIR)$(MANDIR)/man5
	$(INSTALL_DATA) audit.html audit.css audit.js $(DESTDIR)$(SHAREDIR)/openradtool
	$(INSTALL_DATA) b64_ntop.c jsmn.c gensalt.c $(DESTDIR)$(SHAREDIR)/openradtool
	$(INSTALL_PROGRAM) $(BINS) $(DESTDIR)$(BINDIR)

uninstall:
	@for f in $(MAN1S); do \
		echo rm -f $(DESTDIR)$(MANDIR)/man1/$$f ; \
		rm -f $(DESTDIR)$(MANDIR)/man1/$$f ; \
	done
	rm -f $(DESTDIR)$(MANDIR)/man5/ort.5
	rm -f $(DESTDIR)$(SHAREDIR)/openradtool/audit.{html,css,js}
	rm -f $(DESTDIR)$(SHAREDIR)/openradtool/{b64_ntop,jsmn,gensalt}.c
	rmdir $(DESTDIR)$(SHAREDIR)/openradtool
	@for f in $(BINS); do \
		echo rm -f $(DESTDIR)$(BINDIR)/$$f ; \
		rm -f $(DESTDIR)$(BINDIR)/$$f ; \
	done

openradtool.tar.gz.sha512: openradtool.tar.gz
	sha512 openradtool.tar.gz >$@

openradtool.tar.gz: $(DOTAR) $(DOTAREXEC)
	mkdir -p .dist/openradtool-$(VERSION)/
	install -m 0444 $(DOTAR) .dist/openradtool-$(VERSION)
	install -m 0555 $(DOTAREXEC) .dist/openradtool-$(VERSION)
	( cd .dist/ && tar zcf ../$@ ./ )
	rm -rf .dist/

test: test.o db.o db.db
	$(CC) -o $@ test.o db.o $(LDFLAGS) -lsqlbox -lsqlite3 -lpthread $(LDADD)

audit-out.js: ort-audit-json audit-example.txt
	./ort-audit-json user audit-example.txt >$@

db.o: db.c db.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c db.c

test.o: test.c db.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c test.c

db.c: ort-c-source db.txt
	./ort-c-source db.txt >$@

db.h: ort-c-header db.txt
	./ort-c-header db.txt >$@

db.sql: ort-sql db.txt
	./ort-sql db.txt >$@

db.js: ort-javascript db.txt
	./ort-javascript db.txt >$@

db.ts: ort-javascript db.txt
	./ort-javascript -t db.txt >$@

db.update.sql: ort-sqldiff db.old.txt db.txt
	./ort-sqldiff db.old.txt db.txt >$@

db.trans.txt: ort-xliff db.txt db.fr.xml
	./ort-xliff -j db.txt db.fr.xml >$@

db.db: db.sql
	rm -f $@
	sqlite3 $@ < db.sql

$(LIBOBJS) $(OBJS): config.h extern.h ort.h

.5.5.html:
	mandoc -Ostyle=https://bsd.lv/css/mandoc.css -Thtml $< >$@

.1.1.html:
	mandoc -Ostyle=https://bsd.lv/css/mandoc.css -Thtml $< >$@

.dot.svg:
	dot -Tsvg $< > tmp-$@
	xsltproc --novalid notugly.xsl tmp-$@ >$@
	rm tmp-$@

db.txt.xml: db.txt
	( echo "<article data-sblg-article=\"1\">" ; \
	  highlight -l --enclose-pre --src-lang=conf -f db.txt ; \
	  echo "</article>" ; ) >$@

db.fr.xml.html: db.fr.xml
	highlight -s whitengrey -I -l --src-lang=xml db.fr.xml | sed 's!ISO-8859-1!UTF-8!g' >$@

db.h.xml: db.h
	( echo "<article data-sblg-article=\"1\">" ; \
	  highlight -l --enclose-pre --src-lang=c -f db.h ; \
	  echo "</article>" ; ) >$@

db.sql.xml: db.sql
	( echo "<article data-sblg-article=\"1\">" ; \
	  highlight -l --enclose-pre --src-lang=sql -f db.sql ; \
	  echo "</article>" ; ) >$@

db.update.sql.xml: db.update.sql
	( echo "<article data-sblg-article=\"1\">" ; \
	  highlight -l --enclose-pre --src-lang=sql -f db.update.sql ; \
	  echo "</article>" ; ) >$@

test.c.html: test.c
	highlight -s whitengrey -I -l --src-lang=c test.c >$@

test.xml.xml: test.xml
	( echo "<article data-sblg-article=\"1\">" ; \
	  highlight -l --enclose-pre --src-lang=xml -f test.xml ; \
	  echo "</article>" ; ) >$@

db.c.html: db.c
	highlight -s whitengrey -I -l --src-lang=c db.c >$@

db.txt.html: db.txt
	highlight -s whitengrey -I -l --src-lang=c db.txt >$@

audit-example.txt.html: audit-example.txt
	highlight -s whitengrey -I -l --src-lang=c audit-example.txt >$@

db.old.txt.html: db.old.txt
	highlight -s whitengrey -I -l --src-lang=c db.old.txt >$@

db.h.html: db.h
	highlight -s whitengrey -I -l --src-lang=c db.h >$@

db.sql.html: db.sql
	highlight -s whitengrey -I -l --src-lang=sql db.sql >$@

db.js.html: db.js
	highlight -s whitengrey -I -l --src-lang=js db.js >$@

db.ts.html: db.ts
	highlight -s whitengrey -I -l --src-lang=js db.ts >$@

db.update.sql.html: db.update.sql
	highlight -s whitengrey -I -l --src-lang=sql db.update.sql >$@

db.trans.txt.html: db.trans.txt
	highlight -s whitengrey -I -l --src-lang=c db.trans.txt | sed 's!ISO-8859-1!UTF-8!g' >$@

highlight.css:
	highlight --print-style -s whitengrey -O xhtml --stdout >$@

index.html: index.xml $(XMLS) $(IHTMLS) highlight.css
	sblg -s cmdline -t index.xml -o- $(XMLS) >$@

archive.html: archive.xml versions.xml
	sblg -s date -t archive.xml -o- versions.xml >$@

TODO.xml: TODO.md
	( echo "<article data-sblg-article=\"1\">" ; \
	  lowdown -Thtml TODO.md ; \
	  echo "</article>" ; ) >$@

atom.xml: versions.xml atom-template.xml
	sblg -s date -a versions.xml >$@

# Remove what is built by "all" and "www".

clean:
	rm -f $(BINS) version.h paths.h $(LIBOBJS) libort.a test test.o
	rm -f db.c db.h db.o db.sql db.js db.ts db.ts db.update.sql db.db db.trans.txt
	rm -f openradtool.tar.gz openradtool.tar.gz.sha512
	rm -f $(IMAGES) highlight.css $(HTMLS) atom.xml
	rm -f db.txt.xml db.h.xml db.sql.xml db.update.sql.xml test.xml.xml $(IHTMLS) TODO.xml
	rm -f source.o header.o javascript.o sql.o audit.o main.o xliff.o

# Remove both what can be built and what's built by ./configure.

distclean: clean
	rm -f config.h config.log Makefile.configure

# The tests go over each type of file in "regress".
# Each test has a configuration (.ort) and a type to cross-check: a C
# file (.c), header file (.h), or configuration (.orf).
# First, make sure the .ort generates a similar C, header, or config.
# Second, create a configuration from the configuration and try again,
# making sure that it's the same.

tests: all
	@tmp=`mktemp` ; \
	tmp2=`mktemp` ; \
	for f in regress/*.c ; do \
		bf=`basename $$f .c`.ort ; \
		bff=`basename $$f .c`.orf ; \
		set +e ; \
		echo "ort-c-source regress/$$bf... " ; \
		./ort-c-source regress/$$bf > $$tmp ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		diff $(DIFFARGS) $$tmp $$f >/dev/null 2>&1 ; \
		if [ $$? -ne 0 ] ; then \
			diff $(DIFFARGS) -u $$tmp $$f ; \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		echo "ort-c-source (cross-check...)" ; \
		./ort regress/$$bf > $$tmp2 ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		./ort-c-source $$tmp2 > $$tmp ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		diff $(DIFFARGS) $$tmp $$f >/dev/null 2>&1 ; \
		if [ $$? -ne 0 ] ; then \
			diff $(DIFFARGS) -u $$tmp $$f ; \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		set -e ; \
	done ; \
	for f in regress/*.h ; do \
		bf=`basename $$f .h`.ort ; \
		echo "ort-c-header regress/$$bf... " ; \
		set +e ; \
		./ort-c-header regress/$$bf > $$tmp ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		diff $(DIFFARGS) $$tmp $$f >/dev/null 2>&1 ; \
		if [ $$? -ne 0 ] ; then \
			diff $(DIFFARGS) -u $$tmp $$f ; \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		echo "ort-c-header (cross-check...)" ; \
		./ort regress/$$bf > $$tmp2 ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		./ort-c-header $$tmp2 > $$tmp ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		diff $(DIFFARGS) $$tmp $$f >/dev/null 2>&1 ; \
		if [ $$? -ne 0 ] ; then \
			diff $(DIFFARGS) -u $$tmp $$f ; \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		set -e ; \
	done ; \
	for f in regress/*.orf ; do \
		bf=`basename $$f .orf`.ort ; \
		echo "ort regress/$$bf... " ; \
		set +e ; \
		./ort regress/$$bf > $$tmp ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		diff -I '^$$' -w $$tmp $$f >/dev/null 2>&1 ; \
		if [ $$? -ne 0 ] ; then \
			diff -i '^$$' -w -u $$tmp $$f ; \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		echo "ort (cross-check...)" ; \
		./ort regress/$$bf > $$tmp2 ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		./ort $$tmp2 > $$tmp ; \
		if [ $$? -ne 0 ] ; then \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		diff -I '^$$' -w $$tmp $$f >/dev/null 2>&1 ; \
		if [ $$? -ne 0 ] ; then \
			diff -i '^$$' -w -u $$tmp $$f ; \
			rm $$tmp $$tmp2 ; \
			exit 1 ; \
		fi ; \
		set -e ; \
	done ; \
	rm $$tmp $$tmp2

