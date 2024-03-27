Name:	    droid-config-halium-%{flavour}
Provides:   droid-hal
Provides:   droid-config
Provides:   droid-config-halium
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
AutoReqProv: no

#Hack
Provides:    droid-hal-kernel
Provides:    droid-hal-img-boot
Provides:    droid-hal-img-recovery

%description
%{summary}.

%prep
%if 0%{?_obs_build_project:1}
# For OBS builds we need to have tarball extracted after tar_git packaging it
%setup -q -n %{name}-%{version}
%endif

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
rm -rf tmp/
mkdir -p tmp/
echo "%defattr(-,root,root,-)" > tmp/droid-config.files

do_not_move_to_lib64=(
    "firmware"
    "modules"
    "oneshot.d"
    "startup"
    "sysctl.d"
    "systemd/system"
    "systemd/user"
    "sysusers.d"
    "tmpfiles.d"
    "udev"
    "droid/hw-group.d"
    )

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

copy_files_from ./sparse

mkdir -p %{buildroot}/var/lib/lxc/android/rootfs

mkdir -p $RPM_BUILD_ROOT/%{_datadir}/ssu/features.d/

sed -e "s|@FLAVOUR@|%{flavour}|g" \
%if "%{flavour}" != "testing"
    -e "s|:/%%(release)/sailfish_%%(release)_%%(arch)|/sailfish_latest_%%(arch)|g" \
%endif
    adaptation-halium.ini.in > $RPM_BUILD_ROOT/%{_datadir}/ssu/features.d/adaptation-halum.ini

%post
# Create Android-specific users based on droid-hal-device.inc users subpackage

# NOTE: We do not delete groups/users in any case since that would cause issues
# with users being removed from groups where they should really be in.

echo creating droid users and groups
[ -e %{_prefix}/lib/droid ] && cat %{_prefix}/lib/droid | while read name id; do
    if ! getent group $name; then
        if getent group $id; then
            other_name=$(getent group $id 2>/dev/null |cut -d":" -f1)
            echo "Group $name did not exist yet, but another group has the same id ($id, $other_name), renaming that group"
            groupmod -g $id -n $name $other_name || :
        else
            echo "Group $name did not exist yet"
            groupadd -g $id $name || :
        fi
    else
        echo "Group $name already existed, modifying it"
        groupmod -g $id $name || :
    fi
    if ! getent passwd $name; then
        if getent passwd $id; then
            other_name=$(getent passwd $id 2>/dev/null |cut -d":" -f1)
            echo "User $name did not exist yet, but another user has the same id ($id, $other_name), renaming that user"
            usermod -s /sbin/nologin -d / -u $id -g $id -l $name $other_name || :
        else
            echo "User $name did not exist yet"
            useradd -M -N -s /sbin/nologin -d / -u $id -g $id $name || ::
        fi
    else
        echo "User $name already existed, modifying it"
        usermod -s /sbin/nologin -d / -u $id -g $id $name || :
    fi
done

%files -f tmp/droid-config.files
%defattr(-,root,root,-)
/var/lib/lxc/android/rootfs
%{_datadir}/ssu/features.d/adaptation-halum.ini

