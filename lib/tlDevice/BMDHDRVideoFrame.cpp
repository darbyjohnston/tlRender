// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlDevice/BMDOutputPrivate.h>

namespace tl
{
    namespace bmd
    {
        DLHDRVideoFrame::DLHDRVideoFrame(IDeckLinkMutableVideoFrame* frame, image::HDRData& hdrData) :
            _frame(frame),
            _hdrData(hdrData),
            _refCount(1)
        {
            _frame->AddRef();
        }

        DLHDRVideoFrame::~DLHDRVideoFrame()
        {
            _frame->Release();
        }

        HRESULT DLHDRVideoFrame::QueryInterface(REFIID iid, LPVOID* ppv)
        {
#if defined(__APPLE__)
            CFUUIDBytes iunknown = CFUUIDGetUUIDBytes(IUnknownUUID);
#elif defined(_WINDOWS)
            IID iunknown = IID_IUnknown;
#else // __APPLE__
            CFUUIDBytes iunknown;
#endif // __APPLE__
            if (ppv == nullptr)
                return E_INVALIDARG;
            if (memcmp(&iid, &iunknown, sizeof(REFIID)) == 0)
                *ppv = static_cast<IDeckLinkVideoFrame*>(this);
            else if (memcmp(&iid, &IID_IDeckLinkVideoFrame, sizeof(REFIID)) == 0)
                *ppv = static_cast<IDeckLinkVideoFrame*>(this);
            else if (memcmp(&iid, &IID_IDeckLinkVideoFrameMetadataExtensions, sizeof(REFIID)) == 0)
                *ppv = static_cast<IDeckLinkVideoFrameMetadataExtensions*>(this);
            else
            {
                *ppv = nullptr;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }

        ULONG DLHDRVideoFrame::AddRef(void)
        {
            return ++_refCount;
        }

        ULONG DLHDRVideoFrame::Release(void)
        {
            ULONG newRefValue = --_refCount;
            if (newRefValue == 0)
                delete this;
            return newRefValue;
        }

        HRESULT DLHDRVideoFrame::GetInt(BMDDeckLinkFrameMetadataID metadataID, int64_t* value)
        {
            HRESULT result = S_OK;
            switch (metadataID)
            {
            case bmdDeckLinkFrameMetadataHDRElectroOpticalTransferFunc:
                *value = static_cast<int>(_hdrData.eotf);
                break;
            case bmdDeckLinkFrameMetadataColorspace:
                *value = bmdColorspaceRec2020;
                break;
            default:
                value = nullptr;
                result = E_INVALIDARG;
            }
            return result;
        }

        HRESULT DLHDRVideoFrame::GetFloat(BMDDeckLinkFrameMetadataID metadataID, double* value)
        {
            HRESULT result = S_OK;
            switch (metadataID)
            {
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedX:
                *value = _hdrData.primaries[0].x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedY:
                *value = _hdrData.primaries[0].y;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenX:
                *value = _hdrData.primaries[1].x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenY:
                *value = _hdrData.primaries[1].y;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueX:
                *value = _hdrData.primaries[2].x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueY:
                *value = _hdrData.primaries[2].y;
                break;
            case bmdDeckLinkFrameMetadataHDRWhitePointX:
                *value = _hdrData.primaries[3].x;
                break;
            case bmdDeckLinkFrameMetadataHDRWhitePointY:
                *value = _hdrData.primaries[3].y;
                break;
            case bmdDeckLinkFrameMetadataHDRMaxDisplayMasteringLuminance:
                *value = _hdrData.displayMasteringLuminance.max();
                break;
            case bmdDeckLinkFrameMetadataHDRMinDisplayMasteringLuminance:
                *value = _hdrData.displayMasteringLuminance.min();
                break;
            case bmdDeckLinkFrameMetadataHDRMaximumContentLightLevel:
                *value = _hdrData.maxCLL;
                break;
            case bmdDeckLinkFrameMetadataHDRMaximumFrameAverageLightLevel:
                *value = _hdrData.maxFALL;
                break;
            default:
                value = nullptr;
                result = E_INVALIDARG;
            }
            return result;
        }

        HRESULT DLHDRVideoFrame::GetFlag(BMDDeckLinkFrameMetadataID, BOOL* value)
        {
            *value = false;
            return E_INVALIDARG;
        }

#if defined(__APPLE__)
        HRESULT DLHDRVideoFrame::GetString(BMDDeckLinkFrameMetadataID, CFStringRef* value)
        {
            *value = nullptr;
            return E_INVALIDARG;
        }
#elif defined(_WINDOWS)
        HRESULT DLHDRVideoFrame::GetString(BMDDeckLinkFrameMetadataID, BSTR* value)
        {
            *value = nullptr;
            return E_INVALIDARG;
        }
#else // __APPLE__
        HRESULT DLHDRVideoFrame::GetString(BMDDeckLinkFrameMetadataID, const char** value)
        {
            *value = nullptr;
            return E_INVALIDARG;
        }
#endif // __APPLE__

        HRESULT	DLHDRVideoFrame::GetBytes(BMDDeckLinkFrameMetadataID metadataID, void* buffer, uint32_t* bufferSize)
        {
            *bufferSize = 0;
            return E_INVALIDARG;
        }
    }
}
