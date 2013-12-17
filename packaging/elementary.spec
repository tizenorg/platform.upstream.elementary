%bcond_with wayland
%bcond_with x
Name:           elementary
Version:        1.7.8
Release:        2 
License:        LGPL-2.1+
Summary:        EFL toolkit for small touchscreens
Url:            http://trac.enlightenment.org/e/wiki/Elementary
Group:          Graphics/EFL
Source0:        elementary-%{version}.tar.bz2
Source1001: 	elementary.manifest
BuildRequires:  doxygen
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-fb)
BuildRequires:  pkgconfig(ecore-file)
BuildRequires:  pkgconfig(ecore-imf)
%if %{with x}
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(x11)
%endif
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eet)
BuildRequires:  pkgconfig(efreet)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)

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

%package tools
Summary:   EFL elementary configuration and test apps

%description tools
EFL elementary configuration and test apps

%package devel
Summary:        Development components for the elementary package
Group:          Development/Libraries
Requires:       %{name} = %{version}

%description devel
Development files for elementary

%prep
%setup -q
cp %{SOURCE1001} .

%build

%autogen --disable-static \
%if %{with wayland}
         --enable-ecore-wayland \
%endif
%if !%{with x}
         --disable-ecore-x \
%endif
         --enable-build-examples

make %{?_smp_mflags}

%install
%make_install

%find_lang %{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%lang_package 

%files 
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_bindir}/elementary_quicklaunch
%{_bindir}/elementary_run
%{_libdir}/edje/*
%{_libdir}/elementary/modules/*
%{_libdir}/*.so.*
%{_datadir}/elementary/*
%{_datadir}/icons/elementary.png

%files examples
%defattr(-,root,root,-)
%{_libdir}/elementary/examples/*

%files tools
%defattr(-,root,root,-)
%{_datadir}/applications/*
%{_bindir}/elementary_config
%{_bindir}/elementary_test*

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/elementary-1/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc

%changelog
