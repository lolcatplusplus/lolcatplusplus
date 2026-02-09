#!/bin/bash
set -e
set -o pipefail

NEW_VER="$1"

if [ -z "$NEW_VER" ]; then
    echo "Usage: $0 <new_version>"
    exit 1
fi

echo ">> [Binary] Updating to $NEW_VER..."

OLD_VER=$(grep "^pkgver=" PKGBUILD | cut -d'=' -f2)

HASHES=($(grep -A 5 "^sha256sums=" PKGBUILD | grep -oP "(?<=').*(?=')"))
OLD_BIN_HASH=${HASHES[0]}
OLD_LIC_HASH=${HASHES[1]}

URLS=($(grep "source =" .SRCINFO | sed -E 's/.*source = //' | sed -E 's/.*:://' | xargs))
BIN_URL="${URLS[0]//$OLD_VER/$NEW_VER}"
LIC_URL="${URLS[1]//$OLD_VER/$NEW_VER}"

echo "   Fetching Binary: $BIN_URL"
NEW_BIN_HASH=$(curl -sL --fail "$BIN_URL" | sha256sum | awk '{print $1}')

echo "   Fetching License: $LIC_URL"
NEW_LIC_HASH=$(curl -sL --fail "$LIC_URL" | sha256sum | awk '{print $1}')

sed -i "s/^pkgver=.*/pkgver=$NEW_VER/" PKGBUILD
sed -i "s/^pkgrel=.*/pkgrel=1/" PKGBUILD
sed -i "s/$OLD_BIN_HASH/$NEW_BIN_HASH/" PKGBUILD
sed -i "s/$OLD_LIC_HASH/$NEW_LIC_HASH/" PKGBUILD

sed -i "s/pkgver = $OLD_VER/pkgver = $NEW_VER/" .SRCINFO
sed -i "s/pkgrel = .*/pkgrel = 1/" .SRCINFO
sed -i "s/$OLD_BIN_HASH/$NEW_BIN_HASH/" .SRCINFO
sed -i "s/$OLD_LIC_HASH/$NEW_LIC_HASH/" .SRCINFO
sed -i "/source =/s/$OLD_VER/$NEW_VER/g" .SRCINFO

echo ">> [Binary] Done."
