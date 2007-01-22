DOXYGEN=$(shell which doxygen)
doc: reference/html/index.html

reference/html/index.html:
	if test x$(DOXYGEN) = x ; then echo "doxygen is not installed, please install it!" ; exit -1 ; fi
	$(DOXYGEN) Doxyfile

clean-local:
	-rm -rf reference
	-rm doxygen-warnings.txt
