FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git libpq-dev

WORKDIR /app
COPY . .
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --config Release

EXPOSE 8080
CMD ["./build/api-service"]