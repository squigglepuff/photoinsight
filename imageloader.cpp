#include "imageloader.h"

/*!
 * \brief CImageLoader constructor.
 *
 * This constructor doesn't actually do anything except setup an initial sane state for the object. The client *MUST* call "InitLoader()" before using any other object functions!
 */
CImageLoader::CImageLoader() :
    mbInitialized(false),
    meInternalError(Error_Success),
    mvInternalBuffer(),
    miWidth(0),
    miHeight(0),
    miBPP(0),
    mePixelOrder(PixelOrder_RGB),
    msMimeType(""),
    mpImage(nullptr)
{
    // Intentionally left blank.
}

CImageLoader::CImageLoader(const CImageLoader& aCls) :
    mbInitialized(aCls.mbInitialized),
    meInternalError(aCls.meInternalError),
    mvInternalBuffer(aCls.mvInternalBuffer),
    miWidth(aCls.miWidth),
    miHeight(aCls.miHeight),
    miBPP(aCls.miBPP),
    mePixelOrder(aCls.mePixelOrder),
    msMimeType(aCls.msMimeType),
    mpImage(nullptr)
{
    // Intentionally left blank.
}

CImageLoader::~CImageLoader()
{
    ResetState();
}

std::ostream& CImageLoader::operator <<(std::ostream& aOut)
{
    if (mbInitialized && !mvInternalBuffer.empty())
    {
        aOut<< mvInternalBuffer.data();
    }

    return aOut;
}

/*!
 * \brief Initialize loader class/module.
 *
 * This function simply initializes the loader after resetting it to a sane state. This function will also initialize FreeImage under-the-hood.
 *
 * \return TRUE is returned if successful, otherwise FALSE and error code is set.
 */
bool CImageLoader::InitLoader()
{
    if (!mbInitialized)
    {
        ResetState();
        if (Error_Success == InitFreeimage())
        {
            mbInitialized = true;
        }
    }

    return mbInitialized;
}

/*!
 * \brief Clear the error code.
 *
 *  This function will reset the internal error code to be 0 (zero).
 */
void CImageLoader::ClearError()
{
    SetError(Error_Success);
}

/*!
 * \brief Get last error set.
 * \return Returns the last error set by the module.
 */
CImageLoader::Error CImageLoader::GetLastError()
{
    return meInternalError;
}

/*!
 * \brief Convert error code to string.
 *
 * This function will convert a passed error code to a human-readable string.
 *
 * \param[in] aErr - The error code to be converted.
 * \return Human readable version of error.
 */
String CImageLoader::GetErrorString(CImageLoader::Error aErr)
{
    String lMsg = "";
    switch(aErr)
    {
        // Generic errors.
        case Error_Success: { lMsg = "No Error"; break; }
        case Error_NoInit: { lMsg = "Loader not Initialized!"; break; }
        case Error_OutOfMemory: { lMsg = "Out of Memory!"; break; }
        case Error_BadFileDescriptor: { lMsg = "Bad File descriptor!"; break; }
        case Error_FileCorrupt: { lMsg = "File is corrupt/invalid!"; break; }
        case Error_BadPath: { lMsg = "Bad path to file!"; break; }

        // FreeImage specific errors.
        case Error_FI_InitFail: { lMsg = "[FreeImage] Initialization failed!"; break; }
        case Error_FI_LoadFail: { lMsg = "[FreeImage] Failed to load image!"; break; }
        case Error_FI_UnknownType: { lMsg = "[FreeImage] Unknown image type!"; break; }
        case Error_FI_BadPixelFormat: { lMsg = "[FreeImage] Invalid/Unknown pixel format!"; break; }

        // Special case.
        case Error_Unknown: { lMsg = "Unknown error! D:"; break; }

        // System errors.
        default: { lMsg = strerror(aErr); }
    }

    return lMsg;
}

/*!
 * \brief Clear the internal image buffer.
 * \return TRUE is returned if successful, otherwise FALSE and error code is set.
 */
bool CImageLoader::ClearImage()
{
    bool bSuccess = false;
    if (mbInitialized)
    {
        ResetState();
    }
    else
    {
        SetError(Error_NoInit);
    }

    return bSuccess;
}

/*!
 * \brief Load image from disk.
 *
 * This function will attempt to load the image at the specified path via FreeImage.
 *
 * \param[in] aPath - Path to image to be loaded.
 * \return TRUE is returned if successful, otherwise FALSE and error code is set.
 */
