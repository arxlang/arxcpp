# ref: https://github.com/mamba-org/micromamba-docker/blob/main/Dockerfile

FROM condaforge/mambaforge:latest

LABEL maintainer="Ivan Ogasawara <ivan.ogasawara@gmail.com>"
LABEL org.opencontainers.image.title="ArxLang"
LABEL org.opencontainers.image.authors="ArxLang Team"
LABEL org.opencontainers.image.source="https://github.com/arxlang/arx"
LABEL org.opencontainers.image.version="latest"
LABEL org.opencontainers.image.description="ArxLang"
LABEL org.thegraphnetwork.arx.version="latest"

# it is the default, but using it here to have it explicitly
USER root

SHELL ["/bin/bash", "-c"]
# Use bash in Dockerfile RUN commands and make sure bashrc is sourced when
# executing commands with /bin/bash -c
# Needed to have the micromamba activate command configured etc.

ENV ENV_NAME=arx
ENV DEBIAN_FRONTEND=noninteractive
ARG UID=1000
ARG GID=1000

RUN apt-get update -y \
  && apt-get install -y \
    build-essential \
    sudo \
    tini \
    vim \
  && rm -rf /var/lib/apt/lists/* \
    /var/cache/apt/archives \
    /tmp/* \
  && addgroup --gid ${GID} arx \
  && useradd --uid ${UID} --gid ${GID} -ms /bin/bash arx \
  && mkdir -p /opt/arx \
  && chmod -R a+rwx /opt/conda /opt/arx \
  && export ENV_NAME="$ENV_NAME" \
  && echo "arx ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/arx \
  && chmod 0440 /etc/sudoers.d/arx \
  && mkdir -p /opt/arx \
  && chown arx:arx /opt/arx

USER arx

WORKDIR /opt/arx

COPY --chown=arx:arx conda/ /tmp/conda

ENV PATH /opt/conda/envs/$ENV_NAME/bin:$PATH
ENV PYTHONPATH='/opt/arx'

RUN mamba env create -n $ENV_NAME --file /tmp/conda/dev-linux-64.yaml \
  && conda clean --all \
  && find /opt/conda/ -type f,l -name '*.pyc' -delete \
  && find /opt/conda/ -type f,l -name '*.js.map' -delete \
  && rm -rf /opt/conda/pkgs /tmp/*

COPY --chown=arx:arx containers/entrypoint.sh /opt/entrypoint.sh

RUN chmod +x /opt/entrypoint.sh \
  && echo "source /opt/entrypoint.sh" > ~/.bashrc


ENTRYPOINT ["tini", "--", "/opt/entrypoint.sh"]
CMD ["bash"]
