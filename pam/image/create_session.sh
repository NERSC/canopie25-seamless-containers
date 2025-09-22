#!/bin/bash

LOOKUP=/lookup
DEFAULT_IMAGE=ubuntu:22.04
#BASE=/host_mnt/Users/canon/Dev/nersc/pam_nce/homes/
BASE=pamhome

USER=$1

KEY=$(echo $@|sed 's/.*publickey //')

SESS_NAME=$(grep "$KEY" $LOOKUP|cut -d\| -f 2)
export IMAGE=$(grep "$KEY" $LOOKUP|cut -d\| -f 3)
export SESS

if [ -z "$SESS_NAME" ] ; then
    SESS=${USER}-default
    IMAGE="$DEFAULT_IMAGE"
else
    SESS=${USER}-${SESS_NAME}
fi

printenv > /tmp/debug

docker inspect $SESS > /dev/null 2>&1 || \
	docker run -d --name $SESS \
		-v $BASE:/home/${USER} \
		$IMAGE sleep infinity > /dev/null

docker exec $SESS useradd -u 1000 -s /bin/bash $USER

PID=$(docker inspect $SESS|grep Pid|head -1|sed 's/.*://'|sed 's/,//')
echo $PID
