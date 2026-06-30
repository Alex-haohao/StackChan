# StackChan Server Ops

This directory contains self-hosting assets for the official StackChan server
flow. It intentionally lives outside the upstream `manifest/` examples so local
production deployment can evolve without rewriting official source or protocol
files.

The target architecture is:

- StackChan App -> HTTPS/WSS reverse proxy -> StackChan Server
- StackChan device -> HTTPS/WSS reverse proxy -> StackChan Server
- Optional MCP or custom API integrations attach behind the server later

Do not commit real hostnames, passwords, JWT secrets, RSA private keys, or
generated config files. Keep those values in the workspace root `.env`, a local
secret store, or files under `workspace/stackchan-secrets/`.

## Files

- `compose/docker-compose.yml`: minimal single-host Docker Compose topology.
- `docker/Dockerfile`: container build for the Go server.
- `config/config.yaml.example`: production config shape with placeholders only.
- `reverse-proxy/Caddyfile.example`: TLS reverse proxy example.
- `reverse-proxy/nginx-stackchan.conf.example`: nginx reverse proxy example.
- `systemd/stackchan-server.service.example`: systemd service example for
  servers that already run MySQL and nginx.

## Local build

From `server/ops/compose`:

```bash
STACKCHAN_CONFIG_PATH=/absolute/path/to/config.yaml docker compose build
STACKCHAN_CONFIG_PATH=/absolute/path/to/config.yaml docker compose up -d
```

The config file is mounted read-only into `/app/config.yaml`. The database
password and other secrets are not stored in this directory.

## Existing MySQL + nginx server

If the cloud server already has MySQL and nginx, use the systemd/nginx examples
instead of installing Docker just for StackChan:

```bash
sudo install -d -o stackchan-server -g stackchan-server /opt/stackchan-server
sudo install -d -o stackchan-server -g stackchan-server /var/lib/stackchan-server
sudo install -d -o stackchan-server -g stackchan-server /var/log/stackchan-server
```

Place the real config at:

```text
/opt/stackchan-server/shared/config.yaml
```

Each release should expose that file as:

```bash
ln -sfn ../shared/config.yaml /opt/stackchan-server/current/config.yaml
```

Then install the service and reverse proxy examples after replacing placeholder
hostnames and paths.
