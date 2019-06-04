#!/bin/bash

sudo apt-get -y  update
sudo apt-get -y  -y  upgrade
sudo apt-get -y  install openvpn
sudo apt-get -y  install libsctp-dev
sudo apt-get -y  install openssl
sudo add-apt-repository -y "ppa:patrickdk/general-lucid"
sudo apt-get -y  update
sudo apt-get -y  install iperf3
sudo apt-get -y  install iperf
sudo apt-get -y  install htop
sudo apt-get -y  install ipvsadm
sudo apt-get -y  install git
sudo apt-get -y  install libssl-dev
sudo apt-get -y  install g++
sudo apt-get -y  install libboost-all-dev
cd ../NFV_LTE_EPC/NFV_LTE_EPC-2.0/KeyValueStore/Implementation/LevelDB/server/
sudo bash install_server.sh
cd ../client/src/
sudo apt-get -y  install make
make
sudo make install
echo "COMPLETED"


// sudo apt install build-essential