bool CImageLoader::LoadImage(String aPath)
{
    bool bSuccess = false;
    if (mbInitialized)
    {
        if (!aPath.empty())
        {
            FREE_IMAGE_FORMAT lFormat = FreeImage_GetFIFFromFilename(aPath.c_str());
            mpImage = FreeImage_Load(lFormat, aPath.c_str(), 0);

            if (nullptr != mpImage)
            {
                // We need to convert the image to RGB.
                mpImage = FreeImage_ConvertTo32Bits(mpImage);
                FreeImage_FlipVertical(mpImage);

                byte* pDataPtr = FreeImage_GetBits(mpImage);

                miWidth = FreeImage_GetWidth(mpImage);
                miHeight = FreeImage_GetHeight(mpImage);
                miBPP = FreeImage_GetBPP(mpImage);
                msMimeType = FreeImage_GetFIFMimeType(lFormat);
                // PixelOrder not yet implemented!

                mvInternalBuffer.clear();

                u32 iSize = FreeImage_GetMemorySize(mpImage);
                for (u32 iIdx = 0; iIdx < iSize; ++iIdx)
                {
                    mvInternalBuffer.push_back(*(pDataPtr+iIdx));
                }

                SetError(Error_Success);
                bSuccess = true;
            }
            else
            {
                SetError(Error_FI_LoadFail);
            }
        }
        else
        {
            SetError(Error_BadPath);
        }
    }
    else
    {
        SetError(Error_NoInit);
    }

    return bSuccess;
}

/*!
 * \brief Get loaded image data.
 * \return Returns the loaded image data (if any).
 */
std::vector<byte> CImageLoader::GetData()
{
    return mvInternalBuffer;
}

/*!
 * \brief Get raw image data.
 * \return Returns the loaded image data (if any) in a raw buffer.
 */
const byte* CImageLoader::GetRawData()
{
    return FreeImage_GetBits(mpImage);
}

/*!
 * \brief Get the size of the loaded data.
 * \return Returns the size of the loaded image data buffer in bytes.
 */
u32 CImageLoader::GetDataSize()
{
//    return mvInternalBuffer.size();
    return FreeImage_GetMemorySize(mpImage);
}

/*!
 * \brief Get image MIME type.
 * \return Returns MIME type of loaded image as a string.
 */
String CImageLoader::GetMime()
{
    return msMimeType;
}

/*!
 * \brief Get image width.
 * \return Returns the loaded image's width in pixels.
 */
u32 CImageLoader::GetWidth()
{
    return miWidth;
}

/*!
 * \brief Get image height.
 * \return Returns the loaded image's height in pixels.
 */
u32 CImageLoader::GetHeight()
{
    return miHeight;
}

/*!
 * \brief Get image bit-depth.
 * \return Returns the loaded image's bit-depth (8-, 16-, 24-, 32-bit).
 */
u32 CImageLoader::GetBitDepth()
{
    return miBPP;
}

/*!
 * \brief Get image aspect ratio.
 * \return Returns the aspect ratio of the loaded image (if any).
 */
float CImageLoader::GetAspectRatio()
{
    return (static_cast<float>(miWidth) / static_cast<float>(miHeight));
}

/*!
 * \brief Get image pixel order. (RGB, BGR, etc.)
 * \return Returns the loaded image's pixel order.
 */
CImageLoader::PixelOrder CImageLoader::GetPixelOrder()
{
    return mePixelOrder;
}

/*!
 * \brief Reset state
 *
 * This function resets all the internal variables to a sane state.
 */
void CImageLoader::ResetState()
{
    meInternalError = Error_Success;
    mvInternalBuffer.clear();
    miWidth = 0;
    miHeight = 0;
    miBPP = 0;
    mePixelOrder = PixelOrder_RGB;
    msMimeType = "";

    FreeImage_Unload(mpImage);
}

/*!
 * \brief Set last error code.
 * \param[in] aErr - The code to set.
 */
void CImageLoader::SetError(CImageLoader::Error aErr)
{
    meInternalError = aErr;
}

/*!
 * \brief Initialize FreeImage library.
 *
 * This function will attempt to initialize the FreeImage library.
 *
 * \return Returns the error code from attempt (if any).
 */
CImageLoader::Error CImageLoader::InitFreeimage()
{
    LogMessage("Loading FreeImage...", 3, __FILE__, __LINE__);
    FreeImage_Initialise();

    String lMessage = "Loaded FreeImage library: ";
    lMessage.append(FreeImage_GetVersion());

    LogMessage(lMessage, 3, __FILE__, __LINE__);

    //!\todo TODO: Fix this so it error checks!
    return Error_Success;
}
