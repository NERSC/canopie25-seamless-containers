#!/bin/bash


rsyslogd

mkdir /run/sshd

if [ ! -e /home/.ubuntu/.ssh ] ; then
   echo "Initializing ssh"
   mkdir /home/ubuntu/.ssh
   ssh-keygen -f default -N ''
   cat default.pub >> /home/ubuntu/.ssh/authorized_keys
   ssh-keygen -f ubuntu24 -N ''
   cat ubuntu24.pub >> /home/ubuntu/.ssh/authorized_keys
   cat ubuntu24.pub |awk '{print $1" "$2"|ubuntu24|ubuntu:24.04"}' >> /lookup
   chown -R ubuntu /home/ubuntu/.ssh/
fi

/usr/sbin/sshd -D
