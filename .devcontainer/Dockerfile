# .devcontainer/Dockerfile

# Usar una imagen Ubuntu específica para amd64
FROM --platform=linux/amd64 ubuntu:22.04

# Configurar el entorno e instalar dependencias
RUN apt-get update && \
    apt-get install -yqq \
        build-essential \
        cmake \
        gcc \
        g++ \
        gdb \
        curl \
        libssl-dev \
        python3 \
        python3-pip \
        buildah \
        qemu-system-aarch64 \
        qemu-system-x86 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Instalar Conan 2
RUN pip3 install --upgrade pip && \
    pip3 install "conan>=2.0"

# Crear directorio para el SDK y configurar entorno
RUN mkdir -p /opt/vxworks-sdk

# Añadir /opt/vxworks-sdk al PATH para facilidad de uso del SDK
ENV WIND_SDK_HOME=/opt/vxworks-sdk
ENV WIND_SDK_CC_SYSROOT=${WIND_SDK_HOME}/vxsdk/sysroot
ENV WRSD_LICENSE_FILE=${WIND_SDK_HOME}/license
ENV PATH=${WIND_SDK_HOME}/vxsdk/host/x86_64-linux/bin:${PATH}
ENV LD_LIBRARY_PATH=${WIND_SDK_HOME}/vxsdk/host/x86_64-linux/lib:${LD_LIBRARY_PATH}
ENV PATH=${WIND_SDK_HOME}/tools/debug/24.03/x86_64-linux2/bin:${PATH}
ENV LD_LIBRARY_PATH=${WIND_SDK_HOME}/tools/debug/24.03/x86_64-linux2/lib:${LD_LIBRARY_PATH}
ENV WIND_SDK_CCBASE_PATH=${WIND_SDK_HOME}/compilers/llvm-17.0.6.1/LINUX64/bin

# Configuración de las herramientas de compilación específicas del SDK
ENV CC=wr-cc
ENV CXX=wr-c++
ENV CPP=wr-cpp
ENV LD=wr-ld
ENV AR=wr-ar
ENV NM=wr-nm
ENV OBJCOMPY=wr-objcopy
ENV OBJDUMP=wr-objdump
ENV RANLIB=wr-ranlib
ENV READELF=wr-readelf
ENV SIZE=wr-size
ENV STRIP=wr-strip

RUN echo 'source ${WIND_SDK_HOME}/vxsdk/sysroot/usr/rust/rustenv.linux' >> /root/.bashrc

WORKDIR /workspace

RUN mkdir -p /root/.conan2/profiles && \
    echo "[settings]" > /root/.conan2/profiles/vxworks && \
    echo "os=VxWorks" >> /root/.conan2/profiles/vxworks && \
    echo "os.version=7" >> /root/.conan2/profiles/vxworks && \
    echo "arch=armv8" >> /root/.conan2/profiles/vxworks && \
    echo "compiler=clang" >> /root/.conan2/profiles/vxworks && \
    echo "compiler.version=17" >> /root/.conan2/profiles/vxworks && \
    echo "compiler.libcxx=libstdc++11" >> /root/.conan2/profiles/vxworks && \
    echo "build_type=Release" >> /root/.conan2/profiles/vxworks && \
    echo "" >> /root/.conan2/profiles/vxworks && \
    echo "[options]" >> /root/.conan2/profiles/vxworks && \
    echo "shared=False" >> /root/.conan2/profiles/vxworks && \
    echo "fPIC=True" >> /root/.conan2/profiles/vxworks && \
    echo "libxml2/*:iconv=False" >> /root/.conan2/profiles/vxworks && \
    echo "libxml2/*:zlib=False" >> /root/.conan2/profiles/vxworks && \
    echo "*:with_xnnpack=False" >> /root/.conan2/profiles/vxworks && \
    echo "*:tools.cmake.cmaketoolchain:user_toolchain=['/opt/vxworks-sdk/vxsdk/sysroot/mk/rtp.toolchain.cmake']" >> /root/.conan2/profiles/vxworks && \
    echo "*:tools.build:cxxflags=['--std=c++21','-DINCLUDE_VSB_CONFIG_FILE', '-I/opt/vxworks-sdk/vxsdk/sysroot/usr/h']" >> /root/.conan2/profiles/vxworks && \
    echo "*:tools.build:ldflags=['-L/opt/vxworks-sdk/vxsdk/sysroot/usr/lib']" >> /root/.conan2/profiles/vxworks && \
    echo "" >> /root/.conan2/profiles/vxworks && \
    echo "[conf]" >> /root/.conan2/profiles/vxworks && \
    echo "tools.cmake.cmaketoolchain:user_toolchain=['/opt/vxworks-sdk/vxsdk/sysroot/mk/rtp.toolchain.cmake']" >> /root/.conan2/profiles/vxworks