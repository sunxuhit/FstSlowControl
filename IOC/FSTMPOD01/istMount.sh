#!/bin/sh
#######ist01.daq.bnl.local automounter#####
echo "This script will automount ist01.daq.bnl.local via NFS and SSHFS"
echo "If you have any trouble with this script, please contact the STAR S&C computer support team."
echo "E-mail: starsupport@bnl.gov"


echo "Mounting ist01 at /istmount"
sshfs operator@daqman.starp.bnl.gov:/net/ist01/RTScache/data /istmount
