# Guidance on building a Halium based SailfishOS image

## Introduction
The guide here documents the outline steps for building a 'Halium' (https://halium.org/) based SailfishOS image.
Halium provides a common Android system image on which to run a hybris based Linux distribution, removing the need
for a porter to patch and create the modified system image.

The full process is not documented here, but and outline of the main steps and links to example repositories which
can be used as a basis are.

You should be familiar with the Saifish HADK https://docs.sailfishos.org/Develop/HADK/

## Main Requirements

### Kernel Package
An example kernel package is at https://github.com/HelloVolla/kernel-adaptation-halium-algiz
Main points to note:
* linux submodule contains the kernel code
* rpm/ folder contains the package .spec
* kernel-adaptation-simplified/ contains the build/package scripts
  and should be modified to produce a working boot.img for the SoC
* patches/ contains patch files that will be applied
* In the root folder, you may include a config that will be used and an initrd image to boot with
* build.sh Is a helper script for performing a local build using a given android SDK.
  It is also possible to build on OBS using the SailfishOS compiler chain for some devices

### Device Specific Configuration
An example configuration package is at https://github.com/HelloVolla/droid-config-halium-algiz
The config package contains all custom device configuration/files/services.  The Sailfish HADK contains
good documentation on this.

Halium specific changes typically inlcude adding files to sparse/usr/share/halium-overlay which will be
mounted over the top of the android system/ and vendor/ partitions, allowing files on the system to be modified.

Fo image building, you also need to create the image build kickstart files in sparse/usr/share/kickstarts/
which will be used later in a github action.  The kickstart files should reference the device and halium OBS
repositories

If using the generic halium startup scripts, the .spec file should
Require: droid-config-halium

### Common Halium Configuration
The generic droid-config-halium package contains halium port specific scripts which are shared across devices.
These scripts start the halium android container.

This pacakge exists tt https://github.com/sailfish-on-fxtecpro1/droid-config-halium and is built/packaged at
https://build.sailfishos.org/project/show/nemo:devel:hw:halium with subprojects for the halium version.

### Image Build Script
An example build repository is at https://github.com/HelloVolla/sailfish-release-halium-algiz

The build repository runs a github action to build the SailfishOS image, using packages from the generic, device and
halium OBS repositories.

The action is defined in .github/build.yml and runs the create-image.sh script.

The create-image.sh script performs the follwoing actions:
* Installs some required packages
* Downloads the device configuration package and extracts the kickstart file
* Create the filesystem image
* Performs any supplementary changes such as createing a 'super' image

The scripts use specially formed tags on the repository to name the build.  The convention is:
* release-<VERSION>-<RELEASE>-<EXTRA>
Where
* VERSION is devel or testing
* RELEASE is the SailfishOS release number
* EXTRA is the developer build version
 
