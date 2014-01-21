%{!?_rel:%{expand:%%global _rel 0.enl%{?dist}}}
%global _missing_doc_files_terminate_build 0

Summary: EFL toolkit for small touchscreens
Name: elementary
Version: 1.8.3
Release: %{_rel}
License: Lesser GPL
Group: System Environment/Libraries
URL: http://trac.enlightenment.org/e/wiki/Elementary
Source: %{name}-%{version}.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Rui Miguel Silva Seabra <rms@1407.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
#BuildSuggests: xorg-x11-devel, vim-enhanced
BuildRequires: efl-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Elementary is a widget set. It is a new-style of widget set much more canvas
object based than anything else. Why not ETK? Why not EWL? Well they both
tend to veer away from the core of Evas, Ecore and Edje a lot to build their
own worlds. Also I wanted something focused on embedded devices -
specifically small touchscreens. Unlike GTK+ and Qt, 75% of the "widget set"
is already embodied in a common core - Ecore, Edje, Evas etc. So this
fine-grained library splitting means all of this is shared, just a new
widget "personality" is on top. And that is... Elementary, my dear watson.
Elementary.


%package devel
Summary: Elementary headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}, %{name}-bin = %{version}
Requires: efl-devel

%description devel
Headers, static libraries, test programs and documentation for Elementary


%package bin
Summary: Elementary file compiler/decompiler suite
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description bin
Elmementary programs


%prep
%setup -q


%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}


%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install
test -x `which doxygen` && sh gendoc || :

%{find_lang} %{name}


%post
/sbin/ldconfig || :


%postun
/sbin/ldconfig || :


%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT


%files -f %{name}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING README
%{_libdir}/*.a
%{_libdir}/*.la
%{_libdir}/*.so
%{_libdir}/libelementary*.so.*
%{_libdir}/edje/modules/elm/

%files devel
%defattr(-, root, root)
%doc doc/html
%{_includedir}/%{name}*/
%{_libdir}/elementary/
%{_libdir}/pkgconfig/*

%files bin
%defattr(-, root, root)
%{_bindir}/*
%{_datadir}/applications/*.desktop
%{_datadir}/elementary/
%{_datadir}/icons/elementary.png


%changelog
