#!/bin/bash -ex
cd build/install
rm -fr Payload
mkdir Payload
cp -r Metaforce.app Payload
zip -r Metaforce.zip Payload
mv Metaforce.zip Metaforce.ipa
