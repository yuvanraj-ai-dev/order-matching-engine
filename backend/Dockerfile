# ---- Build stage -----------------------------------------------------------
FROM ubuntu:24.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

# libdrogon-dev is available in Ubuntu 24.04's "universe" repo. Most Docker
# base images already have universe enabled; the add-apt-repository call is
# a no-op safety net in case a slimmer base image doesn't.
RUN apt-get update && apt-get install -y --no-install-recommends \
        software-properties-common \
    && add-apt-repository universe \
    && apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        libdrogon-dev \
        libjsoncpp-dev \
        libssl-dev \
        libpq-dev \
        libhiredis-dev \
        zlib1g-dev \
        uuid-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY engine/ ./engine/
COPY backend/ ./backend/

WORKDIR /app/backend
RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j"$(nproc)"

# ---- Runtime stage -----------------------------------------------------------
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        libdrogon1t64 \
        libpq5 \
        libhiredis1.1.0 \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    || (apt-get update && apt-get install -y --no-install-recommends \
        libdrogon1 libpq5 libhiredis0.14 ca-certificates \
        && rm -rf /var/lib/apt/lists/*)
# ^ package suffix (libdrogon1t64 vs libdrogon1, hiredis version) varies by
#   Ubuntu point release; the fallback branch covers older naming.

WORKDIR /app
COPY --from=build /app/backend/build/order_matching_backend ./order_matching_backend
COPY --from=build /app/backend/config.json ./config.json

# Render (and most PaaS hosts) set $PORT; main.cc reads it at startup.
EXPOSE 8080
CMD ["./order_matching_backend"]
