# FROM buildpack-deps:buster
FROM conanio/gcc7:latest

ENV PREMAKE_VERSION 5.0.0-alpha15

# ENV LIBPQ_VERSION 11.7-0+deb10u1
# ENV LIBPQXX_VERSION 6.2.5-1

USER root

RUN wget -qO- \
        https://github.com/premake/premake-core/releases/download/v${PREMAKE_VERSION}/premake-${PREMAKE_VERSION}-linux.tar.gz | \
        tar -zxC /usr/bin/ \
    && chmod +x /usr/bin/premake5

USER conan

# RUN set -ex \
#     && deps=" \
#         libpq-dev=$LIBPQ_VERSION \
#         libpqxx-dev=$LIBPQXX_VERSION \
#     " \
#     && apt-get update && apt-get install -y $deps --no-install-recommends

