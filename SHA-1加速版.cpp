#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <memory.h>
#include<Windows.h>
//初值，CV0,最后意思是结果
uint32_t H0 = 0x67452301;
uint32_t H1 = 0xEFCDAB89;
uint32_t H2 = 0x98BADCFE;
uint32_t H3 = 0x10325476;
uint32_t H4 = 0xC3D2E1F0;

//4个K常数
const uint32_t K0 = 0x5A827999;
const uint32_t K1 = 0x6ED9EBA1;
const uint32_t K2 = 0x8F1BBCDC;
const uint32_t K3 = 0xCA62C1D6;

void subround(uint32_t & A, uint32_t & B, uint32_t & C, uint32_t & D, uint32_t & E, uint32_t & W, uint32_t K, int mode);//0-79的轮函数

//四个非线性函数，直接手动内联了
inline uint32_t f1(uint32_t B, uint32_t C, uint32_t D);
inline uint32_t f2(uint32_t B, uint32_t C, uint32_t D);
inline uint32_t f3(uint32_t B, uint32_t C, uint32_t D);
inline uint32_t f4(uint32_t B, uint32_t C, uint32_t D);

inline uint32_t cirleft(uint32_t word, int bit);//word循环左移bit位，手动内联了
long long msgsize(char*plainaddr);//获得消息长度

