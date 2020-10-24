# $Revision: 1.4 $, $Date: 2005/09/12 16:46:06 $
Summary:	Get dependencies out of RPM spec file
Summary(pl):	Pobieranie zależności z pliku spec pakietu RPM
Name:		rpm-getdeps
Version:	0.0.7
Release:	2
License:	GPL
Vendor:		Joey Hess <joey@kitenet.net>
Group:		Applications/System
Source0:	http://www-user.tu-chemnitz.de/~ensc/getdeps.c
# Source0-md5:	c20a7f6a0ef86461514fbf55092ae434
BuildRequires:	rpm-devel
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Get dependencies out of RPM spec file.

%description -l pl
Pobieranie zależności z pliku spec pakietu RPM.

%prep
%setup -q -c -T

%build
%{__cc} %{rpmcflags} %{rpmldflags} -I/usr/include/rpm -Wall -lrpm -lrpmbuild %{SOURCE0} -o %{name}

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir}

install %{name} $RPM_BUILD_ROOT%{_bindir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/*

%define date	%(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* %{date} PLD Team <feedback@pld-linux.org>
All persons listed below can be reached at <cvs_login>@pld-linux.org

$Log: rpm-getdeps.spec,v $
Revision 1.4  2005/09/12 16:46:06  glen
- integer release, 2, STBR

Revision 1.3  2004/06/17 19:36:58  arekm
- rebuild

Revision 1.2  2004/06/06 15:33:58  qboosh
- pl

Revision 1.1  2004/06/06 09:56:47  arekm
- initial pld release
