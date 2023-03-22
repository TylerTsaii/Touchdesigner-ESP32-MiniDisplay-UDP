# Touchdesigner-ESP32-MiniDisplay-UDP
使用了ESP32的双线程一个线程接收UDP控制震动，另一个线程复制显示屏幕画面。


采用硬件：
1.
1.4英寸172×320 IPS LCD高清显示屏
https://www.dfrobot.com.cn/goods-3628.html
2.DFRobot的ESP32开发板
https://www.dfrobot.com.cn/goods-3009.html


注意点：
1.需要注意引脚序列，每一个板子不同，这里的屏幕也不是标准的网上可以找到的屏幕，官方示例就给了几个，但不太好用。网上好找的源码都需要仔细阅读源码并修改。就很不方便。所以建议购买网上开源项目多的硬件
2.配置环境时，添加开发板管理器的url，官方给的https://download.dfrobot.top/FireBeetle/package_DFRobot_index.json;
在我们公司的网络里面没法用，需要将https改为http，去掉S。
