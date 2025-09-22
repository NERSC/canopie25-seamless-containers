#!/bin/bash

# Grab some things
#echo "$KUBERNETES_SERVICE_HOST" > /etc/kube.host
printenv |grep KUBE|sed 's/^/export /' > /etc/kube.env

# Create user1
useradd user1 -m
ssh-keygen -f /root/.ssh/user1
mkdir /home/user1/.ssh
cat /root/.ssh/user1.pub >> /home/user1/.ssh/authorized_keys
chown -R user1 /home/user1/.ssh
usermod -p $(openssl rand -base64 32) user1

kubectl apply -f /rbac.yaml
/usr/sbin/sshd -D -e
