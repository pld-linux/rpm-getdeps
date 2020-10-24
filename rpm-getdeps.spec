Summary:	Get dependencies out of RPM spec file
Summary(pl):	Pobieranie zależności z pliku spec pakietu RPM
Name:		rpm-getdeps
Version:	0.0.7
Release:	2
License:	GPL
Group:		Applications/System
Source0:	getdeps.c
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
%{__cc} %{rpmcflags} %{rpmldflags} -I/usr/include/rpm -Wall -lrpm -lrpmbuild %{SOURCE0} -o rpm-getdeps

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir}

cp -p rpm-getdeps $RPM_BUILD_ROOT%{_bindir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/rpm-getdeps
