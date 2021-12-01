FROM ubuntu:18.04

RUN apt-get -qq update --yes \
    && apt-get -qq install --yes \
        wget \
        software-properties-common \
        build-essential \
        make \
        cmake \
        libgoogle-glog-dev \
        libgoogle-glog0v5 \
        sudo \
        gdb \
        bsdmainutils \
        flex \
        bison \
        vim \
    && rm -rf /var/lib/apt/lists/*

RUN export uid=1000 gid=1000 home_dir=/home/developer && \
    mkdir -p ${home_dir} && \
    echo "developer:x:${uid}:${gid}:Developer,,,:${home_dir}:/bin/bash" >> /etc/passwd && \
    echo "developer:x:${uid}:" >> /etc/group && \
    echo "developer ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/developer && \
    chmod 0440 /etc/sudoers.d/developer && \
    chown ${uid}:${gid} -R ${home_dir}

WORKDIR /home/developer
USER developer
ENV HOME /home/developer

ENTRYPOINT /bin/bash
