#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

void Blowfish_encipher(unsigned long *xl, unsigned long *xr);
void Blowfish_decipher(unsigned long *xl, unsigned long *xr);
int64_t Blowfish_hash(const void* buf, size_t len);

#ifdef __cplusplus
}
#endif
