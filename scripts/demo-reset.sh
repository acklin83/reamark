#!/usr/bin/env sh
# Wipe and restart the public ReaMark demo instance (DB + uploads).
# Run daily via cron or Synology DSM Task Scheduler.
#
#   ./demo-reset.sh /volume1/docker/reamark-demo
#
# The directory must contain docker-compose.demo.yml (and optionally .env.demo).
set -eu

DIR="${1:-/volume1/docker/reamark-demo}"
cd "$DIR"

COMPOSE="docker compose -f docker-compose.demo.yml"
[ -f .env.demo ] && COMPOSE="$COMPOSE --env-file .env.demo"

$COMPOSE down
rm -rf data-demo
# Synology/Container Manager does not auto-create bind-mount dirs — recreate it.
mkdir -p data-demo/uploads
$COMPOSE up -d

echo "ReaMark demo reset at $(date) — fresh empty instance is up."
