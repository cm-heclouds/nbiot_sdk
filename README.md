# nbiot_sdk
OneNET NBIOT C SDK
## 目录
 * 编译SDK
 * 目录结构
 * 常见问题
 
## 编译SDK
### 通过CMAKE编译SDK
1. 生成调试版本：cmake -DCMAKE_BUILD_TYPE=Debug
2. 生成发布版本：cmake -DCMAKE_BUILD_TYPE=Release
3. 生成版本包含单元测试：cmake -DUNIT_TEST=1

### 自定义工程工程中编译
1. 将wakaama、source、platforms目录及其子目录中的源码包含到自定义工程中
2. 包含以下头文件目录：
   * nbiot_sdk/wakaama/core
   * nbiot_sdk/include
3. 修改include/config.h中的配置信息，以符合自定义工程的需求

## 目录结构
```
nbiot_sdk
   + doc             sdk使用说明以及移植说明文档等
   + include         NBIOT SDK API相关头文件
   + platforms       平台相关的接口（include/platform.h;include/utils.h）实现
       + posix       支持posix的系统的相关接口实现
       + win         windows系统的相关接口实现
   + sample          NBIOT SDK使用示例
   + source          NBIOT SDK内部实现
       + dtls        dtls实现（通过tinydtls裁剪而来）
       + lwm2m       lwm2m、coap实现（通过wakaama裁剪而来）
   + test            NBIOT SDK部分测试用例（基于GTest）
   + tinydtls        eclipse tinydtls（https://sourceforge.net/p/tinydtls/code/ci/master/tree/）
   + wakaama         eclipse wakaama（https://github.com/eclipse/wakaama）
```
## 常见问题
1. NBIOT SDK对设备内存的最低需求是多少？
```
   随着数据流个数的增加，内存需求也会相应地增加
   例如：sample中对内存的需求分别是：（无dtls：stack 1kb heap 1kb）、（dtls：stack 1kb heap 2.5kb）
```
