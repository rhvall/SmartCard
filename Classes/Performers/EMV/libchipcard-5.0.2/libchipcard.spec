%define name libchipcard
%define version 5.0.2
%define rpm_cxxflags \"-O2 -march=i486 -mcpu=i586\"
%define rpm_cflags \"-O2 -march=i486 -mcpu=i586\"

%define dist    
%define disttag 
%define distver 


# Note: There is not yet a special symbol for the rpm release
# version. We set it to one here.
%define release 1.%{disttag}%{distver}

Summary: A library for easy access to smart cards (chipcards).
Name: %{name}
Version: %{version}
Release: %{release}
Source: http://download.sourceforge.net/libchipcard/%{name}-%{version}.tar.gz
Requires: gwenhywfar
Group: Libraries/System
License: LGPL
Packager: Martin Preuss <martin@libchipcard.de>
URL: http://www.libchipcard.de
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prereq: /sbin/ldconfig
Prefix: %{_prefix}
BuildRequires: gwenhywfar-devel >= 2.0.0

%description
Libchipcard allows easy access to smart cards. It provides basic access
to memory and processor cards and has special support for German medical
cards, German "GeldKarte" and HBCI (homebanking) cards (both type 0 and 
type 1).
It accesses the readers via CTAPI or IFD interfaces and has successfully
been tested with Towitoko, Kobil, SCM, Orga, Omnikey and Reiner-SCT readers.
This package contains the chipcard-daemon needed to access card readers.

%package devel
Summary: LibChipCard server development kit
Group: Development/Libraries
%description devel
This package contains chipcard-config and header files for writing 
drivers, services or even your own chipcard daemon for LibChipCard.

%package crypttoken
Summary: CryptToken Plugins for Gwenhywfar
Group: Development/Libraries
%description crypttoken
This package contains the CryptToken plugins for Gwenhywfar. These are used
by AqBanking to access chipcards for the German homebanking protocol HBCI.

%prep
%setup -q

%build
%{configure}
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
LIBRARY_PATH=$RPM_BUILD_ROOT%{_libdir} make DESTDIR=$RPM_BUILD_ROOT \
  install

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig
/sbin/chkconfig --add chipcardd

%preun
/sbin/chkconfig --del chipcardd

%postun
/sbin/ldconfig


%files
%defattr(-,root,root,0755)
%doc README COPYING ChangeLog
%{_libdir}/libchipcardc.so
%{_libdir}/libchipcardc.so.*
%{_sysconfdir}/chipcard
%{_bindir}/cardcommander
%{_bindir}/chipcard-tool
%{_bindir}/geldkarte
%{_bindir}/kvkcard
%{_bindir}/memcard
%{_datadir}/chipcard

%files devel
%defattr(-,root,root,0755)
%doc README COPYING ChangeLog
%{_bindir}/chipcard-config
%{_includedir}/*
%{_libdir}/*.*a
%{_datadir}/aclocal/chipcard.m4

%files crypttoken
%defattr(-,root,root,0755)
%{_libdir}/gwenhywfar/plugins

%changelog

