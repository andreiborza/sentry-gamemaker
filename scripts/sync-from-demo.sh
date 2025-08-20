#!/bin/bash

set -e

DEMO_DIR="demo/SentryGameMaker"
SRC_DIR="src"

echo "🔄 Syncing changes from demo back to src..."

# Sync Sentry.yy extension file
if [ -f "$DEMO_DIR/extensions/Sentry/Sentry.yy" ]; then
    echo "📄 Syncing $DEMO_DIR/extensions/Sentry/Sentry.yy -> $SRC_DIR/extensions/Sentry/Sentry.yy"
    cp "$DEMO_DIR/extensions/Sentry/Sentry.yy" "$SRC_DIR/extensions/Sentry/Sentry.yy"
else
    echo "⚠️  Warning: $DEMO_DIR/extensions/Sentry/Sentry.yy not found"
fi


echo "✅ Sync complete!"