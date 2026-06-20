# Running a public demo instance

A demo instance lets anyone try ReaMark without an account — create projects,
upload tracks, leave comments — on a throwaway instance that resets regularly.

> **Warning:** demo mode has **no access control**. Run it on a separate
> instance with its own data, never alongside real client material.

## What demo mode does (`REAMARK_DEMO_MODE=1`)

- The admin UI opens straight to the dashboard — no login.
- The entire admin API (create project/song, upload, comment, edit, delete) is
  public.
- A banner warns visitors that everything is wiped regularly.
- Email notifications and the test-email button are disabled.
- Uploads are capped (`REAMARK_DEMO_MAX_UPLOAD_MB`, default 30) and the number
  of projects is capped (`REAMARK_DEMO_MAX_PROJECTS`, default 100).

## Setup

On the host (e.g. Synology, in a directory separate from production):

```bash
mkdir -p /volume1/docker/reamark-demo && cd /volume1/docker/reamark-demo
curl -O https://raw.githubusercontent.com/acklin83/reamark/main/docker-compose.demo.yml
curl -O https://raw.githubusercontent.com/acklin83/reamark/main/scripts/demo-reset.sh
chmod +x demo-reset.sh
echo "REAMARK_SECRET_KEY=$(openssl rand -hex 32)" > .env.demo
mkdir -p data-demo/uploads   # Synology does not auto-create bind-mount dirs
docker compose -f docker-compose.demo.yml --env-file .env.demo up -d
```

The demo listens on port **8091**. Point a reverse proxy at it
(e.g. `reamark.stoersender.ch` → `localhost:8091`) with TLS.

## Daily reset

Wipe and restart once a day so abuse/clutter never lasts long.

**Synology:** Control Panel → Task Scheduler → Create → Scheduled Task (user
`root`), daily at e.g. 04:00:

```
/volume1/docker/reamark-demo/demo-reset.sh /volume1/docker/reamark-demo
```

**cron (generic):**

```
0 4 * * * /volume1/docker/reamark-demo/demo-reset.sh /volume1/docker/reamark-demo >> /var/log/reamark-demo-reset.log 2>&1
```
