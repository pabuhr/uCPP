## Define the appropriate configuration variables.

PACKAGE = /u0/usystem/software/u++-6.1.0

## Include the architecture dependent definitions.

include ${PACKAGE}/CONFIG

## Define the path names of important directories.

SRCDIR = ${PACKAGE}/doc/refman
EXPDIR = ${PACKAGE}/src/examples

## Define some of the commands.

TeXLIB = .:../TeXmacros:
LaTeX  = TEXINPUTS=${TeXLIB} && export TEXINPUTS && latex
BibTeX = BSTINPUTS=${TeXLIB} && export BSTINPUTS && bibtex
Lgrind = /u/pabuhr/bin/lgrind
Lgrindefs = /u/pabuhr/lib/vgrindefs

## Define some of the tools.

include ${PACKAGE}/src/MakeTools

## Define the text source files.

SOURCES = ${addprefix ${SRCDIR}/, ${addsuffix .tex, \
uC++ \
} }

FIGURES = ${addsuffix .tex, \
CyclicFullCoroutine \
uC++MutexObject \
DirectComm \
ServerStructures \
ClientServerUnconnected \
ClientServerConnected \
RunTimeStructure \
ReadyQueue \
DSLNodes \
DSLStack \
DSLQueue \
DSLSequence \
}

PROGRAMS = ${addsuffix .tex, \
BinaryInsertionSort \
RWEx1 \
MonAcceptBB \
MonConditionBB \
TaskAcceptBB \
SemaphoreBB \
LOOK \
File \
ClientUNIXDGRAM \
ServerUNIXDGRAM \
ClientINETSTREAM \
ServerINETSTREAM \
}

## Define the documents that need to be made.

DOCUMENT = ${SRCDIR}/uC++.ps

# Directives #

all : ${DOCUMENT}

install : all ${INSTALLDOCDIR}
	for file in ${notdir ${DOCUMENT}} ; do \
		if cmp -s $$file ${INSTALLDOCDIR}/$$file ; then . ; else ${INSTALL} $$file ${INSTALLDOCDIR}/$$file ; fi \
	done

depend :

clean :
	rm -f *.zzz *.bbl *.aux *.dvi *.idx *.ilg *.ind *.log *.toc *.blg *.out *.brf ${SOURCES} ${FIGURES} ${PROGRAMS} ${DOCUMENT}

uninstall :
	rm -f ${addprefix ${INSTALLDOCDIR}/, ${notdir ${DOCUMENT}}}

# File Dependencies #

${INSTALLDOCDIR} :
	mkdir $@

${DOCUMENT} : ${basename ${DOCUMENT}}.dvi
	dvips $< -o $@

${basename ${DOCUMENT}}.dvi : Makefile ${PROGRAMS} ${FIGURES} ${SOURCES} ${basename ${DOCUMENT}}.bbl
	${LaTeX} ${basename $@}.tex
	if fgrep -s "Label(s) may have changed" ${basename $@}.log ; then ${LaTeX} ${basename $@}.tex ; fi
	makeindex ${basename $@}.idx
	${LaTeX} ${basename $@}.tex
	${LaTeX} ${basename $@}.tex		% to get index title in toc

${basename ${DOCUMENT}}.bbl : ${basename ${DOCUMENT}}.tex
	if [ ! -r ${basename $@}.ind ] ; then touch ${basename $@}.ind ; fi
	if [ ! -r ${basename $@}.aux ] ; then ${LaTeX} ${basename $@}.tex ; fi
	-${BibTeX} ${basename $@}
	${LaTeX} ${basename $@}.tex
	-${BibTeX} ${basename $@}

# This recipe is done only on plg (architecture master machine) because of the
# additional preprocessing pass over the document to handle lgrind macros.

HOSTNAME = ${shell hostname}
ifeq (${HOSTNAME},plg2)
${basename ${DOCUMENT}}.tex : %.tex : %.pretex
	${Lgrind} -e -d ${Lgrindefs} -lu++ -tab 4 $< > $@

## Define the default recipes.

vpath %.cc ${EXPDIR}:${EXPDIR}/IO	# add prefix for PROGRAMS

%.tex : %.cc
	${Lgrind} -i -d ${Lgrindefs} -lu++ -tab 4 $< > $@
endif

%.tex : %.fig
	fig2dev -L eepic $< > $@

## Local Variables: ##
## compile-command: "make" ##
## End: ##
