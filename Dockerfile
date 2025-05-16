# FROM nvidia/cuda:12.8.0-cudnn-devel-ubuntu20.04 as runner
# FROM nvidia/cuda:12.8.0-cudnn-devel-ubuntu20.04 as builder

# FROM nvidia/cuda:11.8.0-cudnn8-devel-ubuntu22.04 as runner
# FROM nvidia/cuda:11.8.0-cudnn8-devel-ubuntu22.04 as builder

# FROM nvidia/cuda:11.8.0-devel-ubuntu18.04 as runner
# FROM nvidia/cuda:11.8.0-devel-ubuntu18.04 as builder

FROM augustus/mau-profile as runner
FROM augustus/mau-profile as builder

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y tzdata

RUN apt-get update && apt-get install -y \
    curl \
    jq \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    python3-venv libz3-dev libssl-dev \
    z3 \
    && rm -rf /var/lib/apt/lists/*
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain nightly-2023-04-01
ENV PATH="/root/.cargo/bin:${PATH}"
RUN rustc --version && cargo --version

RUN pip3 install --upgrade pip
RUN mkdir /bins

RUN apt-get update && apt-get install -y clang pkg-config cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /builder

COPY Cargo.toml .
COPY rust-toolchain.toml .
COPY src ./src
COPY cli ./cli
COPY benches ./benches
COPY externals ./externals

# Copy the dynamic library
COPY runner/librunner.so /usr/local/lib/librunner.so

ENV LD_LIBRARY_PATH="/usr/local/nvidia/lib:/usr/local/nvidia/lib64:$LD_LIBRARY_PATH"
ENV LIBRARY_PATH="/usr/local/cuda/lib64:/usr/local/lib:/usr/local/cuda/lib64/stubs:$LIBRARY_PATH"

# build offchain binary
WORKDIR /builder/cli
RUN RUSTFLAGS="-Awarnings" cargo build --release --locked
RUN cp target/release/cli /bins/cli_offchain

# build onchain binary
RUN sed -i -e 's/"cmp"/"cmp","flashloan_v2"/g' ../Cargo.toml
RUN RUSTFLAGS="-Awarnings" cargo build --release --locked
RUN cp target/release/cli /bins/cli_onchain

RUN sed -i -e 's/"deployer_is_attacker"/"print_logs"/g' ../Cargo.toml
RUN sed -i -e 's/"print_txn_corpus",//g' ../Cargo.toml
RUN sed -i -e 's/"full_trace",//g' ../Cargo.toml
RUN RUSTFLAGS="-Awarnings" cargo build --release --locked
RUN cp target/release/cli /bins/cli_print_logs

FROM runner
WORKDIR /app
COPY --from=builder /bins /bins

COPY ui /app/ui
RUN pip3 install -r ui/requirements.txt
RUN pip3 install solc-select

COPY proxy /app/proxy
RUN pip3 install -r proxy/requirements.txt

COPY ui/start.sh .
RUN chmod +x start.sh

EXPOSE 8000

CMD ./start.sh
