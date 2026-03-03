# Slipstream fork (DNS over QUIC over shadowsocks)

Install build dependencies (Ubuntu/Debian).

Only tested on Ubuntu 24.04

```bash
sudo apt-get update
sudo apt-get install -y \
  python3-pip cmake git pkg-config libssl-dev ninja-build gcc g++
python3 -m pip install --user meson
```

### Configure and build

```bash
~/.local/bin/meson setup build
~/.local/bin/meson compile -C build
```

For optimized release binaries:

```bash
~/.local/bin/meson setup --buildtype=release -Db_lto=true --warnlevel=0 build-release
~/.local/bin/meson compile -C build-release
```

### Verify binaries

```bash
./build/slipstream-server --help
./build/slipstream-client --help
```



DNS server:
```bash
ns.fzserver.com A record to [server ip]
ns.fzserver.com NS record to ns.fzserver.com
```

server side service:

```bash
[Unit]
Description=Slipstream DNS Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=/home/slipstream
ExecStart=/home/slipstream/build/slipstream-server --dns-listen-port=8853 --target-address=127.0.0.1:9090 --domain=ns.fzserver.com
Restart=on-failure
RestartSec=2

[Install]
WantedBy=multi-user.target
```
`9090` is the port of the Shadowsocks server running on the same machine. Slipstream tunnels traffic to this local Shadowsocks instance, which then handles the actual proxy connection.
- `8853` is the port of the Slipstream server.
- `ns.fzserver.com` is the NS domain name of the Slipstream server.
- `127.0.0.1` is the IP address of the machine running the Shadowsocks server.


## Iptables:

```bash
iptables -I INPUT -p udp --dport 8853 -j ACCEPT
iptables -t nat -I PREROUTING -i ens5 -p udp --dport 53 -j REDIRECT --to-ports 8853
```

`ens5` is the network interface name of the machine.


Client side service:

create a folder inside home directory called slipstream and put the slipstream-client-1.9 inside it.

```bash
[Unit]
Description=Slipstream DNS client
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=/home/slipstream
ExecStart=/home/slipstream/slipstream-client-1.9 --resolver=8.8.8.8:53 --resolver=8.8.4.4:53 --domain=ns.fzserver.com --tcp-listen-port=7000
Restart=on-failure
RestartSec=2

[Install]
WantedBy=multi-user.target
```




---

**What has changed**

1. Fixed the assertion crash in SPCDNS:
- Changed class assertion in [codec.c](/home/slipstream/extern/SPCDNS/src/codec.c:379) from `<= 4` to `<= 65535`.
- This prevents aborting when non-`IN/CH/HS/CS` class queries are re-encoded.

2. Added defensive DNS class filtering in server decode:
- In [slipstream_server.c](/home/slipstream/src/slipstream_server.c:156), queries with class other than `CLASS_IN` are now refused (`RCODE_REFUSED`) early.
- This avoids feeding unsupported classes into the rest of the pipeline.

3. Hardened fd/pipe lifecycle to reduce `Bad file descriptor` noise:
- Initialized stream descriptors to `-1` in [slipstream_server.c](/home/slipstream/src/slipstream_server.c:222).
- Closed `fd`, `pipefd[0]`, and `pipefd[1]` safely in [slipstream_server.c](/home/slipstream/src/slipstream_server.c:265).
- Replaced noisy `printf` on `POLLNVAL` with debug log in [slipstream_server.c](/home/slipstream/src/slipstream_server.c:390).
- Closed socket on `connect()` failure in [slipstream_server.c](/home/slipstream/src/slipstream_server.c:438).
- Treated `EPIPE/EBADF/ECONNRESET` on send as normal teardown in [slipstream_server.c](/home/slipstream/src/slipstream_server.c:472).
- On stream FIN, close socket/pipe safely in [slipstream_server.c](/home/slipstream/src/slipstream_server.c:588).


----



![GitHub Release](https://img.shields.io/github/v/release/EndPositive/slipstream?include_prereleases&sort=semver&display_name=tag)
![GitHub License](https://img.shields.io/github/license/EndPositive/slipstream)

A high-performance covert channel over DNS, powered by QUIC multipath.

<p align="center">
  <picture align="center">
    <source media="(prefers-color-scheme: dark)" srcset="docs/assets/file_transfer_times_dark.png">
    <source media="(prefers-color-scheme: light)" srcset="docs/assets/file_transfer_times_light.png">
    <img alt="Shows a bar chart with benchmark results." src="docs/assets/file_transfer_times_light.png">
  </picture>
</p>

<p align="center">
  <i>Exfiltrating a 10 MB file over a single DNS resolver.</i>
</p>

## Highlights

* Adaptive congestion control for rate-limited resolvers
* Parallel routing over multiple multiple rate-limited resolvers
* 60% lower header overhead than DNSTT

## Installation

Get the latest binaries [GitHub releases](https://github.com/EndPositive/slipstream/releases/latest) or pull the latest version from the [GitHub Container Registry](https://github.com/users/EndPositive/packages?repo_name=slipstream).

## Documentation

slipstream's documentation is available at [endpositive.github.io/slipstream](https://endpositive.github.io/slipstream/).

# Acknowledgements

David Fifield's DNSTT and Turbo Tunnel concept has been a massive source of inspiration.
Although slipstream inherits no code, this work could not have been possible without his ideas.
