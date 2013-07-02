Name:           elementary
Version:        1.7.7
Release:        2 
License:        LGPL-2.1+
Summary:        EFL toolkit for small touchscreens
Url:            http://trac.enlightenment.org/e/wiki/Elementary
Group:          Graphics/EFL
Source0:        elementary-%{version}.tar.bz2
BuildRequires:  doxygen
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-fb)
BuildRequires:  pkgconfig(ecore-file)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eet)
BuildRequires:  pkgconfig(efreet)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(x11)
BuildRequires:  eet-tools
BuildRequires:  python-devel

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

%package examples
Summary:   EFL elementary examples

%description examples
EFL elementary examples

%package devel
Summary:        Development components for the elementary package
Group:          Development/Libraries
Requires:       %{name} = %{version}

%description devel
Development files for elementary

%prep
%setup -q

%build

%autogen --disable-static --enable-build-examples
make %{?_smp_mflags}

%install
%make_install

%find_lang %{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%lang_package 

%files 
%defattr(-,root,root,-)
%license COPYING
%{_bindir}/*
%{_libdir}/edje/*
%{_libdir}/elementary/*
%{_libdir}/*.so.*
%{_datadir}/applications/*
%{_datadir}/elementary/*
%{_datadir}/icons/elementary.png

%files examples
%defattr(-,root,root,-)
%{_datadir}/elementary/examples/*

%files devel
%defattr(-,root,root,-)
%{_includedir}/elementary-1/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc

%changelog
