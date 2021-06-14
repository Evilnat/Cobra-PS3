#ifndef SAVEGAMES_H__
#define SAVEGAMES_H__

#define AUTOSIGN						0
#define ACCOUNTID						1

#define READ 							0
#define WRITE 							1

#define HEADER_SIZE 					0x60
#define Y_TABLE_OFFSET  				0x7B60

#define COPY_PROTECTION_OFFSET 			0x150
#define USER_ID_1_OFFSET 				0x570
#define USER_ID_2_OFFSET  				0x584

#define XREGISTRY_FILE 					"/dev_flash2/etc/xRegistry.sys"

#define LOGINUSERID 					"/setting/user/lastLoginUserId"
#define SETTING_AUTOSIGN 				"/setting/user/%08d/npaccount/autoSignInEnable"
#define SETTING_ACCOUNTID 				"/setting/user/%08d/npaccount/accountid"

uint8_t account_id[0x10];
uint32_t userID;

void sha1_hmac(const unsigned char *key, size_t keylen, const unsigned char *input, size_t ilen, unsigned char output[20]);
int patch_savedata(const char *path);
int xreg_data(char *value, int type, int mode, int overwrite, int checkEmpty);
int set_fakeID(int userid, int overwrite);

#endif /* SAVEGAMES_H__ */
