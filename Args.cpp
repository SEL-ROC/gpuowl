// GpuOwL, a Mersenne primality tester. Copyright (C) 2017-2018 Mihai Preda.

#include "args.h"
#include "file.h"

#include <vector>
#include <cstring>
#include <cassert>

vector<string> getDevices();

bool Args::parse(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
      printf(R"(
Command line options:

-user <name>       : specify the user name.
-cpu  <name>       : specify the hardware name.
-time              : display kernel profiling information.
-fft <size>        : specify FFT size, such as: 5000K, 4M, +2, -1.
-block <value>     : PRP GEC block size. Default 400. Smaller block is slower but detects errors sooner.
-carry long|short  : force carry type. Short carry may be faster, but requires high bits/word.
-list fft          : display a list of available FFT configurations.
-tf <bit-offset>   : enable auto trial factoring before PRP. Pass 0 to bit-offset for default TF depth.
-device <N>        : select a specific device:
)");

      vector<string> devices = getDevices();
      for (int i = 0; i < int(devices.size()); ++i) {
        printf(" %d : %s\n", i, devices[i].c_str());
      }      
      return false;
    } else if (!strcmp(arg, "-list")) {
      if (i < argc - 1 && !strcmp(argv[++i], "fft")) {
        listFFT = true;
      } else {
        log("-list expects \"fft\"\n");
        return false;
      }              
    } else if (!strcmp(arg, "-precompiled")) {
      usePrecompiled = true;
    } else if (!strcmp(arg, "-fft")) {
      if (i < argc - 1) {
        string s = argv[++i];
        fftSize = atoi(s.c_str()) * ((s.back() == 'K') ? 1024 : ((s.back() == 'M') ? 1024 * 1024 : 1));
      } else {
        log("-fft expects <size>\n");
        return false;
      }
    } else if (!strcmp(arg, "-tf")) {
      if (i < argc - 1) {
        tfDelta = atoi(argv[++i]);
        enableTF = true;
      } else {
        log("-tf expects <bit-offset>\n");
        return false;
      }
    } else if (!strcmp(arg, "-dump")) {
      if (i < argc - 1 && argv[i + 1][0] != '-') {
        dump = argv[++i];
      } else {
        log("-dump expects name");
        return false;
      }
    } else if (!strcmp(arg, "-user")) {
      if (i < argc - 1) {
        user = argv[++i];
      } else {
        log("-user expects name\n");
        return false;
      }
    } else if (!strcmp(arg, "-cpu")) {
      if (i < argc - 1) {
        cpu = argv[++i];
      } else {
        log("-cpu expects name\n");
        return false;
      }
    } else if (!strcmp(arg, "-cl")) {
      if (i < argc - 1) {
        clArgs = argv[++i];
      } else {
        log("-cl expects options string to pass to CL compiler\n");
        return false;
      }
    } else if(!strcmp(arg, "-time")) {
      timeKernels = true;
    } else if (!strcmp(arg, "-carry")) {
      if (i < argc - 1) {
        std::string s = argv[++i];
        if (s == "short" || s == "long") {
          carry = s == "short" ? CARRY_SHORT : CARRY_LONG;
          continue;
        }
      }
      log("-carry expects short|long\n");
      return false;
    } else if (!strcmp(arg, "-block")) {
      if (i < argc - 1) {
        blockSize = atoi(argv[++i]);
        assert(blockSize > 0);
        if (10000 % blockSize) {
          log("Invalid blockSize %u, must divide 10000\n", blockSize);
          return false;
        }
        continue;
      } else {
        log("-block expects <value>\n");
        return false;
      }
    } else if (!strcmp(arg, "-device") || !strcmp(arg, "-d")) {
      if (i < argc - 1) {
        device = atoi(argv[++i]);
        /*
        int nDevices = getNumberOfDevices();
        if (device < 0 || device >= nDevices) {
          log("invalid -device %d (must be between [0, %d]\n", device, nDevices - 1);
          return false;
        }
        */
      } else {
        log("-device expects <N> argument\n");
        return false;
      }
    } else {
      log("Argument '%s' not understood\n", arg);
      return false;
    }
  }
  
  return true;
}
