#!/bin/bash
# Root build script
set -e
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"${PROJECT_DIR}/backend/build.sh"
