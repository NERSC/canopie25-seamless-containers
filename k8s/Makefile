build:
	docker build -t login image/
	kubectl create ns demo || true
	kubectl apply -f clusterrolebinding.yaml
	kubectl apply -n demo -f serviceaccount.yaml
	kubectl -n demo apply -f login.yaml

test:
	kubectl -n demo exec -it deploy/login -- /test.sh

shell:
	kubectl -n demo exec -it deploy/login -- bash

check:
	kubectl -n demo get all
	kubectl -n demo events

clean:
	kubectl -n demo delete deployment/login || true
	kubectl delete -n user1 deploy/login || true
	kubectl delete -n user1 deploy/ubuntu24 || true
	kubectl delete --wait=false ns user1 || true

cleanall:
	kubectl delete ns demo
	kubectl delete clusterrolebinding/creator-binding

