---
title: Docker image
parent: Installation
nav_order: 22
---

# GitHub Container Registry

Docker images are published to the GHCR at every release.
The client and server are packaged in different images.

* [ghcr.io/endpositive/slipstream-client](https://ghcr.io/endpositive/slipstream-client)
* [ghcr.io/endpositive/slipstream-server](https://ghcr.io/endpositive/slipstream-server)

### Tags

* latest
* vX.X.X

# Usage

The client requires port 5201 to be forwarded to the host.
The server requires port 53 to be forwarded.

```shell
$ docker run \
  --rm \
  -p 53:53 \
  ghcr.io/endpositive/slipstream-server:v0.0.1 \
  --target-address=x.x.x.x:yy \
  --domain=test.com
```

```shell
$ docker run \
  --rm \
  -p 5201:5201 \
  ghcr.io/endpositive/slipstream-server:v0.0.1 \
  --domain=test.com \
  --resolver=1.1.1.1:53
```

Any TCP connections on the client's port `5201` will now be forwarded to `x.x.x.x:yy`.
You could also run a slipstream on a different port than 53, but then a public resolver won't be able to reach the server.
This may be useful in scenarios where you setup a direct connection between the client and server rather than through public DNS infrastructure.
