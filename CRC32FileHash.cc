/*
 * CRC32FileHash.cc
 *
 *  Created on: 2014-10-20
 *      Author: MAC
 */

#include "CRC32FileHash.h"
#include "windows.h"

const UINT32 CRC32FileHash::kCRC32Table[] = {
	0x000000000L, 0x077073096L, 0x0EE0E612CL, 0x0990951BAL,
	0x0076DC419L, 0x0706AF48FL, 0x0E963A535L, 0x09E6495A3L,
	0x00EDB8832L, 0x079DCB8A4L, 0x0E0D5E91EL, 0x097D2D988L,
	0x009B64C2BL, 0x07EB17CBDL, 0x0E7B82D07L, 0x090BF1D91L,
	0x01DB71064L, 0x06AB020F2L, 0x0F3B97148L, 0x084BE41DEL,
	0x01ADAD47DL, 0x06DDDE4EBL, 0x0F4D4B551L, 0x083D385C7L,
	0x0136C9856L, 0x0646BA8C0L, 0x0FD62F97AL, 0x08A65C9ECL,
	0x014015C4FL, 0x063066CD9L, 0x0FA0F3D63L, 0x08D080DF5L,
	0x03B6E20C8L, 0x04C69105EL, 0x0D56041E4L, 0x0A2677172L,
	0x03C03E4D1L, 0x04B04D447L, 0x0D20D85FDL, 0x0A50AB56BL,
	0x035B5A8FAL, 0x042B2986CL, 0x0DBBBC9D6L, 0x0ACBCF940L,
	0x032D86CE3L, 0x045DF5C75L, 0x0DCD60DCFL, 0x0ABD13D59L,
	0x026D930ACL, 0x051DE003AL, 0x0C8D75180L, 0x0BFD06116L,
	0x021B4F4B5L, 0x056B3C423L, 0x0CFBA9599L, 0x0B8BDA50FL,
	0x02802B89EL, 0x05F058808L, 0x0C60CD9B2L, 0x0B10BE924L,
	0x02F6F7C87L, 0x058684C11L, 0x0C1611DABL, 0x0B6662D3DL,
	0x076DC4190L, 0x001DB7106L, 0x098D220BCL, 0x0EFD5102AL,
	0x071B18589L, 0x006B6B51FL, 0x09FBFE4A5L, 0x0E8B8D433L,
	0x07807C9A2L, 0x00F00F934L, 0x09609A88EL, 0x0E10E9818L,
	0x07F6A0DBBL, 0x0086D3D2DL, 0x091646C97L, 0x0E6635C01L,
	0x06B6B51F4L, 0x01C6C6162L, 0x0856530D8L, 0x0F262004EL,
	0x06C0695EDL, 0x01B01A57BL, 0x08208F4C1L, 0x0F50FC457L,
	0x065B0D9C6L, 0x012B7E950L, 0x08BBEB8EAL, 0x0FCB9887CL,
	0x062DD1DDFL, 0x015DA2D49L, 0x08CD37CF3L, 0x0FBD44C65L,
	0x04DB26158L, 0x03AB551CEL, 0x0A3BC0074L, 0x0D4BB30E2L,
	0x04ADFA541L, 0x03DD895D7L, 0x0A4D1C46DL, 0x0D3D6F4FBL,
	0x04369E96AL, 0x0346ED9FCL, 0x0AD678846L, 0x0DA60B8D0L,
	0x044042D73L, 0x033031DE5L, 0x0AA0A4C5FL, 0x0DD0D7CC9L,
	0x05005713CL, 0x0270241AAL, 0x0BE0B1010L, 0x0C90C2086L,
	0x05768B525L, 0x0206F85B3L, 0x0B966D409L, 0x0CE61E49FL,
	0x05EDEF90EL, 0x029D9C998L, 0x0B0D09822L, 0x0C7D7A8B4L,
	0x059B33D17L, 0x02EB40D81L, 0x0B7BD5C3BL, 0x0C0BA6CADL,
	0x0EDB88320L, 0x09ABFB3B6L, 0x003B6E20CL, 0x074B1D29AL,
	0x0EAD54739L, 0x09DD277AFL, 0x004DB2615L, 0x073DC1683L,
	0x0E3630B12L, 0x094643B84L, 0x00D6D6A3EL, 0x07A6A5AA8L,
	0x0E40ECF0BL, 0x09309FF9DL, 0x00A00AE27L, 0x07D079EB1L,
	0x0F00F9344L, 0x08708A3D2L, 0x01E01F268L, 0x06906C2FEL,
	0x0F762575DL, 0x0806567CBL, 0x0196C3671L, 0x06E6B06E7L,
	0x0FED41B76L, 0x089D32BE0L, 0x010DA7A5AL, 0x067DD4ACCL,
	0x0F9B9DF6FL, 0x08EBEEFF9L, 0x017B7BE43L, 0x060B08ED5L,
	0x0D6D6A3E8L, 0x0A1D1937EL, 0x038D8C2C4L, 0x04FDFF252L,
	0x0D1BB67F1L, 0x0A6BC5767L, 0x03FB506DDL, 0x048B2364BL,
	0x0D80D2BDAL, 0x0AF0A1B4CL, 0x036034AF6L, 0x041047A60L,
	0x0DF60EFC3L, 0x0A867DF55L, 0x0316E8EEFL, 0x04669BE79L,
	0x0CB61B38CL, 0x0BC66831AL, 0x0256FD2A0L, 0x05268E236L,
	0x0CC0C7795L, 0x0BB0B4703L, 0x0220216B9L, 0x05505262FL,
	0x0C5BA3BBEL, 0x0B2BD0B28L, 0x02BB45A92L, 0x05CB36A04L,
	0x0C2D7FFA7L, 0x0B5D0CF31L, 0x02CD99E8BL, 0x05BDEAE1DL,
	0x09B64C2B0L, 0x0EC63F226L, 0x0756AA39CL, 0x0026D930AL,
	0x09C0906A9L, 0x0EB0E363FL, 0x072076785L, 0x005005713L,
	0x095BF4A82L, 0x0E2B87A14L, 0x07BB12BAEL, 0x00CB61B38L,
	0x092D28E9BL, 0x0E5D5BE0DL, 0x07CDCEFB7L, 0x00BDBDF21L,
	0x086D3D2D4L, 0x0F1D4E242L, 0x068DDB3F8L, 0x01FDA836EL,
	0x081BE16CDL, 0x0F6B9265BL, 0x06FB077E1L, 0x018B74777L,
	0x088085AE6L, 0x0FF0F6A70L, 0x066063BCAL, 0x011010B5CL,
	0x08F659EFFL, 0x0F862AE69L, 0x0616BFFD3L, 0x0166CCF45L,
	0x0A00AE278L, 0x0D70DD2EEL, 0x04E048354L, 0x03903B3C2L,
	0x0A7672661L, 0x0D06016F7L, 0x04969474DL, 0x03E6E77DBL,
	0x0AED16A4AL, 0x0D9D65ADCL, 0x040DF0B66L, 0x037D83BF0L,
	0x0A9BCAE53L, 0x0DEBB9EC5L, 0x047B2CF7FL, 0x030B5FFE9L,
	0x0BDBDF21CL, 0x0CABAC28AL, 0x053B39330L, 0x024B4A3A6L,
	0x0BAD03605L, 0x0CDD70693L, 0x054DE5729L, 0x023D967BFL,
	0x0B3667A2EL, 0x0C4614AB8L, 0x05D681B02L, 0x02A6F2B94L,
	0x0B40BBE37L, 0x0C30C8EA1L, 0x05A05DF1BL, 0x02D02EF8DL
};

void CRC32FileHash::Initialize()
{
	hash_ = 0;
	hash_ = ~hash_;
}

void CRC32FileHash::Update(UINT32 bytecount)
{
	PBYTE buffer = buffer_;
	while (bytecount > 0)
	{
		hash_ = kCRC32Table[(hash_ ^ *buffer) & 0xFF] ^ (hash_ >> 8);
		++buffer;
		--bytecount;
	}
}

void CRC32FileHash::Finalize()
{
	hash_ = ~hash_;
}

void CRC32FileHash::ConvertHashToDigestString()
{
	char charbuffer[9];
	wsprintf(charbuffer, "%08X", hash_);
	digest_.append(charbuffer);
}
