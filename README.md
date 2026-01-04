# IOCFBootleg

`IOCFSerialize` and `IOCFUnserialize` for non-Apple platforms.  
Includes a very bootleg stub-out of CoreFoundation.

`IOCFSerialize.c` and `IOCFUnserialize.tab.c` were ripped out of [`IOKitUser-100222.40.4`](https://github.com/apple-oss-distributions/IOKitUser/releases/tag/IOKitUser-100222.40.4).

### Usage

`make` attempts to build a shared library for your native platform.  
Can be made to cross-compile by adjusting `CC` and `CFLAGS`.

See the `Makefile` for compiler flags if you want to directly include these files in your project.

### License

Mixed. A lot of code in this repo was ripped out of Apple's OSS releases, so that is APSL.

My own contributions (particularly `CoreFoundation.h`/`CoreFoundation.c`) can be treated as public domain.
