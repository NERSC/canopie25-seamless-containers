#!/bin/bash
echo ""
echo "-----------------------"
echo "Using the default env"
echo 'Expected output: VERSION="22.04.5 LTS (Jammy Jellyfish)"'
ssh -o StrictHostKeyChecking=accept-new -i default ubuntu@localhost cat /etc/os-release | grep "VERSION="

echo "-----------------------"
echo "Using the ubuntu24 env"
echo 'Expected output: VERSION="24.04.3 LTS (Noble Numbat)"'
ssh -i ubuntu24 ubuntu@localhost cat /etc/os-release | grep "VERSION="
