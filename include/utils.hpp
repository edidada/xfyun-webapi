#include <string>
#include <openssl/hmac.h>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

using namespace std;

/**
 * 获取当前时间戳，并格式化成rfc1123格式
 * 输入：
 * 输出：当前时间戳的rfc1123格式字符串。"Thu, 05 Dec 2019 09:54:17 CST"
 */
string get_time_rfc1123()
{
	time_t rawtime;
	time(&rawtime);
	char date[64];
	strftime(date, sizeof(date), "%a, %d %b %Y %X %Z\n", localtime(&rawtime));

	return string(date);
}

/**
 * hmac_sha256算法，利用key对data进行加密认证处理
 * 输入：data，待sha256的数据；key，mac的密钥
 * 输出：256位加密字符，为方便处理，输出时转换为了string。"e�FX��}U0���^V!D@�����`|v"
 */
string get_hmac_sha256(const string &data, const string &key)
{
	unsigned char *result = new unsigned char[40];
	unsigned int result_len = 0;

	HMAC_CTX *ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, key.c_str(), key.size(), EVP_sha256(), NULL);
	HMAC_Update(ctx, (unsigned char *)data.c_str(), data.size());
	HMAC_Final(ctx, result, &result_len);
	HMAC_CTX_free(ctx);

	// ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	// 不可以return string(result)，string的构造函数不接受unsigned char *
	// 不可以直接 return (char *)result
	// 因为result后面还有空余字符，加密的值要严格控制在result_len的长度范围内
	// ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	return string((char *)result, result_len);
}

/**
 * base64编码算法
 * 输入：data，待编码数据
 * 输出：data被base64编码后的字符串。"KuSCRlicg30QVTChBdToXlYhREDPxdkDiKNgfHYiDWU="
 */
string get_base64_encode(const string &data)
{
	typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<string::const_iterator, 6, 8>> Base64EncodeIterator;

	stringstream result;

	copy(Base64EncodeIterator(data.begin()), Base64EncodeIterator(data.end()), ostream_iterator<char>(result));
	size_t equal_count = (3 - data.length() % 3) % 3;
	for (size_t i = 0; i < equal_count; i++)
	{
		result.put('=');
	}

	return result.str();
}

/**
 * url编码算法，转义url中的特殊字符和中文字符等
 * 输入：url，待编码的url
 * 输出：被编码转义后的url
 */
string get_url_encode(const string &url)
{
	typedef unsigned char BYTE;
	string result;
	size_t len = url.length();
	unsigned char *pBytes = (unsigned char *)url.c_str();
	char szAlnum[2];
	char szOther[4];
	for (size_t i = 0; i < len; i++)
	{
		// 由于迅飞在解码url时以'='获得参数值，以'&'分割参数，故这两个参数不可转义
		if (isalnum((BYTE)url[i]) || url[i] == '&' || url[i] == '=')
		{
			sprintf(szAlnum, "%c", url[i]);
			result.append(szAlnum);
		}
		else
		{
			sprintf(szOther, "%%%X%X", pBytes[i] >> 4, pBytes[i] % 16);
			result.append(szOther);
		}
	}
	return result;
}

/**
 * 延迟函数，因为Windows和Linux下的延迟函数各异，开发者可以自定义
 * 输入：t，延迟秒数
 * 输出：
 */
void delay(double t)
{
	clock_t start_time = clock();
	while ((clock() - start_time) < t * CLOCKS_PER_SEC)
	{
	}
}