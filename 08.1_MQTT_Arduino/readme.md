# README

## 服务器
    巴法云(bemfa.com)
## 协议
    mqtt TCP
## 注意
    mqtt tcp 服务器是巴法云(bemfa.com)的，esp自带的mqtt-client组件不能直接连接，我现在也不会修改，所以使用了esp的arduino组件，复制了巴法云的arduino代码实现mqtt的tcp连接，但是速度很慢

    巴法云的官方接入文档里好像说的挺简单的，应该也可以自己使用TCP发送字符串建立mqtt连接，应该快很多