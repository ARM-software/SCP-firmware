#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

FROM ubuntu:20.04 as ci

ARG ARM_GNU_RM_URL="https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2"
ARG ARM_COMPILER_6_URL="https://developer.arm.com/-/media/Files/downloads/compiler/DS500-BN-00026-r5p0-16rel1.tgz"
ARG DOXYGEN_URL="https://sourceforge.net/projects/doxygen/files/rel-1.8.20/doxygen-1.8.20.linux.bin.tar.gz"
ARG CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v3.18.3/cmake-3.18.3-Linux-x86_64.tar.gz"
ARG AARCH64_GCC_URL="https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf.tar.xz"
ARG CPPCHECK_SRC_URL="git://github.com/danmar/cppcheck.git"
ARG CPPCHECK_CHECKOUT_TAG="1.90"
ARG IWYU_SRC_URL="https://github.com/include-what-you-use/include-what-you-use.git"


ENV ARMLMD_LICENSE_FILE=

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y --no-install-recommends \
        bundler gcc g++ gpg-agent git gosu lsb-release make ninja-build \
        xz-utils python3 python3-pip software-properties-common wget && \
    wget -nv -O - -c "https://apt.llvm.org/llvm.sh" | bash && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        clang-format libclang-cpp9 libclang1-9 \
        clang-tidy-10 libclang-9-dev \
        clang-10 llvm-10 libclang-10-dev \
        llvm-10-dev && \
    python3 -m pip install --upgrade pip

ENV DEBIAN_FRONTEND=

RUN mkdir "/opt/arm-gnu-rm" && \
    wget -nv -O - -c "${ARM_GNU_RM_URL}" | \
    tar -jxf - -C "/opt/arm-gnu-rm" --strip-components=1 && \
    echo 'export PATH=/opt/arm-gnu-rm/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/arm-gnu-rm/bin:${PATH}"

RUN mkdir "/tmp/arm-compiler-6" && \
    wget -nv -O - -c "${ARM_COMPILER_6_URL}" | \
    tar -zxf - -C "/tmp/arm-compiler-6" && \
    sh "/tmp/arm-compiler-6/install_x86_64.sh" -d "/opt/arm-compiler-6" \
        --no-interactive --i-agree-to-the-contained-eula && \
    rm -rf "/tmp/arm-compiler-6" && \
    echo 'export PATH=/opt/arm-compiler-6/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/arm-compiler-6/bin:${PATH}"

RUN mkdir "/opt/doxygen" && \
    wget -nv -O - -c "${DOXYGEN_URL}" | \
    tar -zxf - -C "/opt/doxygen" --strip-components=1 && \
    echo 'export PATH=/opt/doxygen/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/doxygen/bin:${PATH}"

RUN mkdir "/opt/cmake" && \
    wget -nv -O - -c "${CMAKE_URL}" | \
    tar -zxf - -C "/opt/cmake" --strip-components=1 && \
    echo 'export PATH=/opt/cmake/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/cmake/bin:${PATH}"

RUN mkdir "/opt/aarch64-gcc" && \
    wget -nv -O - -c "${AARCH64_GCC_URL}" | \
    tar -Jxf - -C "/opt/aarch64-gcc" --strip-components=1 && \
    echo 'export PATH=/opt/aarch64-gcc/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/aarch64-gcc/bin:${PATH}"

RUN cwd=$PWD && mkdir "/opt/cppcheck" && cd "/opt/cppcheck" && \
    git clone --depth 1 --branch "${CPPCHECK_CHECKOUT_TAG}" \
    "${CPPCHECK_SRC_URL}" source && \
    cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=/opt/cppcheck \
        -DFILESDIR=/opt/cppcheck ./source && \
    cmake --build . -- install && cd $cwd && \
    echo 'export PATH=/opt/cppcheck/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/cppcheck/bin:${PATH}"

RUN cwd=$PWD && mkdir "/opt/iwyu" && cd "/opt/iwyu" && \
    git clone "${IWYU_SRC_URL}" -b clang_10 --single-branch iwyu-10 && \
    cmake -G "Ninja" -DCMAKE_PREFIX_PATH=/usr/lib/llvm-10 \
    ./iwyu-10 && cmake --build . && \
    cd bin && ln -s include-what-you-use iwyu && cd $cwd && \
    echo 'export PATH=/opt/iwyu/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/iwyu/bin:${PATH}"

COPY rootfs/usr/local/bin/init /usr/local/bin/init
RUN  chmod 755 "/usr/local/bin/init"

ENTRYPOINT [ "sh", "/usr/local/bin/init" ]

FROM ci as jenkins

ARG JENKINS_UID=1000
ARG JENKINS_GID=1000

RUN addgroup --gid "${JENKINS_GID}" jenkins && \
    adduser --uid "${JENKINS_UID}" --gid "${JENKINS_GID}" --disabled-password \
        --gecos "" jenkins

ENTRYPOINT [ "sh", "-c" ]

FROM ci as dev

VOLUME /scp-firmware
WORKDIR /scp-firmware

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y --no-install-recommends nano sudo

ENV DEBIAN_FRONTEND=

FROM dev as vagrant

VOLUME /vagrant
WORKDIR /vagrant

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y --no-install-recommends openssh-server && \
    mkdir "/run/sshd"

ENV DEBIAN_FRONTEND=

RUN adduser --disabled-password --gecos "" vagrant && \
    usermod -aG sudo vagrant && \
    passwd -d vagrant

ENV NOTVISIBLE="in users profile"
RUN echo "export VISIBLE=now" >> "/etc/profile"

RUN printf "\nAcceptEnv ARMLMD_LICENSE_FILE" >> /etc/ssh/sshd_config

RUN mkdir -p "/home/vagrant/.ssh" && \
    chmod 755 "/home/vagrant/.ssh"

COPY rootfs/home/vagrant/.ssh/authorized_keys /home/vagrant/.ssh/authorized_keys
RUN  chmod 644 "/home/vagrant/.ssh/authorized_keys"

RUN printf "\ncd /vagrant" >> "/home/vagrant/.bashrc"

EXPOSE 22
