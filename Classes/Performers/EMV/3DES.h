//
//  3DES.h
//  SmartCard
//
//  Created by Chenfan on 10/31/11.
//  Modified by Ra渓 Valencia on 17/12/12
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#ifndef	_3DES__H_
#define _3DES__H_

#ifdef __cplusplus
extern "C"
{
#endif
    
#define ENCRYPT	0
#define DECRYPT	1
    
#define ECB	0
#define CBC	1
    
#define KEY_LEN_8 8
#define KEY_LEN_16 16
#define KEY_LEN_24 24
    
#define PAD_ISO_1	0
#define PAD_ISO_2	1
#define PAD_PKCS_7	2
    
	typedef char (*PSubKey)[16][48];
    
	/*解密后,将填充的字符去掉*/
	int RunRsm(char *Text,int len);
    
	/*将48位的明文密钥转换成24的字符串密钥*/
	int	CovertKey(char* iKey, char *oKey);
    
	/*******************************************************************
     函 数 名 称:	RunPad
     功 能 描 述：	根据协议对加密前的数据进行填充
     参 数 说 明：	bType	:类型：PAD类型
     In		:数据串指针
     Out		:填充输出串指针
     in_len	:数据的长度
     padlen	:(in,out)输出buffer的长度，填充后的长度
     
     返回值 说明：	char	:是否填充成功
     *******************************************************************/
	int	RunPad(int nType,const char* In,unsigned in_len,char* Out,int* padlen);
    
	/*******************************************************************
     函 数 名 称:	Run1Des
     功 能 描 述：	执行单DES算法对文本加解密
     参 数 说 明：	bType	:类型：加密ENCRYPT，解密DECRYPT
     bMode	:模式：ECB,CBC
     In		:待加密串指针
     in_len	:待加密串的长度，同时Out的缓冲区大小应大于或者等于in_len
     Key		:密钥(可为8位,16位,24位)支持3密钥
     key_len	:密钥长度，多出24位部分将被自动裁减
     Out		:待输出串指针
     out_len :输出缓存大小
     cvecstr :8字节随即字符串
     
     返回值 说明：	int     :是否加密成功，1：成功，0：失败
     *******************************************************************/
	int Run1Des(int bType, int bMode, const char *In, unsigned int in_len, const char *Key, unsigned int key_len, char* Out, unsigned int out_len, const char cvecstr[8]);
    
	/*******************************************************************
     函 数 名 称:	Run3Des
     功 能 描 述：	执行3DES算法对文本加解密
     参 数 说 明：	bType	:类型：加密ENCRYPT，解密DECRYPT
     bMode	:模式：ECB,CBC
     In		:待加密串指针
     in_len	:待加密串的长度，同时Out的缓冲区大小应大于或者等于in_len
     Key		:密钥(可为8位,16位,24位)支持3密钥
     key_len	:密钥长度，多出24位部分将被自动裁减
     Out		:待输出串指针
     out_len :输出缓存大小
     cvecstr :8字节随即字符串
     
     返回值 说明：	int     :是否加密成功，1：成功，0：失败
     
     作       者:	huangjf
     更 新 日 期：	2009.6.17
     
     3DES(加密) = DES(key1, 加密) DES(key2, 解密)  DES(key3, 加密)
     3DES(解密) = DES(key3, 解密) DES(key2, 加密)  DES(key1, 解密)
     每个KEY为64位，总共可以有192位的KEY, 但一般都只使用128位的key
     如果只用128位密钥，则key3 = key1
     
     *******************************************************************/
	int Run3Des(int bType, int bMode, const char *In, unsigned int in_len, const char *Key, unsigned int key_len, char* Out, unsigned int out_len, const char cvecstr[8]);
    
	/*******************************************************************
     函 数 名 称:	Crypt3Des
     功 能 描 述：	实现3DES的加解密
     参 数 说 明：	type	:类型：加密ENCRYPT，解密DECRYPT
     in		:待加密串指针或者待解密的密码字符串指针
     Out		:加密后的密码或者解密后的明文
     Key		:密钥(48位的ASCII字符串)
     
     返回值 说明：
     0:成功
     -1:非法的密钥长度
     -2:密钥含有非16进制字符
     -3:明文填充失败
     -4:加解密失败
     -5:非法的操作类型
     *******************************************************************/
	int Crypt3Des(int type,char* in,char* key,char* out);
    
	char *Base64Encode(char *src, int srclen);
	char *Base64Decode(char *src);
	unsigned char GetByte(char *s);
    
#ifdef __cplusplus
}
#endif
#endif