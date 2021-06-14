#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <lv2/lv2.h>

typedef struct
{
    int mode;                   /*!<  encrypt/decrypt   */
    unsigned long sk[32];       /*!<  DES subkeys       */
} des_context;

void xtea_ctr(uint8_t *key, uint64_t nounce, uint8_t *buf, int size);

#endif /* __CRYPTO_H__ */