int main()
{
	LARGE_INTEGER start, finish, frequency;//计时用
	QueryPerformanceFrequency(&frequency);
	double quadpart = (double)frequency.QuadPart;
	double elapsed = 0;


	char inputfileaddr[81] = "input.file";//输入
	char outputfile[81] = "digest.file";//输出
	uint32_t W[80];//用空间换时间，W块有80个
	memset(W, 0, 64);//原始的为0就好啦
	uint32_t A, B, C, D, E;
	long long msglen = msgsize(inputfileaddr) * 8;//获得原文位数，二进制位单位，不是字节！
	FILE *fp = NULL;
	if ((fp = fopen(inputfileaddr, "rb")) == NULL)
	{
		printf("open %s faied!\n", inputfileaddr);
		system("PAUSE");
		return 1;
	}
	long long counter = 1, times = 0;
	int flag = 0;
	int bytes;//最后一次到底读了多少字节
	if (msglen % 512 > 440)//大于440的意思就是到了448那就用了W[13]，W[14]和W[15]是长度，那就放不下0x80了
	{
		times = (msglen + 512 - 1) / 512;//向上取整
		flag = 1;
	}
	else if (msglen % 512 == 0)
	{
		times = (msglen + 512 - 1) / 512 + 1;//多做一次,因为造了一个块，后面加了一个块
		flag = 2;
	}
	else times = (msglen + 512 - 1) / 512;//至少要做那么多次，可能会多一次

	A = H0; B = H1; C = H2; D = H3; E = H4;
	while (counter < times)//一般情况少做最后一次，特殊情况少做最后两次
	{
		fread(W, sizeof(char), 64, fp);//读出一个消息块512bits,读到W里面
		for (int i = 0; i < 16; i++)//把顺序转过来,得到想要的存储顺序
		{
			W[i] = (W[i] >> 24) + (W[i] >> 8 & 0xff00) + (W[i] << 8 & 0xff0000) + (W[i] << 24);
		}
		QueryPerformanceCounter(&start);//计时		
		for (int i = 0; i < 20; i++)
		{
			if (i >= 16)//W[i]生成以后还要左移1位
				W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) | 
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16])>>31);
			subround(A, B, C, D, E, W[i], K0, 1);
		}

		for (int i = 20; i < 40; i++)
		{
			W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
			subround(A, B, C, D, E, W[i], K1, 2);
		}

		for (int i = 40; i < 60; i++)
		{
			W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
			subround(A, B, C, D, E, W[i], K2, 3);
		}

		for (int i = 60; i < 80; i++)
		{
			W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
			subround(A, B, C, D, E, W[i], K3, 4);
		}
		A= H0 = H0 + A;//加了以后继续CVi
		B= H1 = H1 + B;
		C= H2 = H2 + C;
		D= H3 = H3 + D;
		E= H4 = H4 + E;
		counter++;
		QueryPerformanceCounter(&finish);
		elapsed += (finish.QuadPart - start.QuadPart) / quadpart;//结束一次计时
	}
	memset(W, 0, 64);
	unsigned char*p;
	if (flag == 0 || flag == 1)
	{

		bytes = fread(W, sizeof(char), 64, fp);//读出一个消息块512bits,读到W里面
		QueryPerformanceCounter(&start);//计时	
		p = (unsigned char*)&W[0];
		p = p + bytes;
		*p = 0x80;
		p = (unsigned char*)&msglen;
		if (flag == 0)
		{
			memcpy(&W[14], p + 4, 4);//复制长度
			memcpy(&W[15], p, 4);//复制长度，要变一下存储顺序
			W[14] = (W[14] >> 24) + (W[14] >> 8 & 0xff00) + (W[14] << 8 & 0xff0000) + (W[14] << 24);
			W[15] = (W[15] >> 24) + (W[15] >> 8 & 0xff00) + (W[15] << 8 & 0xff0000) + (W[15] << 24);
		}
	}
	else if (flag == 2)
	{
		memset(W, 0, 64);
		p = (unsigned char*)&W[0];
		*p = 0x80;
		p = (unsigned char*)&msglen;
		memcpy(&W[14], p + 4, 4);//复制长度
		memcpy(&W[15], p, 4);//复制长度，要变一下存储顺序
		W[14] = (W[14] >> 24) + (W[14] >> 8 & 0xff00) + (W[14] << 8 & 0xff0000) + (W[14] << 24);
		W[15] = (W[15] >> 24) + (W[15] >> 8 & 0xff00) + (W[15] << 8 & 0xff0000) + (W[15] << 24);
	}
	for (int i = 0; i < 16; i++)//顺序永久转过来
	{
		W[i] = (W[i] >> 24) + (W[i] >> 8 & 0xff00) + (W[i] << 8 & 0xff0000) + (W[i] << 24);
	}
	A = H0; B = H1; C = H2; D = H3; E = H4;
	for (int i = 0; i < 20; i++)
	{
		if (i >= 16)
			W[i] = cirleft(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
		subround(A, B, C, D, E, W[i], K0, 1);
	}
	for (int i = 20; i < 40; i++)
	{
		W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
			((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
		subround(A, B, C, D, E, W[i], K1, 2);
	}
	for (int i = 40; i < 60; i++)
	{
		W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
			((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
		subround(A, B, C, D, E, W[i], K2, 3);
	}

	for (int i = 60; i < 80; i++)
	{
		W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
			((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
		subround(A, B, C, D, E, W[i], K3, 4);
	}
	 A=H0 = H0 + A;
	 B=H1 = H1 + B;
	 C=H2 = H2 + C;
	 D=H3 = H3 + D;
	 E=H4 = H4 + E;
	QueryPerformanceCounter(&finish);
	elapsed += (finish.QuadPart - start.QuadPart) / quadpart;//结束一次计时
	if (flag == 1)
	{
		QueryPerformanceCounter(&start);
		memset(W, 0, 64);
		p = (unsigned char*)&msglen;
		memcpy(&W[14], p + 4, 4);//复制长度
		memcpy(&W[15], p, 4);//复制长度
		//A = H0; B = H1; C = H2; D = H3; E = H4;
		for (int i = 0; i < 20; i++)
		{
			if (i >= 16)
				W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
			subround(A, B, C, D, E, W[i], K0, 1);
		}

		for (int i = 20; i < 40; i++)
		{
			W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
			subround(A, B, C, D, E, W[i], K1, 2);
		}

		for (int i = 40; i < 60; i++)
		{
			W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
			subround(A, B, C, D, E, W[i], K2, 3);
		}

		for (int i = 60; i < 80; i++)
		{
			W[i] = ((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) << 1) |
				((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]) >> 31);
			subround(A, B, C, D, E, W[i], K3, 4);
		}
		A=H0 = H0 + A;
		B=H1 = H1 + B;
		C=H2 = H2 + C;
		D=H3 = H3 + D;
		E=H4 = H4 + E;
		QueryPerformanceCounter(&finish);
		elapsed += (finish.QuadPart - start.QuadPart) / quadpart;//结束一次计时
	}
	FILE*output;
	if ((output = fopen(outputfile, "wb")) == NULL)
	{
		printf("open %s faied!\n", outputfile);
		system("PAUSE");
		return 1;
	}
	fprintf(output, "%08X%08X%08X%08X%08X", A,B,C,D,E);
	printf("%08X%08X%08X%08X%08X\n", A, B, C, D, E);
	fclose(output);
	fclose(fp);
	printf("the time for proceed is %f ms\n", elapsed * 1000);
	system("PAUSE");
	return 0;
}

void subround(uint32_t & A, uint32_t & B, uint32_t & C, uint32_t & D, uint32_t & E, uint32_t &W, uint32_t K, int mode)
{
	uint32_t t;
	switch (mode)
	{
	case 1:
		t = A;
		A = ((B&C) | ((~B)&D)) + ((A << 5) | (A >> 27)) + E + W + K;
		E = D;
		D = C;
		C = (B << 30) | (B >> 2);
		B = t;
		break;
	case 2:
		t = A;
		A = (B^C^D) + ((A << 5) | (A >> 27)) + E + W + K;
		E = D;
		D = C;
		C = (B << 30) | (B >> 2);
		B = t;
		break;
	case 3:
		t = A;
		A = ((B&C) | (B&D) | (C&D)) + ((A << 5) | (A >> 27)) + E + W + K;
		E = D;
		D = C;
		C = (B << 30) | (B >> 2);
		B = t;
		break;
	case 4:
		t = A;
		A = (B^C^D) + ((A << 5) | (A >> 27)) + E + W + K;
		E = D;
		D = C;
		C = (B << 30) | (B >> 2);
		B = t;
		break;
	default:
		break;
	}

}

inline uint32_t f1(uint32_t B, uint32_t C, uint32_t D)//手动内联了
{
	return(B&C) | ((~B)&D);
}

inline uint32_t f2(uint32_t B, uint32_t C, uint32_t D)
{
	return(B^C^D);
}

inline uint32_t f3(uint32_t B, uint32_t C, uint32_t D)
{
	return (B&C) | (B&D) | (C&D);
}

inline uint32_t f4(uint32_t B, uint32_t C, uint32_t D)
{
	return(B^C^D);
}

inline uint32_t cirleft(uint32_t word, int bit)//word循环左移bit位
{
	return(word << bit) | (word >> (32 - bit));
}

long long msgsize(char*plainaddr)
{
	long long size = 0;
	FILE *fp = NULL;
	if ((fp = fopen(plainaddr, "rb")) == NULL)
	{

		printf("open %s faied!", plainaddr);
		exit(0);
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fclose(fp);
	return size;
}
