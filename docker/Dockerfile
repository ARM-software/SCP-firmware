#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

FROM ubuntu:20.04@sha256:9fa30fcef427e5e88c76bc41ad37b7cc573e1d79cecb23035e413c4be6e476ab as ci

ARG ARM_GNU_RM_URL="https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2"
ARG DOXYGEN_URL="https://sourceforge.net/projects/doxygen/files/rel-1.8.13/doxygen-1.8.13.linux.bin.tar.gz"
ARG CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v3.25.2/cmake-3.25.2-linux-x86_64.tar.gz"
ARG AARCH64_GCC_URL="https://developer.arm.com/-/media/Files/downloads/gnu-a/10.2-2020.11/binrel/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf.tar.xz"
ARG CPPCHECK_SRC_URL="https://github.com/danmar/cppcheck.git"
ARG CPPCHECK_CHECKOUT_TAG="1.90"
ARG IWYU_SRC_URL="https://github.com/include-what-you-use/include-what-you-use.git"
ARG LINUX_SCRIPTS_URL="https://raw.githubusercontent.com/torvalds/linux/master/scripts"

ARG ROOTFS_PATH="docker/rootfs"

ENV ARMLMD_LICENSE_FILE=

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        bundler gcc g++ gpg-agent git gosu lsb-release make ninja-build \
        xz-utils python3 python3-pip software-properties-common wget lcov && \
    apt-get update && \
    python3 -m pip install --upgrade pip && \
    gem install bundler

ENV DEBIAN_FRONTEND=

RUN mkdir "/opt/arm-gnu-rm" && \
    wget -nv -O - -c "${ARM_GNU_RM_URL}" | \
    tar -jxf - -C "/opt/arm-gnu-rm" --strip-components=1 && \
    echo 'export PATH=/opt/arm-gnu-rm/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/arm-gnu-rm/bin:${PATH}"

VOLUME "/opt/arm-compiler-6"
ENV PATH="/opt/arm-compiler-6/bin:${PATH}"

RUN mkdir "/opt/doxygen" && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        libclang1-9 libclang-9-dev libclang-cpp9 && \
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

RUN apt-get update && \
    wget --no-check-certificate -O - \
        https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository \
        'deb http://apt.llvm.org/focal/   llvm-toolchain-focal-13  main' && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        clang-tidy-13 clang-13 llvm-13 libclang-13-dev llvm-13-dev lld-13

COPY ${ROOTFS_PATH}/usr/local/bin/prepare_llvm /usr/local/bin/prepare_llvm
RUN  chmod 755 "/usr/local/bin/prepare_llvm"
RUN  prepare_llvm

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
    git clone "${IWYU_SRC_URL}" -b clang_13 --single-branch iwyu-13 && \
    cmake -G "Ninja" -DCMAKE_PREFIX_PATH=/usr/lib/llvm-13g \
    ./iwyu-13 && cmake --build . && \
    cd bin && ln -s include-what-you-use iwyu && cd $cwd && \
    echo 'export PATH=/opt/iwyu/bin:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/iwyu/bin:${PATH}"

RUN cwd=$PWD && mkdir "/opt/checkpatch" && cd "/opt/checkpatch" && \
    wget -nvc "${LINUX_SCRIPTS_URL}/checkpatch.pl" && \
    wget -nvc "${LINUX_SCRIPTS_URL}/spelling.txt" && \
    wget -nvc "${LINUX_SCRIPTS_URL}/const_structs.checkpatch" && \
    chmod 755 "/opt/checkpatch/checkpatch.pl" && \
    echo 'export PATH=/opt/checkpatch:${PATH}' >> \
        "/etc/profile.d/50-scp-firmware-env.sh"

ENV PATH="/opt/checkpatch:${PATH}"

COPY ${ROOTFS_PATH}/usr/local/bin/init /usr/local/bin/init
RUN  chmod 755 "/usr/local/bin/init"

RUN ln -s /usr/bin/python3 /usr/bin/python

COPY requirements.txt .
RUN PIP_ROOT_USER_ACTION=ignore python3 -m pip install -r requirements.txt

COPY Gemfile .
RUN BUNDLE_SILENCE_ROOT_WARNING=true bundler install

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

RUN adduser --disabled-password --gecos "" user && \
    usermod -aG sudo user && \
    passwd -d user

ENV NOTVISIBLE="in users profile"
RUN echo "export VISIBLE=now" >> "/etc/profile"

RUN echo "\nexport USER=user" >> "/home/user/.bashrc"
ENV PATH="/home/user/.local/bin:${PATH}"

ENTRYPOINT [ "sh", "/usr/local/bin/init" ]

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

COPY ${ROOTFS_PATH}/home/vagrant/.ssh/authorized_keys /home/vagrant/.ssh/authorized_keys
RUN  chmod 644 "/home/vagrant/.ssh/authorized_keys"

RUN printf "\ncd /vagrant" >> "/home/vagrant/.bashrc"

RUN echo "\nexport USER=vagrant" >> "/home/user/.bashrc"
ENV PATH="/home/vagrant/.local/bin:${PATH}"

EXPOSE 22
