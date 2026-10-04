#include <nn/atk/atk_StreamSoundFileLoader.h>

namespace nn::atk::detail {
/**
 * @brief Destroy the region reader interface without owning the stream resource.
 */
IRegionInfoReadable::~IRegionInfoReadable() = default;

/**
 * @brief Destroy the loader without closing its externally owned file stream.
 */
StreamSoundFileLoader::~StreamSoundFileLoader() = default;

/**
 * @brief Construct an unattached stream file reader.
 */
StreamSoundFileReader::StreamSoundFileReader() : mHeader(nullptr), mInfo(nullptr) {}
}
