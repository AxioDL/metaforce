#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

void Blowfish_encipher(uint32_t *xl, uint32_t *xr);
void Blowfish_decipher(uint32_t *xl, uint32_t *xr);
int64_t Blowfish_hash(const void* buf, size_t len);

#ifdef __cplusplus
}
#endif
