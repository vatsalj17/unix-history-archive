all:
	echo "Nothing to make; \"make cp\" to install."
cp:
	-mkdir ${DESTDIR}/usr/lib/me
	cp acm.me ${DESTDIR}/usr/lib/me/acm.me
	cp chars.me ${DESTDIR}/usr/lib/me/chars.me
	cp deltext.me ${DESTDIR}/usr/lib/me/deltext.me
	cp eqn.me ${DESTDIR}/usr/lib/me/eqn.me
	cp float.me ${DESTDIR}/usr/lib/me/float.me
	cp footnote.me ${DESTDIR}/usr/lib/me/footnote.me
	cp index.me ${DESTDIR}/usr/lib/me/index.me
	cp local.me ${DESTDIR}/usr/lib/me/local.me
	cp null.me ${DESTDIR}/usr/lib/me/null.me
	cp sh.me ${DESTDIR}/usr/lib/me/sh.me
	cp tbl.me ${DESTDIR}/usr/lib/me/tbl.me
	cp thesis.me ${DESTDIR}/usr/lib/me/thesis.me
	cp tmac.e ${DESTDIR}/usr/lib/tmac/tmac.e
	ed - ${DESTDIR}/usr/lib/me/acm.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/chars.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/deltext.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/eqn.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/float.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/footnote.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/index.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/local.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/null.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/sh.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/tbl.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/me/thesis.me <strip-me.ed
	ed - ${DESTDIR}/usr/lib/tmac/tmac.e <strip-me.ed
	cp revisions ${DESTDIR}/usr/lib/me

clean:
