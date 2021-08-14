test:
	hugo server --bind 0.0.0.0 -p 58082 -b http://home.jibin.net:58082

deploy:
	python3 deploy.py

.phony: deploy
