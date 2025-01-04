#!/usr/bin/env bash
##########################################################
# deploy_influxdb.sh
#
# Script to deploy a local InfluxDB instance for dev/testing.
# Uses Docker if available, otherwise installs via package manager.
# Adjust as needed for your environment.
##########################################################

set -e

INFLUXDB_DOCKER_IMAGE="influxdb:1.8"
INFLUXDB_CONTAINER_NAME="dev_influxdb"
LOCAL_PORT=8086
DOCKER_AVAILABLE=$(command -v docker || true)

if [ -n "$DOCKER_AVAILABLE" ]; then
  echo "Docker found. Deploying InfluxDB in a Docker container."
  # Check if container is already running
  if [ "$(docker ps -q -f name=${INFLUXDB_CONTAINER_NAME})" ]; then
    echo "InfluxDB container is already running."
  else
    # Check if container exists (stopped)
    if [ "$(docker ps -aq -f status=exited -f name=${INFLUXDB_CONTAINER_NAME})" ]; then
      echo "InfluxDB container exists but is stopped. Starting..."
      docker start ${INFLUXDB_CONTAINER_NAME}
    else
      echo "No existing container found. Pulling image and creating container."
      docker pull ${INFLUXDB_DOCKER_IMAGE}
      docker run -d \
        --name ${INFLUXDB_CONTAINER_NAME} \
        -p ${LOCAL_PORT}:8086 \
        ${INFLUXDB_DOCKER_IMAGE}
    fi
  fi
  echo "InfluxDB is running on port ${LOCAL_PORT}."
  echo "Use 'docker logs ${INFLUXDB_CONTAINER_NAME}' to see logs."
else
  echo "Docker not available. Installing InfluxDB via package manager..."

  if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
  else
    echo "Unknown OS, please install InfluxDB manually."
    exit 1
  fi

  case $OS in
    ubuntu|debian)
      echo "Installing InfluxDB on Debian/Ubuntu..."
      wget -qO- https://repos.influxdata.com/influxdb.key | sudo apt-key add -
      source /etc/os-release
      echo "deb https://repos.influxdata.com/${ID} ${VERSION_CODENAME} stable" | sudo tee /etc/apt/sources.list.d/influxdb.list
      sudo apt-get update
      sudo apt-get install -y influxdb
      sudo systemctl enable influxdb
      sudo systemctl start influxdb
      ;;
    centos|rhel|fedora)
      echo "Installing InfluxDB on CentOS/RHEL/Fedora..."
      sudo yum install -y influxdb
      sudo systemctl enable influxdb
      sudo systemctl start influxdb
      ;;
    *)
      echo "OS not supported by this script. Install InfluxDB manually."
      exit 1
      ;;
  esac

  echo "InfluxDB is running. Listening on port 8086 by default."
fi

# Create a dev database if it doesn't exist
echo "Attempting to create dev database 'market_data_dev'..."
curl -XPOST "http://localhost:${LOCAL_PORT}/query" --data-urlencode "q=CREATE DATABASE market_data_dev" || true

echo "Done."
