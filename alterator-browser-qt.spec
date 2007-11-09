
Name: qmergeinifiles
Version: 0.0.1
Release: alt1

Source: %name-%version.tar

Summary: Utility to merge INI-format files
Group: Development/Other
License: GPL

Requires: libqt4-core >= %{get_version libqt4-core}

BuildRequires(pre): libqt4-core
BuildRequires: gcc-c++ libqt4-devel libstdc++-devel

%description
Utility to merge INI-format files


%prep
%setup -q
%_qt4dir/bin/qmake -spec default

%build
%make_build CUSTOM_OPT_FLAGS="%optflags"

%install
%make INSTALL_ROOT=%buildroot install


%files
%_bindir/*

%changelog
