![GitHub Release](https://img.shields.io/github/v/release/EndPositive/slipstream?include_prereleases&sort=semver&display_name=tag)
![GitHub License](https://img.shields.io/github/license/EndPositive/slipstream)

# slipstream

A high-performance covert channel over DNS, powered by QUIC multipath.

`slipstream` tunnels TCP traffic over DNS:

- `slipstream-client` listens on a local TCP port and accepts application connections.
- Client and server exchange QUIC packets encoded in DNS messages.
- `slipstream-server` decodes DNS traffic and forwards stream data to a configured upstream TCP address.

## Build

Install dependencies on Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y \
  python3-pip cmake git pkg-config libssl-dev ninja-build gcc g++
python3 -m pip install --user meson
```

Configure and build:

```bash
~/.local/bin/meson setup build
~/.local/bin/meson compile -C build
```

Release build:

```bash
~/.local/bin/meson setup --buildtype=release -Db_lto=true --warnlevel=0 build-release
~/.local/bin/meson compile -C build-release
```

## Binaries

```bash
./build/slipstream-server --help
./build/slipstream-client --help
```

### Server options

- `--domain` (required)
- `--target-address` (default: `127.0.0.1:5201`)
- `--dns-listen-port` (default: `53`)
- `--dns-listen-ipv6` (default: disabled)
- `--cert` (default: `certs/cert.pem`)
- `--key` (default: `certs/key.pem`)

### Client options

- `--domain` (required)
- `--resolver` (required, can be set multiple times)
- `--tcp-listen-port` (default: `5201`)
- `--congestion-control` (`dcubic` default, or `bbr`)
- `--gso` (default: disabled)
- `--keep-alive-interval` (default: `400`, `0` disables keep alive)

Notes:

- Resolver entries accept IPv4 or bracketed IPv6 forms (for example `8.8.8.8:53` or `[2001:db8::1]:53`).
- IPv4 and IPv6 resolver addresses cannot be mixed in the same client run.

## Quick usage

Server:

```bash
./build/slipstream-server \
  --domain=test.com \
  --target-address=127.0.0.1:5201
```

Client:

```bash
./build/slipstream-client \
  --domain=test.com \
  --resolver=1.1.1.1:53 \
  --resolver=8.8.8.8:53
```

For more detail, see:

- `docs/installation/`
- `docs/usage.md`
- `docs/protocol.md`
