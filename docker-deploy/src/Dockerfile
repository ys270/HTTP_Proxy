FROM ubuntu:18.04
RUN apt-get update && apt-get install -y g++
RUN mkdir /main
RUN mkdir /var/log/erss
COPY . /main/
WORKDIR /main
