#!/bin/sh
cd /
touch /dev/.coldboot_done

export LD_LIBRARY_PATH=

# Save systemd notify socket name to let droid-init-done.sh pick it up later
echo $NOTIFY_SOCKET > /run/droid-hal/notify-socket-name

# Use exec nohup since systemd may send SIGHUP, but droid-hal-init doesn't
# handle it. This avoids having to modify android_system_core, which would
# require different handling for every different android version.
# exec nohup /sbin/droid-hal-init

# breaks LXC if mounted
if [ -d /sys/fs/cgroup/schedtune ]; then
    umount -l /sys/fs/cgroup/schedtune || true
fi

mkdir -p /dev/__properties__
mkdir -p /dev/socket

# mount binderfs if needed
if [ ! -e /dev/binder ]; then
    mkdir -p /dev/binderfs
    mount -t binder binder /dev/binderfs -o stats=global
    ln -s /dev/binderfs/*binder /dev
fi

# Some special handling for /android/apex
if [ -d /android/apex ]; then
    echo "Handling /android/apex bind-mounts"

    mount -t tmpfs android_apex /android/apex
    for apex in "com.android.runtime" "com.android.art" "com.android.i18n"; do
        target_path="/android/apex/${apex}"

        for suffix in ".release" ".debug" ""; do # No suffix is valid too
            source_path="/android/system/apex/${apex}${suffix}"
            if [ -e "$source_path" ]; then
                mkdir -p $target_path
                mount -o bind $source_path $target_path
                break
            fi
        done
    done
fi

# Bind mount any files in /usr/share/halium-overlay/ over the android system
OVERLAYDIR=/usr/share/halium-overlay/
for f in $(find $OVERLAYDIR -type f) $(find $OVERLAYDIR -type l); do
    ANDROID_POINT=${f#"$OVERLAYDIR"}
    echo mounting $f to $ANDROID_POINT;
    mount -o bind $f /android/$ANDROID_POINT
done

if [ -f /usr/bin/droid/halium-setup-local.sh ]; then
    /bin/sh /usr/bin/droid/halium-setup-local.sh
fi

lxc-start -n android

exit 0
