docfiles=dynamic-modules.txt mainpage.txt

doc : reference/html/index.html

DOXYGEN=$(shell which doxygen)

reference/html/index.html: Doxyfile
	@if test x$(DOXYGEN) = x ; \
     then "echo ARGH, you do not have doxygen installed. Please install it !" ;exit -1;fi
	$(DOXYGEN) Doxyfile

clean-local:
	-rm -rf reference doxygen-warnings.txt

EXTRA_DIST=Doxyfile.in $(docfiles)


