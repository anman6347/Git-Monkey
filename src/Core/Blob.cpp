#include "Blob.hpp"

int Blob::create_blob_bin_data()
{
    // read content in text mode, and set content
    get_text_data();
    for (int i = 0; i < text_data.size(); i++) {
        blob_data.content.push_back(text_data[i]);
    }
    // set size
    blob_data.bh.content_size = std::to_string(blob_data.content.size());

    // create blob binary data
    bin_data.insert(bin_data.end(), blob_data.bh.sig.begin(), blob_data.bh.sig.end());
    bin_data.push_back(' ');
    for(int i = 0; i < blob_data.bh.content_size.size(); i++) {
        bin_data.push_back(static_cast<uint8_t>(blob_data.bh.content_size[i]));
    }
    bin_data.push_back('\0');
    bin_data.insert(bin_data.end(), blob_data.content.begin(), blob_data.content.end());

    return EXIT_SUCCESS;
}
