#ifndef FX3DEVDEBUGINFO
#define FX3DEVDEBUGINFO

#include <cstdint>
#include <cstdio>
#include <vector>

#include "fx3deverr.h"
#include "cyerrors.h"

struct fx3_dev_debug_info_t {
    fx3_dev_err_t status;
    std::vector<uint32_t> data;
    double size_tx_mb_inc;
    bool speed_only;

    fx3_dev_debug_info_t() : status( FX3_ERR_OK ), data(19), size_tx_mb_inc( 0.0 ), speed_only( true )
    {
        for ( auto& x : data ) {
            x = 0;
        }
    }

    void print() {
        fprintf( stderr, "info: " );
        int i = 0;
        for ( auto& x : data ) {
            if ( i <= 7 ) {
                fprintf( stderr, "%5u ", x );
            } else {
                fprintf( stderr, "0x%04X ", x );
            }
            i++;
        }
        fprintf( stderr, "\n" );
    }
    size_t dataSize8() {
        return data.size()*sizeof(uint32_t);
    }
};


struct CyU3PSibDevInfo_t
{
    uint32_t        cardType;           /**< Type of storage device connected on the S port. (Can be none if
                                             no device detected). */
    uint32_t        clkRate;            /**< Current operating clock frequency for the device. */
    uint32_t        numBlks;            /**< Number of blocks of the storage device. */
    uint32_t        eraseSize;          /**< The erase unit size in bytes for this device. */
    uint16_t        blkLen;             /**< Current block size setting for the device. */
    uint16_t        ccc;                /**< Card command classes (CCC) from the CSD register. */
    uint8_t         removable;          /**< Indicates if the storage device is a removable device. */
    uint8_t         writeable;          /**< Whether the storage device is write enabled. */
    uint8_t         locked;             /**< Identifies whether the storage device is password locked. */
    uint8_t         ddrMode;            /**< Whether DDR clock mode is being used for this device. */
    uint8_t         opVoltage;          /**< Current operating voltage setting for the device. */
    uint8_t         busWidth;           /**< Current bus width setting for the device. */
    uint8_t         numUnits;           /**< Number of boot LUNs & User LUNs present on this device. */

    void print() {
        if ( cardType == 1111 ) {
            fprintf( stderr, "DevInfo ERROR: 0x%02X ", cardType );
            if ( clkRate == CY_U3P_ERROR_INVALID_DEV ) {
                fprintf( stderr, "CY_U3P_ERROR_INVALID_DEV" );
            }
            fprintf( stderr, "\n" );

        } else {
            fprintf( stderr, "%10s = %u\n", "cardType", cardType );
            fprintf( stderr, "%10s = %u\n", "clkRate", clkRate );
            fprintf( stderr, "%10s = %u\n", "numBlks", numBlks );
            fprintf( stderr, "%10s = %u\n", "eraseSize", eraseSize );
            fprintf( stderr, "%10s = %u\n", "blkLen", blkLen );
            fprintf( stderr, "%10s = %u\n", "ccc", ccc );
            fprintf( stderr, "%10s = %u\n", "removable", removable );
            fprintf( stderr, "%10s = %u\n", "writeable", writeable );
            fprintf( stderr, "%10s = %u\n", "locked", locked );
            fprintf( stderr, "%10s = %u\n", "ddrMode", ddrMode );
            fprintf( stderr, "%10s = %u\n", "opVoltage", opVoltage );
            fprintf( stderr, "%10s = %u\n", "busWidth", busWidth );
            fprintf( stderr, "%10s = %u\n", "numUnits", numUnits );
        }
        fprintf( stderr, "\n" );
    }
};


#endif // FX3DEVDEBUGINFO

