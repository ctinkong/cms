# cms
cms is an industrial-strength live streaming server,support rtmp,http-flv,hls.in the future,it will support more protocol.
# Building cms with make new version number
```shell
git clone https://github.com/ctinkong/cms.git
cd cms
./configure
```
# Building cms not change version number
```shell
git clone https://github.com/ctinkong/cms.git
cd cms
make
```
# Notice
```shell
to build cms needs s2n,whitch supports TLS/SSL protocols.see https://github.com/awslabs/s2n
and current version,s2n is builded with openssl-1.1.1b.
```
