#include <iostream>
#include <chrono>
#include <inttypes.h>

#include "hwfx3/fx3dev.h"
#include "hwfx3/host_commands.h"

#ifdef WIN32
#include "hwfx3/fx3devcyapi.h"
#include <conio.h>
#endif

#include "processors/streamdumper.h"

using namespace std;

int main( int argn, const char** argv )
{
#ifndef WIN32
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    timeBeginPeriod(1);
#endif

    cerr << "*** SDDumper for nut4nt-fx3s board ***" << endl << endl;
    if ( argn != 3 ) {
        cerr << "Usage: "
             << "SDDumper"            << " "
             << "FX3_IMAGE"           << " "
             << "cypress | libusb"
             << endl << endl;

        cerr << "Example (dumping one minute of signal and use cypress driver):" << endl;
        cerr << "SDDumper AmungoItsFx3Firmware.img cypress" << endl;
        cerr << endl;

        return 0;
    }

    std::string fximg( argv[1] );
    std::string driver( argv[2] );

    bool useCypress = ( driver == string( "cypress" ) );

    cerr << "------------------------------" << endl;
    cerr << "Using fx3 image from '" << fximg << "'" << endl;
    cerr << "You choose to use __" << ( useCypress ? "cypress" : "libusb" ) << "__ driver" << endl;
    cerr << "------------------------------" << endl;

    cerr << "Wait while device is being initing..." << endl;
    FX3DevIfce* dev = nullptr;

#ifdef WIN32
    if ( useCypress ) {
        dev = new FX3DevCyAPI();
    } else {
        dev = new FX3Dev( 2 * 1024 * 1024, 7 );
    }
#else
    if ( useCypress ) {
        cerr << endl
             << "WARNING: you can't use cypress driver under Linux."
             << " Please check if you use correct scripts!"
             << endl;
    }
    dev = new FX3Dev( 2 * 1024 * 1024, 7 );
#endif

    if ( dev->init(fximg.c_str(), nullptr ) != FX3_ERR_OK ) {
        cerr << endl << "Problems with hardware or driver type" << endl;
        return -1;
    }
    cerr << "Device was inited." << endl << endl;
    dev->log = false;

    std::this_thread::sleep_for(chrono::milliseconds(500));

    CyU3PSibDevInfo_t sdinfo;
    dev->sendCommand( CMD_SDDEV0_INFO, &sdinfo, 32 );
    sdinfo.print();

    dev->sendCommand( CMD_SDDEV1_INFO, &sdinfo, 32 );
    sdinfo.print();

    thread poller;
    bool poller_running = true;
    bool device_is_ok = true;
    try {

        bool first_loop = true;
        poller = thread( [&]() {
            double lastWriteGb = 0.0;
            double lastSkipGb  = 0.0;

            while ( poller_running && device_is_ok ) {
                const double delta_sec = 5.0;
                this_thread::sleep_for(chrono::milliseconds(  (int)(delta_sec*1000.0) ));
                fx3_dev_debug_info_t info = dev->getDebugInfoFromBoard(false);
                //info.print();
                double allWriteGb;
                double allSkipGb;
                const double GB = (1024.0*1024.0*1024.0);
                double sector_size = info.data[10];

                double dmaSz = (double) info.data[ 4 ];
                double skipGb[2];
                skipGb[0] = dmaSz * (double)info.data[12] / GB;
                skipGb[1] = dmaSz * (double)info.data[15] / GB;

                double addrGb[2];
                addrGb[0] = info.data[13] * sector_size / GB;
                addrGb[1] = info.data[16] * sector_size / GB;

                double maxAddr[2];
                maxAddr[0] = info.data[14] * sector_size / GB;
                maxAddr[1] = info.data[17] * sector_size / GB;

                allWriteGb = addrGb[0] + addrGb[1];
                allSkipGb  = skipGb[0] + skipGb[1];
                double deltaWriteGb = allWriteGb - lastWriteGb;
                double deltaAllGb   = (allWriteGb + allSkipGb) - (lastWriteGb + lastSkipGb);
                double rcvSpeedMbps = 1024.0 * deltaAllGb   / delta_sec;
                double wrSpeedMbps  = 1024.0 * deltaWriteGb / delta_sec;

                if ( !first_loop ) {

                    fprintf( stderr, "SR %.1f, wr %.1f  ", rcvSpeedMbps, wrSpeedMbps );

                    for ( int i = 0; i < 2; i++ ) {
                        fprintf( stderr, "[%d] %.3f / %.3f (%.3f)  lost %6.3f%%     ",
                                 i, skipGb[i], addrGb[i], maxAddr[i], 100.0 * skipGb[i]/(addrGb[i]+skipGb[i]) );
                    }
                    fprintf( stderr, "\n" );

                } else {
                    first_loop = false;
                }

                lastWriteGb = allWriteGb;
                lastSkipGb  = allSkipGb;


            }
            cerr << "Poller thread finished" << endl;
        });

        while ( device_is_ok && !_kbhit() ) {
            std::this_thread::sleep_for(chrono::milliseconds(100));
        }
        cerr << endl;

        cerr << "Done" << endl;
    } catch ( std::exception& e ){
        cerr << endl << "Error!" << endl;
        cerr << e.what();
    }

    dev->changeHandler(nullptr);

    poller_running = false;
    if ( poller.joinable() ) {
        poller.join();
    }

    delete dev;

    return 0;
}

