# PAM Proof-of-Concept

This directory contains a proof-of-concept for the PAM model described in the paper.
This requires docker with system privileges.  The example will create a Docker container
with the PAM PoC and start an sshd.  It creates two environments: a default that uses Ubunbtu 22.04 and another with Ubuntu 24.04.  It maps a specific ssh key to the Ubuntu 24.04 environment.
The PoC uses a very simply lookup file.  A production version would use some service to define and manage these mappings.  

The tests run by exec'ing into the pam_test container and doing the ssh connection inside this container to localhost. The sshd is also exposed on port 2222.  Test can be done this way as well but this requires manually creating a ssh key and adding the public key to the authorized_keys file in the ubuntu home directory.

## Directions

Build and start the service and run a test.
```
make
make test
```

Start an interactive session
```
make debug
ssh default
cat /etc/os-release
exit
ssh ubuntu24
cat /etc/os-release
exit
```

Clean up
```
make clean
```
