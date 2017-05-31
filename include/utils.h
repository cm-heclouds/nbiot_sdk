/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef ONENET_UTILS_H_
#define ONENET_UTILS_H_

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 字符串长度
 * @param 指向字符串的内存
 * @return 返回字符串长度
**/
int nbiot_strlen( const char *str );

/**
 * 拷贝字符串
 * @param dest 指向目标字符串的内存
 *        src  指向源字符串的内存
 *        size 拷贝字符个数，-1表示全拷贝
 * @return 返回拷贝字符总数
**/
int nbiot_strncpy( char       *dest,
                   const char *src,
                   int         size );

/**
 * 字符串比较
 * @param str1 指向字符串1的内存
 *        str2 指向字符串2的内存
 *        size 比较字符个数，-1表示全比较
 * @return str1 < str2 返回负值
 *         str1 = str2 返回0
 *         str1 > str2 返回正值
**/
int nbiot_strncmp( const char *str1,
                   const char *str2,
                   int         size );

/**
 * 创建新的字符串
 * @param str  指向源字符串的内存
 *        size 字符串长度，-1表示整个字符串
 * @return 返回新字符串的内存指针
**/
char* nbiot_strdup( const char *str,
                    int         size );

/**
 * 反向查找字符所在位置
 * @param str  指向源字符串的内存
 *        size 源字符串长度,-1表示整个字符串
 *        ch   指定字符
 * @return 查找成功，返回字符的指针，否则返回NULL
**/
const char* nbiot_strrchr( const char *str,
                           int         size,
                           char        ch );

/**
 * 字符串转换成整数
 * @param str 源字符串
 *        len 转换字符个数，-1表示整个字符串
 * @return 返回整数
**/
int nbiot_atoi( const char *str,
                int         size );

/**
 * 整数转换成字符串
 * @param val 整数
 *        str 指向目标字符串内存
 *        len 字符串内存大小，-1表示足够内存大小
 * @return 返回转换字符总数
**/
int nbiot_itoa( int   val,
                char *str,
                int   size );

/**
 * 拷贝内存块
 * @param dst  指向目标内存块
 *        src  指向源内存块
 *        size 拷贝总字节数
 * @return 返回目标内存块指针
**/
void* nbiot_memmove( void       *dst,
                     const void *src,
                     size_t      size );

/**
 * 内存块比较
 * @param mem1 指向内存块1
 *        mem2 指向内存块2
 *        size 比较总字节数
 * @return mem1 > mem2 返回正数
 *         mem1 = mem2 返回0
 *         mem1 < mem2 返回负数
**/
int nbiot_memcmp( const void *mem1,
                  const void *mem2,
                  size_t      size );

/**
 * 内存块内容置0
 * @param mem  指向目标内存块
 *        size 内存块的总字节数
**/
void nbiot_memzero( void  *mem,
                    size_t size );

/**
 * 生成随机数
 * @return 返回随机整数
**/
int nbiot_rand( void );

#ifdef NBIOT_DEBUG
/**
 * 检查指定字符是否是space
 * @param ch 指定字符
 * @return 是space返回非0值，否则返回0值
**/
int nbiot_isspace( int ch );

/**
* 检查指定字符是否可打印
* @param ch 指定字符
* @return 可打印返回非0值，否则返回0值
**/
int nbiot_isprint( int ch );

/**
 * 格式化输出函数
 * @param format 格式化字符串
**/
void nbiot_printf( const char *format, ... );

/**
 * 格式化打印内存块信息
 * @param buf 指向内存块
 *        len 内存块大小
**/
void nbiot_buffer_printf( const void *buf,
                          size_t      len );
#endif

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* ONENET_UTILS_H_ */
