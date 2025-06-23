FROM debian:bookworm

# Update the package repository and install essential tools
RUN apt-get update && apt-get install -y \
    wget \
    build-essential \
    make \
    cmake \
    git \
    libopencv-dev \
    && rm -rf /var/lib/apt/lists/*


# Set working directory in the container
WORKDIR /app

# Copy source code into the container
COPY . .

# Get relevant libraries
WORKDIR /app/src/OnnxInferLib
# RUN wget "https://github.com/microsoft/onnxruntime/releases/download/v1.22.0/onnxruntime-linux-x64-1.22.0.tgz"
# RUN tar -xvf onnxruntime-linux-x64-1.22.0.tgz
# RUN mv onnxruntime-linux-x64-1.22.0 onnxruntime
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.22.0/onnxruntime-linux-aarch64-1.22.0.tgz
RUN tar -xvf onnxruntime-linux-aarch64-1.22.0.tgz
RUN mv onnxruntime-linux-aarch64-1.22.0 onnxruntime


# (Optional) Build your project (example for CMake)

WORKDIR /app
RUN git clone https://github.com/redis/hiredis.git
RUN cd hiredis && make && make install

WORKDIR /app
RUN git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
RUN cd AMQP-CPP && mkdir build && cd build && cmake .. -DAMQP-CPP_BUILD_SHARED=ON && cmake --build . --target install

WORKDIR /app
RUN make clean && make prep && make build-project

# CMD ["tail", "-f", "/dev/null"]
# Run app
CMD ["make", "run"]
