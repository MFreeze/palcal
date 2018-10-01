Summary: A command line calendar that displays holidays and user-defined events.
Name:       pal
Version: 0.4.3
Release:    1
Copyright:  GPL
Group:	    Applications/Productivity
Url:        http://palcal.sourceforge.net/
Source:     %{name}-%{version}.tgz
BuildRoot:  /var/tmp/%{name}-buildroot

%description
pal is a command line calendar that displays holidays and user-defined
events that are specified in text files.

%define debug_package %{nil}

%prep
%setup -q

%build
cd src
make OPT="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
cd src
make DESTDIR="$RPM_BUILD_ROOT" install-no-rm


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/bin/pal
/usr/share/man/man1/pal.1.gz
/usr/share/doc/pal-%{version}/*
/usr/share/locale/*/*/*
/usr/share/pal
/etc/pal.conf

%changelog
* Fri Mar 19 2004 Scott Kuhl
- Updated list of installed files

* Mon Aug  4 2003 Scott Kuhl
- Created spec file



