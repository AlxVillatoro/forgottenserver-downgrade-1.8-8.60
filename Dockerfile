FROM ubuntu:22.04 AS build

ENV DEBIAN_FRONTEND=noninteractive \
    VCPKG_ROOT=/opt/vcpkg \
    VCPKG_DEFAULT_TRIPLET=x64-linux
ENV PATH="${VCPKG_ROOT}:${PATH}"

RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf \
    automake \
    bison \
    build-essential \
    ca-certificates \
    curl \
    flex \
    git \
    libtool \
    ninja-build \
    pkg-config \
    python3 \
    tar \
    unzip \
    zip \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt
RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git \
    && ${VCPKG_ROOT}/bootstrap-vcpkg.sh

WORKDIR /usr/src/forgottenserver-downgrade

# Copy the vcpkg manifest first so dependency installation can be cached.
COPY vcpkg.json ./
RUN vcpkg install --triplet ${VCPKG_DEFAULT_TRIPLET}

COPY cmake ./cmake
COPY src ./src
COPY CMakeLists.txt ./

RUN cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=${VCPKG_DEFAULT_TRIPLET} \
      -DENABLE_NATIVE_OPTIMIZATIONS=OFF \
      -DSKIP_GIT=ON \
    && cmake --build build --config RelWithDebInfo --target tfs --parallel

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libgcc-s1 \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/src/forgottenserver-downgrade/build/tfs /bin/tfs
COPY data /srv/data/
COPY LICENSE README.md *.dist *.sql key.pem /srv/

EXPOSE 7171 7172
WORKDIR /srv
VOLUME /srv
ENTRYPOINT ["/bin/tfs"]
