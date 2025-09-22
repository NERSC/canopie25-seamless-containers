FROM ubuntu:24.04


RUN \
    apt-get -y update && \
    apt-get -y install openssh-server python3


RUN \
    apt-get install -y apt-transport-https ca-certificates curl gnupg && \
    curl -fsSL https://pkgs.k8s.io/core:/stable:/v1.32/deb/Release.key | gpg --dearmor -o /etc/apt/keyrings/kubernetes-apt-keyring.gpg && \
    chmod 644 /etc/apt/keyrings/kubernetes-apt-keyring.gpg && \
    echo 'deb [signed-by=/etc/apt/keyrings/kubernetes-apt-keyring.gpg] https://pkgs.k8s.io/core:/stable:/v1.32/deb/ /' > \
        /etc/apt/sources.list.d/kubernetes.list && \
    chmod 644 /etc/apt/sources.list.d/kubernetes.list && \
    apt-get -y update && apt-get -y install kubectl

RUN \
    apt-get -y install libnss-ldapd vim \
        python3-requests python3-ldap3 tini sudo

RUN mkdir /run/sshd

COPY connect /usr/bin/connect

COPY entry.sh /entry.sh
COPY test.sh /test.sh

COPY create /create

COPY sshd_config /etc/ssh/sshd_config
COPY ssh_config /root/.ssh/config

COPY demo.sudo /etc/sudoers.d/demo

COPY rbac.yaml /rbac.yaml
COPY default.yaml /default.yaml
COPY ubuntu24.yaml /ubuntu24.yaml

ENTRYPOINT ["tini"]
CMD ["/entry.sh"]
