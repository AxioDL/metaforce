#include "CPlayerState.hpp"
#include "IOStreams.hpp"

namespace urde
{
/* TODO: Implement this properly */
/* NOTE: This is only to be used as a reference,
 * and is not indicative of how the actual format is structured
 * a proper RE is still required!
 */

const u32 CPlayerState::PowerUpMaxes[41] =
{   1, 1, 1, 1, 250, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 14,  1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const char* PowerUpNames[41]=
{
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Missiles",
    "Unknown",
    "Unknown",
    "Power Bombs",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Energy Tanks",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
};

CPlayerState::CPlayerState(CBitStreamReader& in)
    : x188_staticIntf(5)
{
    x0_24_ = true;
    in.readUint32Big();
    in.readBool();
    in.readBool();
    in.readBool();

    atInt8 data[0xAE];
    in.readBytesToBuf(data, 0xAE);
    for (u32 k = 0; k < 3; k++)
    {
        printf("Game %i\n", k+1);
        atInt8 save[0x3AC];
        in.readBytesToBuf(save, 0x3AC);
        {
            CBitStreamReader stream(save, 0x3AC);

            printf("Game State\n");
            for (u32 i = 0; i < 0x80; i++)
                printf("%i\n", stream.ReadEncoded(8));

            s32 tmp = stream.ReadEncoded(32);
            printf("%i\n", tmp);
            tmp = stream.ReadEncoded(1);
            printf("%i\n", tmp);
            tmp = stream.ReadEncoded(1);
            printf("%i\n", tmp);
            tmp = stream.ReadEncoded(32);
            printf("%f\n", *reinterpret_cast<float*>(&tmp));
            tmp = stream.ReadEncoded(32);
            printf("%f\n", *reinterpret_cast<float*>(&tmp));
            tmp = stream.ReadEncoded(32);
            printf("%x\n", tmp);

            printf("PlayerState\n");
            x4_ = stream.ReadEncoded(0x20);
            printf("%x\n", tmp);
            tmp = stream.ReadEncoded(0x20);
            printf("Base health %f\n", *reinterpret_cast<float*>(&tmp));
            xc_baseHealth = *reinterpret_cast<float*>(&tmp);
            x8_ = stream.ReadEncoded(CBitStreamReader::GetBitCount(5));
            printf("%i\n", x8_);
            x20_ = stream.ReadEncoded(CBitStreamReader::GetBitCount(4));
            printf("%i\n", x20_);
            x24_powerups.resize(41);
            printf("Powerups\n");
            for (u32 i = 0; i < x24_powerups.size(); ++i)
            {
                if (PowerUpMaxes[i] == 0)
                    continue;

                u32 a = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxes[i]));
                u32 b = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxes[i]));
                x24_powerups[i] = CPowerUp(a, b);
                printf("%2i(%15s): a=%i b=%i\n", i, PowerUpNames[i], a, b);
            }

            for (u32 i = 0; i < 0x34e; i++)
            {
                u32 tmp = stream.ReadEncoded(1);
                printf("%i\n", tmp);
            }

            tmp = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
            printf("%i\n", tmp);
            tmp = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
            printf("%i\n", tmp);
        }
    }
}

}
