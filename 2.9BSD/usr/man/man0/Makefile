FMTR=	vtroff

trman: cover introduction table-contents permindex

cover:
	${FMTR} title copyr

introduction:
	${FMTR} -ms pref 
	${FMTR} -ms intro

table-contents:	toc1 toc2 toc3 toc4 toc5 toc6 toc7 toc8
	${FMTR} -n11 contents

permindex: ptxindex
	${FMTR} -n21 ptx.in

ptxx: 	tocx1 tocx2 tocx3 tocx4 tocx5 tocx6 tocx7 tocx8
	makeindex t
	
tocx1 tocx2 tocx3 tocx4 tocx5 tocx6 tocx7 tocx8: makeindex
	makeindex all

toc1 toc2 toc3 toc4 toc5 toc6 toc7 toc8: makeindex
	makeindex all
