#!/bin/bash
set -e
set -o pipefail

NEW_VER="$1"

if [ -z "$NEW_VER" ]; then
    echo "Usage: $0 <new_version>"
    exit 1
fi

echo ">> [Source] Updating to $NEW_VER..."

OLD_VER=$(grep "^pkgver=" PKGBUILD | cut -d'=' -f2)
OLD_HASH=$(grep "^sha256sums=" PKGBUILD | cut -d"'" -f2)
echo "   Old Version: $OLD_VER"

SRC_LINE=$(grep "source =" .SRCINFO | head -n 1)
NEW_SRC_LINE="${SRC_LINE//$OLD_VER/$NEW_VER}"
DOWNLOAD_URL=$(echo "$NEW_SRC_LINE" | sed -E 's/.*source = //' | sed -E 's/.*:://' | xargs)

echo "   Downloading: $DOWNLOAD_URL"
NEW_HASH=$(curl -sL --fail "$DOWNLOAD_URL" | sha256sum | awk '{print $1}')
echo "   New Hash:    $NEW_HASH"

sed -i "s/^pkgver=.*/pkgver=$NEW_VER/" PKGBUILD
sed -i "s/^pkgrel=.*/pkgrel=1/" PKGBUILD
sed -i "s/$OLD_HASH/$NEW_HASH/" PKGBUILD

sed -i "s/pkgver = $OLD_VER/pkgver = $NEW_VER/" .SRCINFO
sed -i "s/pkgrel = .*/pkgrel = 1/" .SRCINFO

sed -i "s/$OLD_HASH/$NEW_HASH/" .SRCINFO
sed -i "/source =/s/$OLD_VER/$NEW_VER/g" .SRCINFO

echo ">> [Source] Done."
