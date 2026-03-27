FROM ubuntu:24.04

# setting non-interactive means 
# “don’t stop and ask me questions in the terminal” during install
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
