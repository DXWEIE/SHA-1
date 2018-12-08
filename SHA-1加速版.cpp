#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <memory.h>
#include<Windows.h>
//��ֵ��CV0,�����˼�ǽ��
uint32_t H0 = 0x67452301;
uint32_t H1 = 0xEFCDAB89;
uint32_t H2 = 0x98BADCFE;
uint32_t H3 = 0x10325476;
uint32_t H4 = 0xC3D2E1F0;

//4��K����
const uint32_t K0 = 0x5A827999;
const uint32_t K1 = 0x6ED9EBA1;
const uint32_t K2 = 0x8F1BBCDC;
const uint32_t K3 = 0xCA62C1D6;

void subround(uint32_t & A, uint32_t & B, uint32_t & C, uint32_t & D, uint32_t & E, uint32_t & W, uint32_t K, int mode);//0-79���ֺ���

//�ĸ������Ժ�����ֱ���ֶ�������
inline uint32_t f1(uint32_t B, uint32_t C, uint32_t D);
inline uint32_t f2(uint32_t B, uint32_t C, uint32_t D);
inline uint32_t f3(uint32_t B, uint32_t C, uint32_t D);
inline uint32_t f4(uint32_t B, uint32_t C, uint32_t D);

inline uint32_t cirleft(uint32_t word, int bit);//wordѭ������bitλ���ֶ�������
long long msgsize(char*plainaddr);//�����Ϣ����

int main()
{
	LARGE_INTEGER start, finish, frequency;//��ʱ��
	QueryPerformanceFrequency(&frequency);
	double quadpart = (double)frequency.QuadPart;
	double elapsed = 0;


	char inputfileaddr[81] = "input.file";//����
	char outputfile[81] = "digest.file";//���
	uint32_t W[80];//�ÿռ任ʱ�䣬W����80��
	memset(W, 0, 64);//ԭʼ��Ϊ0�ͺ���
	uint32_t A, B, C, D, E;
	long long msglen = msgsize(inputfileaddr) * 8;//���ԭ��λ����������λ��λ�������ֽڣ�
	FILE *fp = NULL;
	if ((fp = fopen(inputfileaddr, "rb")) == NULL)
	{
		printf("open %s faied!\n", inputfileaddr);
		system("PAUSE");
		return 1;
	}
	long long counter = 1, times = 0;
	int flag = 0;
	int bytes;//���һ�ε��׶��˶����ֽ�
	if (msglen % 512 > 440)//����440����˼���ǵ���448�Ǿ�����W[13]��W[14]��W[15]�ǳ��ȣ��ǾͷŲ���0x80��
	{
		times = (msglen + 512 - 1) / 512;//����ȡ��
		flag = 1;
	}
	else if (msglen % 512 == 0)
	{
		times = (msglen + 512 - 1) / 512 + 1;//����һ��,��Ϊ����һ���飬�������һ����
		flag = 2;
	}
	else times = (msglen + 512 - 1) / 512;//����Ҫ����ô��Σ����ܻ��һ��

	A = H0; B = H1; C = H2; D = H3; E = H4;
	while (counter < times)//һ������������һ�Σ�������������������
	{
		fread(W, sizeof(char), 64, fp);//����һ����Ϣ��512bits,����W����
		for (int i = 0; i < 16; i++)//��˳��ת����,�õ���Ҫ�Ĵ洢˳��
		{
			W[i] = (W[i] >> 24) + (W[i] >> 8 & 0xff00) + (W[i] << 8 & 0xff0000) + (W[i] << 24);
		}
		QueryPerformanceCounter(&start);//��ʱ		
		for (int i = 0; i < 20; i++)
		{
			if (i >= 16)//W[i]�����Ժ�Ҫ����1λ
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
		A= H0 = H0 + A;//�����Ժ����CVi
		B= H1 = H1 + B;
		C= H2 = H2 + C;
		D= H3 = H3 + D;
		E= H4 = H4 + E;
		counter++;
		QueryPerformanceCounter(&finish);
		elapsed += (finish.QuadPart - start.QuadPart) / quadpart;//����һ�μ�ʱ
	}
	memset(W, 0, 64);
	unsigned char*p;
	if (flag == 0 || flag == 1)
	{

		bytes = fread(W, sizeof(char), 64, fp);//����һ����Ϣ��512bits,����W����
		QueryPerformanceCounter(&start);//��ʱ	
		p = (unsigned char*)&W[0];
		p = p + bytes;
		*p = 0x80;
		p = (unsigned char*)&msglen;
		if (flag == 0)
		{
			memcpy(&W[14], p + 4, 4);//���Ƴ���
			memcpy(&W[15], p, 4);//���Ƴ��ȣ�Ҫ��һ�´洢˳��
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
		memcpy(&W[14], p + 4, 4);//���Ƴ���
		memcpy(&W[15], p, 4);//���Ƴ��ȣ�Ҫ��һ�´洢˳��
		W[14] = (W[14] >> 24) + (W[14] >> 8 & 0xff00) + (W[14] << 8 & 0xff0000) + (W[14] << 24);
		W[15] = (W[15] >> 24) + (W[15] >> 8 & 0xff00) + (W[15] << 8 & 0xff0000) + (W[15] << 24);
	}
	for (int i = 0; i < 16; i++)//˳������ת����
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
	elapsed += (finish.QuadPart - start.QuadPart) / quadpart;//����һ�μ�ʱ
	if (flag == 1)
	{
		QueryPerformanceCounter(&start);
		memset(W, 0, 64);
		p = (unsigned char*)&msglen;
		memcpy(&W[14], p + 4, 4);//���Ƴ���
		memcpy(&W[15], p, 4);//���Ƴ���
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
		elapsed += (finish.QuadPart - start.QuadPart) / quadpart;//����һ�μ�ʱ
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

inline uint32_t f1(uint32_t B, uint32_t C, uint32_t D)//�ֶ�������
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

inline uint32_t cirleft(uint32_t word, int bit)//wordѭ������bitλ
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
