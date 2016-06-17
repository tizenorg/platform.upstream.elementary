%define dbus_unavailable 1

%bcond_with wayland
%bcond_with x
Name:           elementary
Version:        1.16.0
Release:        0
License:        LGPL-2.1+ and CC-BY-SA-3.0
Summary:        EFL toolkit for small touchscreens
Url:            http://trac.enlightenment.org/e/wiki/Elementary
Group:          Graphics & UI Framework/Development
Source0:        elementary-%{version}.tar.bz2
Source100:      elementary.conf
Source1001:     elementary.manifest
BuildRequires:  doxygen
BuildRequires:  gettext-devel
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-file)
BuildRequires:  pkgconfig(ecore-imf)

%if %{with wayland}
BuildRequires:  pkgconfig(ecore-wayland)
%endif

%if %{with x}
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(x11)
%endif

%if !%{with x}
%if !%{with wayland}
BuildRequires:  pkgconfig(ecore-fb)
%endif
%endif

BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eet)
BuildRequires:  pkgconfig(efreet)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ethumb)
BuildRequires:  pkgconfig(emotion)
BuildRequires:  eet-tools
BuildRequires:  eolian-devel
BuildRequires:  python-devel
BuildRequires:  pkgconfig(ttrace)

#TINEN ONLY(20160425) : Support Mobile target language.
BuildRequires:  pkgconfig(icu-i18n)
#

Recommends:     %{name}-locale = %{version}

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
EFL elementary configuration and test apps package

%package devel
Summary:        Development files for elementary
Group:          Development/Libraries
Requires:       %{name} = %{version}

%description devel
Development components for the elementary package


%prep
%setup -q
cp %{SOURCE1001} .

%build

%if "%{?tizen_profile_name}" == "mobile"
	cp %{_builddir}/%{buildsubdir}/src/lib/mobile/* %{_builddir}/%{buildsubdir}/src/lib
%endif

%autogen --disable-static \
%if %{with wayland}
         --enable-ecore-wayland \
%endif
%if !%{with x}
         --disable-ecore-x \
%endif
         --with-elementary-base-dir="share/.elementary" \
%if %dbus_unavailable
         --disable-build-examples
%else
         --enable-build-examples
%endif

%__make %{?_smp_mflags}

%install
%make_install

mkdir -p %{buildroot}%{_tmpfilesdir}
install -m 0644 %SOURCE100 %{buildroot}%{_tmpfilesdir}/elementary.conf

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
%{_tmpfilesdir}/elementary.conf

%exclude %{_datadir}/elementary/config/

%if ! %dbus_unavailable
%files examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/elementary/examples/*
%endif

%files tools
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/applications/*
%{_bindir}/elementary_config
%{_bindir}/elementary_test*
%{_bindir}/elementary_codegen
%{_bindir}/elm_prefs_cc

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/%{name}-1/*.h*
%{_datadir}/eolian/include/%{name}-1/*.eo
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%{_libdir}/cmake/Elementary/ElementaryConfig.cmake
%{_libdir}/cmake/Elementary/ElementaryConfigVersion.cmake
/usr/share/eolian/include/elementary-*1/*.eot
