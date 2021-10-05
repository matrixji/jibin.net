test:
	hugo server --bind 0.0.0.0 -p 58082 -b http://localhost:58082

deploy:
	rm -fr public
	hugo -t mj --minify
	cp -fr rootfs/* public/
	rsync -avpz --delete public/ root@jibin.net:/opt/jibin.net

.phony: deploy
