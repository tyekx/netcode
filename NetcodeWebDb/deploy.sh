#!/bin/bash

pushd ~

if [ -d "eggengine" ]; then
    git pull
else
    git clone https://github.com/tyekx/eggengine.git
fi

pushd ~/www/netcode-game.eu

if [ -d "public_html" ]; then
    rm -rf public_html
fi

cp -R ~/eggengine/NetcodeWebFrontend public_html

rm -f public_html public_html/backend

cp -R ~/eggengine/NetcodeWebBackend public_html/backend

popd

popd
