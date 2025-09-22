FROM ubuntu:24.04 AS build

RUN \
    apt-get -y update && apt-get -y install gcc libpam-dev make

ADD ./src /src

RUN \
    cd /src && make


FROM ubuntu:24.04

RUN apt-get -y update && \
    apt-get -y install openssh-server podman \
		vim docker.io rsyslog

COPY --from=build /src/pam_container.so /tmp/

RUN \
    export ARCH=$(uname -m) && \
    mv /tmp/pam_container.so /usr/lib/$ARCH-linux-gnu/security/

ADD ./image/create_session.sh /bin/create_session.sh

#RUN \
#    useradd -u 1000 -s /bin/bash -m testuser

ADD ./image/sshd /etc/pam.d/sshd

ADD ./image/sshd_config /etc/ssh/sshd_config

ADD ./image/entry.sh /entry.sh
ADD ./image/test.sh /test.sh

RUN \
    rm /etc/update-motd.d/10-help-text \
       /etc/update-motd.d/50-motd-news \
       /etc/update-motd.d/60-unminimize

ADD ./image/motd /etc/update-motd.d/00-header
ADD ./image/ssh_config /root/.ssh/config

EXPOSE 22
ENTRYPOINT ["/entry.sh"]
