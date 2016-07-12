AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in
#Tizen Only
#objects breaks the build on windows hosting environment.
#windows hosting efl doesn't require objects.
if ELEMENTARY_WINDOWS_BUILD
SUBDIRS              = themes images desktop edje_externals
else
SUBDIRS              = themes images objects desktop edje_externals
endif
