FROM docker.io/library/debian:12 AS build

RUN apt-get update \
  && apt-get install -y \
    build-essential pkg-config curl \
    libsfml-dev libbox2d-dev \
  && apt-get clean

WORKDIR /build

ADD Makefile ./

RUN make deps

ADD src/ ./src

RUN make -j"$(nproc)"


FROM docker.io/library/debian:12

RUN apt-get update \
  && apt-get install -y \
    libsfml-graphics2.5 libbox2d2 \
  && apt-get clean

WORKDIR /app

COPY --from=build /build/game ./game

ENTRYPOINT ["./game"]
