# cms
cms is an industrial-strength live streaming server,support rtmp,http-flv,hls and so on.
#Building cms with make new version number
```shell
git clone https://github.com/ctinkong/cms.git
cd cms
./configure
```
#Building cms not change version number
```shell
git clone https://github.com/ctinkong/cms.git
cd cms
make
```
#Notice
```shell
build cms need s2n lib,see https://github.com/awslabs/s2n
and current version,s2n builded with openssl-1.1.1b
```
