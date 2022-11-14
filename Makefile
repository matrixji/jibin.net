test:
	hugo server --disableFastRender --bind 0.0.0.0 -p 58082 -b http://localhost:58082

build:
	rm -fr public
	hugo --minify
	cp -fr rootfs/* public/

deploy: build
	rsync -avpz --delete public/ root@jibin.net:/opt/jibin.net

.phony: deploy build
