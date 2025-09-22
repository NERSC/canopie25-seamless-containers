# Kubernetes Login Proof-of-Concept

This directory contains a proof-of-concept for the kubernetes based model described in the paper.
This requires a minimal kubernetes cluster.  The example will create a mock login
container and demonstrate starting up two login environments for a test user.
The proof-of-concept uses a simple script that is run via sudo.  A more functional version
could use a separate API service to handle initializing the namespaces and pods.

The tests are run by exec'ing into the login container and doing the ssh connection inside this container to localhost.

## Directions

Build and start the service and run a test.
```
make build
make test
```

Start an interactive session
```
make shell
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
make cleanall
```

## Contents

| File | Description |
|---|---|
|Makefile|Makefile|
|image| Directory for Container image|
|image/entry.sh|Entrypoint script.  This initializes the environment and starts sshd|
|image/Dockerfile|Dockerfile to build the image|
|image/sshd_config|sshd config that configures ForceCommand|
|image/demo.sudo|Sudo config|
|image/ssh_config|ssh client config to simplify testing|
|image/ubuntu24.yaml|Ubuntu24 Login definition|
|image/rbac.yaml|YAML to create the namespace and service accounts.  Hardcoded for user1|
|image/connect|The script that will be run by the ForceCommand|
|image/create|The script run via sudo to initialize the namespace for the user, create a token, and ensure the requested environment is started |
|image/test.sh|Test script to show things are working|
|image/default.yaml|THe default ubuntu 22.04 deployment|
|serviceaccount.yaml|Creator service account definition for the login service|
|login.yaml|Deployment definition for the login pod|
|clusterrolebinding.yaml|Cluster Role Binding for the creator service account|
