## Run automake on this file to produce Makefile.in

dist_doc_DATA = FORMAT1.3 FORMAT1.4 FORMAT2.0 FORMAT2.1 FORMAT3.0 \
	FORMAT3.1 FORMAT3.2 xfig-howto.pdf xfig_ref_en.pdf

doc_DATA = xfig_man.html

man1_MANS = xfig.man

EXTRA_DIST  = html MAKEPS

MAINTAINERCLEANFILES = Makefile.in

# Below, $< is not fully portable, see info autoconf "Portable Make",
# use $? instead. man is sensitive to spaces after the options.
# Probably, groff -Thtml -mman xfig.man > $@ is more portable?

# Do not make xfig_man.html at build time, like so:
#xfig_man.html: xfig.man
#	man -Thtml -l $? >$@
# but make xfig_man.html.in at distribution time.
# Chicken and egg problem: To configure and make dist, the maintainer
# must provide a xfig_man.html.in, which is then re-created at
# make dist time. An empty file is sufficient, e.g.,
# touch xfig_man.html.in. But see below, for the maintainer.
dist-hook:
	chmod u+w "$(distdir)" \
	&& { test -f "$(distdir)/xfig_man.html.in" \
		&& { test -w "$(distdir)/xfig_man.html.in" \
			|| chmod u+w "$(distdir)/xfig_man.html.in"; } \
		|| :; }
	man -Thtml -l "$(srcdir)/xfig.man.in" > "$(distdir)/xfig_man.html.in"

# If the maintainer wants to compile, xfig.man.in needs to be present here
# in the source tree. Create it manually and keep it. If xfig_man.html.in is
# modified, xfig.man is instantiated automatically.
$(srcdir)/xfig_man.html.in: $(srcdir)/xfig.man.in
	man -Thtml -l xfig.man.in > xfig_man.html.in
#	$(SHELL) $(top_builddir)/config.status --file=xfig_man.html

$(srcdir)/xfig.man.in:
	touch $@

install-data-hook:
	cp -Rp "$(srcdir)/html" "$(DESTDIR)$(htmldir)" \
	&& chmod -R u+rwX "$(DESTDIR)$(htmldir)"
	cd "$(DESTDIR)$(htmldir)/html" \
	&& rm -f ja_JP.ujis && $(LN_S) japanese ja_JP.ujis

uninstall-local:
	rm -rf "$(DESTDIR)$(htmldir)/html"
