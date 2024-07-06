FROM amd64/alpine:3.20.1

RUN apk add \
    samurai \
    ninja-build \
    autoconf \
    automake \
    gcc \
    g++ \
    cmake \
    git \
    libtool \
    libunwind-dev \
    gtest-dev \
    benchmark-dev \
    sqlite-dev \
    libpq-dev \
    mariadb-connector-c-dev \
    asio-dev \
    openssl-dev \
    libarchive-dev \
    python3 \
    py3-pip

COPY ./requirements.txt /tmp/requirements.txt

RUN pip install -r /tmp/requirements.txt --break-system-packages && \
    pip cache purge
