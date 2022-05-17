Name:	    droid-config-halium
Provides:   droid-hal
Provides:   droid-config
Summary:    Config packages for
Version:    1
%if 0%{?_obs_build_project:1}
Release:    1
%else
%define rel_date %(date +'%%Y%%m%%d%%H%%M')
Release: %{rel_date}
%endif
License:    GPLv2
Source0:    %{name}-%{version}.tar.bz2

%description
%{summary}.

%prep


%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
rm -rf tmp/
mkdir -p tmp/
echo "%defattr(-,root,root,-)" > tmp/droid-config.files

copy_files_from() {
  config_dir=$1
  if [ -d $config_dir ]; then
    olddir=$PWD
    cd $config_dir
    for f in $(find . \( -type f -o -type l \) -print); do
      dst=$(echo $f | sed 's/^.//')
      if echo $dst | grep -qE "^/usr/lib/|^/lib/"; then
        move_to_lib64=true
        for stay_in_lib in "${do_not_move_to_lib64[@]}"; do
          if echo $dst | grep -qE "^/usr/lib/$stay_in_lib/|^/lib/$stay_in_lib/"; then
            move_to_lib64=false
            break
          fi
        done
        if [ "$move_to_lib64" = true ]; then
          if echo $dst | grep -q "^/usr/lib/"; then
            dst=$(echo $dst | sed 's /usr/lib/ %{_libdir}/ ')
          else
            dst=$(echo $dst | sed 's /lib/ /%{_lib}/ ')
          fi
        fi
      fi
      dstdir=$RPM_BUILD_ROOT$(dirname $dst)
      if [ ! -d $dstdir ]; then
        mkdir -p $dstdir
      fi
      cp -Pv $f $dstdir
      echo $dst >> $olddir/tmp/droid-config.files
    done
    cd - >/dev/null
  fi
}

copy_files_from sparse


%files -f tmp/droid-config.files
%defattr(-,root,root,-)
