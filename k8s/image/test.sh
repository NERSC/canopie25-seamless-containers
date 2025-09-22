#!/bin/bash

echo "Get keys"
ssh-keyscan localhost > /etc/ssh/ssh_known_hosts

echo ""
echo "-----------------------"
echo "Testing default for Ubuntu 22.04"
echo 'Expected output: VERSION="22.04.5 LTS (Jammy Jellyfish)"'
ssh default cat /etc/os-release |grep VERSION=
echo "-----------------------"
echo "Testing ubuntu24 for Ubuntu 24.04"
echo 'Expected output: VERSION="24.04.3 LTS (Noble Numbat)"'
ssh ubuntu24 cat /etc/os-release |grep VERSION=
