FROM golang:latest

ENV LANG=en_US.utf-8
ENV PATH=/root/.local/bin:${PATH}

RUN apt update
RUN apt install -y rsync
RUN go env -w GO111MODULE=on
RUN go env -w GOPROXY=https://goproxy.cn,direct
RUN CGO_ENABLED=1 go install --tags extended github.com/gohugoio/hugo@latest

EXPOSE 58082