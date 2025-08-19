#!/bin/bash

# Generate yymanifest.xml for GameMaker extension package
# Usage: ./generate_manifest.sh <extension_folder>

EXTENSION_DIR="$1"
if [ -z "$EXTENSION_DIR" ]; then
    echo "Usage: $0 <extension_folder>"
    exit 1
fi

if [ ! -d "$EXTENSION_DIR" ]; then
    echo "Error: Directory $EXTENSION_DIR not found"
    exit 1
fi

echo '<?xml version="1.0" encoding="utf-8"?>'
echo '<files>'

# Find all files in the extension directory (excluding directories and manifest)
find "$EXTENSION_DIR" -type f | while read -r file; do
    # Get relative path from extension directory
    relative_path="${file#$EXTENSION_DIR/}"
    
    # Skip the yymanifest.xml file itself
    if [ "$relative_path" = "yymanifest.xml" ]; then
        continue
    fi
    
    # Calculate MD5 hash
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        md5_hash=$(md5 -q "$file")
    else
        # Linux
        md5_hash=$(md5sum "$file" | cut -d' ' -f1)
    fi
    
    # Convert to uppercase (GameMaker uses uppercase MD5)
    md5_hash=$(echo "$md5_hash" | tr '[:lower:]' '[:upper:]')
    
    # Output XML line
    echo "    <file md5=\"$md5_hash\">$relative_path</file>"
done

echo '</files>'