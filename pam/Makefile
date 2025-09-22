
build:
	docker build -t pam-test . 


test:
	docker run -d --name pam_test -p 2222:22 --privileged -v /run/docker.sock:/run/docker.sock -v pamhome:/home/ubuntu --pid host pam-test
	sleep 1
	docker exec -it pam_test ./test.sh
#	docker run -d --name pam_test -p 2222:22 --privileged -v /run/docker.sock:/run/docker.sock -v `pwd`/home:/home --pid host pam-test

debug:
	docker exec -it pam_test bash

clean:
	docker rm -f ubuntu-default
	docker rm -f ubuntu-ubuntu24
	docker rm -f pam_test

logs:
	docker logs pam_test
	docker exec -t pam_test cat /var/log/auth.log

all: clean build test
