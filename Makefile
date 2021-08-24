test:
	hugo server --bind 0.0.0.0 -p 58082 -b http://$(shell ifconfig eth0 | grep -w inet | awk '{print $$2}'):58082

deploy:
	python3 deploy.py

.phony: deploy
