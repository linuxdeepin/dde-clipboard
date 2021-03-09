# Run tests in check section
# disable for bootstrapping

%bcond_with check

%global with_debug 1
%if 0%{?with_debug}
%global debug_package   %{nil}
%endif

Name:           dde-clipboard
Version:        5.3.0.3
Release:        1
Summary:        Qt platform theme integration plugins for DDE
License:        GPLv3+
URL:            http://shuttle.corp.deepin.com/cache/repos/eagle/release-candidate/RERFNS4wLjAuNjU3NQ/pool/main/d/dde-clipboard/
Source0:        %{name}_%{version}.orig.tar.xz

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-linguist
BuildRequires:  dtkwidget-devel
BuildRequires:  dtkwidget
BuildRequires:  libgio-qt
BuildRequires:  libgio-qt-devel
BuildRequires:  pkgconfig(dframeworkdbus) >= 2.0

%description
Qt platform theme integration plugins for DDE
 Multiple Qt plugins to provide better Qt5 integration for DDE is included.


%prep
%autosetup

%build
export PATH=$PATH:/usr/lib64/qt5/bin
mkdir build && cd build
%{_libdir}/qt5/bin/qmake ..
%{__make}

%install
pushd %{_builddir}/%{name}-%{version}/build
%make_install INSTALL_ROOT=%{buildroot}
popd

%files
/etc/xdg/autostart/dde-clipboard.desktop
%{_bindir}/%{name}
%{_bindir}/%{name}loader
%{_datadir}/dbus-1/services/com.deepin.dde.Clipboard.service
%{_datadir}/%{name}/translations/

%changelog
* Wed Jun 10 2020 uoser <uoser@uniontech.com> - 5.3.0.3
- Update to 5.3.0.3

