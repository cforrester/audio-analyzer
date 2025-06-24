FROM ubuntu:22.04
RUN apt-get update && \
    apt-get install -y g++ \
    cmake pkg-config libfftw3-dev libaubio-dev \
    libssl-dev \
    libasio-dev \
    ffmpeg libjsoncpp-dev \
    make && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN mkdir build && cd build && cmake .. && make
CMD ["./build/audio_analyzer"]
