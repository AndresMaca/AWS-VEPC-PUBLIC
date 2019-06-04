#!/bin/bash
sudo apt-get -y update 
sudo apt-get install -y build-essential
sudo apt-get install -y libsnappy-dev
sudo apt-get install -y git 
wget https://github.com/google/leveldb/archive/v1.20.tar.gz
tar xvf v1.20.tar.gz
rm -f v1.20.tar.gz
cd leveldb-1.20/
sudo apt-get install -y make
make
sudo scp -r out-static/lib* out-shared/lib* "/usr/local/lib"
cd include
sudo scp -r leveldb /usr/local/include
sudo ldconfig
pwd

