# nbiot_sdk
OneNET NBIOT C SDK
## 目录
 * Support&Not Support
 * 编译SDK
 * 目录结构
 * 常见问题

## Support&Not Support
### Support
1. register
2. update
3. de-register
4. observe
5. cancel observe
6. discover
7. read
8. partial update write
9. execute
10. notify
### Not Support
1. bootstrap
2. replace write
3. create
4. delete
5. write attribute
6. coap block
 
## 编译SDK
### 通过CMAKE编译SDK
1. 生成调试版本：cmake -DCMAKE_BUILD_TYPE=Debug
2. 生成发布版本：cmake -DCMAKE_BUILD_TYPE=Release
3. 生成版本包含单元测试：cmake -DUNIT_TEST=1

### 自定义工程工程中编译
1. 将source、platforms目录及其子目录中的源码包含到自定义工程中
2. 包含以下头文件目录：
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
   + test            NBIOT SDK部分测试用例（基于GTest）
```
## 常见问题
1. NBIOT SDK对设备内存的最低需求是多少？
```
   随着数据流个数的增加，内存需求也会相应地增加
   例如：sample中对内存的需求分别是：（stack 1kb heap 512b）
```
