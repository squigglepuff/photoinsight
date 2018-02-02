#ifndef IMAGELOADER_H
#define IMAGELOADER_H

// C++14 headers.
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cassert>
#include <memory>
#include <vector>

// FreeImage headers.
#include <FreeImage.h>

// Custom types.
#if !defined(_HAS_CUSTOM_TYPES)
#define _HAS_CUSTOM_TYPES
typedef unsigned char byte; //!< Exactly 1 byte of data.
typedef unsigned short u16; //!< 16-bits representing an integer.
typedef unsigned int u32; //!< 32-bits representing an integer.
typedef unsigned long long u64; //!< 64-bits representing and integer.
typedef std::string String; //!< Synonymous for std::string (ease of use and reading).
#endif //#if !defined(_HAS_CUSTOM_TYPES)

#if !defined(_HAS_SIZE_MACRO)
#define _HAS_SIZE_MACRO
#define SZ(x) (sizeof(x)/sizeof(*(x)))
#endif //#if !defined(_HAS_SIZE_MACRO)

/*!
 * \brief Image loading class
 *
 * This class provides a simple and intuitive way to load most image file formats via both the internal QT systems and FreeImage (system-installed).
 * The class doesn't paint any images nor will it save them, it will simply open existing files and return the image data contained within in a readable
 * format for something such as OpenGL, QImage, DirectX, etc.
 * The constructor of this class does NOT initialize anything, it simply puts all of it's members into an initial sane state. The client MUST call the "InitLoader()" function
 * before doing work. Failure to do so will result in an invalid/false return and the internal error code being set to "Error_NoInit".
 */
class CImageLoader
{
public:
    /*!
     * \brief Internal error code enum for CImageLoader.
     *
     * This enum is internal to this class and is ONLY accessible via the opaque type "Error_t". The enum defines all the possible errors that the class handles internally and
     * reports to the client application/library.
     */
    enum Error
    {
        // Generic errors.
        Error_Success = 0x00000000,
        Error_NoInit = 0xCC000001,
        Error_OutOfMemory = 0xCC000002,
        Error_BadFileDescriptor = 0xCC000003,
        Error_FileCorrupt = 0xCC000004,
        Error_BadPath = 0xCC000005,

        // Freeimage related errors.
        Error_FI_InitFail = 0xFF000000,
        Error_FI_LoadFail = 0xFF000001,
        Error_FI_UnknownType = 0xFF000002,
        Error_FI_BadPixelFormat = 0xFF000003,

        // Unknown/Extra errors.
        Error_Unknown = 0xffccffcc
    };

    /*!
     * \brief Internal pixel order/color scheme enum for CImageLoader.
     *
     * This enum defines the currently handled pixel-order/color schemes for image formats. These are only accessible via the opaque type "PixelFormat_t" and it's
     * REQUIRED that the client application/library translates these to the native drawing API equivilent.
     */
    enum PixelOrder
    {
        PixelOrder_RGB16,
        PixelOrder_RGB,
        PixelOrder_RGBA,
        PixelOrder_BGR16,
        PixelOrder_BGR,
        PixelOrder_BGRA,
        PixelOrder_CYMK,
        PixelOrder_Grayscale
    };

    CImageLoader();
    CImageLoader(const CImageLoader& aCls);
    ~CImageLoader();

    std::ostream& operator<<(std::ostream& aOut);

    bool InitLoader();

    void ClearError();
    Error GetLastError();
    String GetErrorString(Error aErr = Error_Success);

    bool ClearImage();
    bool LoadImage(String aPath);

    std::vector<byte> GetData();
    const byte* GetRawData();
    u32 GetDataSize();

    // Misc. information.
    String GetMime();
    u32 GetWidth();
    u32 GetHeight();
    u32 GetBitDepth();
    float GetAspectRatio();
    PixelOrder GetPixelOrder();

private:
    void ResetState();
    void SetError(Error aErr);
    Error InitFreeimage();

    bool mbInitialized;
    Error meInternalError;
    std::vector<byte> mvInternalBuffer;
    u32 miWidth;
    u32 miHeight;
    u32 miBPP;
    PixelOrder mePixelOrder;
    String msMimeType;
    FIBITMAP* mpImage;
};

/*
 * 0 = Fatal
 * 1 = Critical/Error
 * 2 = Warning
 * 3 = Information
 * 4 = Debug
 * 5 = Trace
 */
extern void LogMessage(String asMsg, u32 aiLevel, String asFile, int aiLine);

#endif // IMAGELOADER_H
