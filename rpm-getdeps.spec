# NOTES:
# BUG: Sometimes does not print last BR: see gnuchess.spec d19b5d9
# Consider using rpmspec tool instead
Summary:	Get dependencies out of RPM spec file
Summary(pl.UTF-8):	Pobieranie zależności z pliku spec pakietu RPM
Name:		rpm-getdeps
Version:	0.0.8
Release:	1
License:	GPL
Group:		Applications/System
Source0:	getdeps.c
Source1:	Makefile
BuildRequires:	rpm-devel >= 1:4.16.0
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Get dependencies out of RPM spec file.

%description -l pl.UTF-8
Pobieranie zależności z pliku spec pakietu RPM.

%prep
%setup -q -c -T
ln -s %{SOURCE0} .
ln -s %{SOURCE1} .

%build
echo %{__make} \
	CC="%{__cc}" \
	RPMLDFLAGS="%{rpmldflags}" \
	RPMCFLAGS="%{rpmcflags}"

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir}

cp -p rpm-getdeps $RPM_BUILD_ROOT%{_bindir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/rpm-getdeps
