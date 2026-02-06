#include "input.hpp"
#include <algorithm>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/locale/encoding.hpp>
#include <vector>

#include "utf-8.hpp"

namespace io = boost::iostreams;

const unsigned BUFFER_SIZE = 4096;

// Lazily convert encoding to UTF-8
class EncodingFilter : public io::multichar_input_filter {
public:
    explicit EncodingFilter(std::string encoding) : m_encoding(std::move(encoding)) {}

    template<typename Source>
    std::streamsize read(Source &src, char *s, std::streamsize n) {
        if (n <= 0)
            return 0;

        if (!m_buffer.empty()) {
            std::streamsize count = std::min(n, static_cast<std::streamsize>(m_buffer.size()));
            std::copy(m_buffer.begin(), m_buffer.begin() + count, s);
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + count);
            return count;
        }

        char in_buf[BUFFER_SIZE];
        std::streamsize got = io::read(src, in_buf, sizeof(in_buf));

        bool is_eof = got == -1;
        if (is_eof) {
            if (!m_remainder.empty()) {
                m_remainder.clear();
            }
            return -1;
        }

        std::string chunk = m_remainder;
        chunk.append(in_buf, got);
        m_remainder.clear();

        // For UTF-16, we need even number of bytes.
        size_t process_len = chunk.size();
        if (m_encoding.find("UTF-16") != std::string::npos) {
            size_t remainder_len = chunk.size() % 2;
            m_remainder = chunk.substr(chunk.size() - remainder_len);
            process_len -= remainder_len;
        }
        // For single-byte encodings (ISO-8859-*), process_len is all.
        if (process_len == 0) {
            // We got data but couldn't process it (e.g. 1 byte of UTF-16).
            // Need to read more.
            return read(src, s, n);
        }

        try {
            std::string converted = boost::locale::conv::to_utf<char>(chunk.substr(0, process_len), m_encoding);
            m_buffer.insert(m_buffer.end(), converted.begin(), converted.end());
        } catch (...) {
            // Conversion error
            m_buffer.push_back('?');
        }

        // Recursively try to serve from the now-filled buffer
        return read(src, s, n);
    }


private:
    std::string m_encoding;
    std::vector<char> m_buffer;
    std::string m_remainder;
};

std::unique_ptr<std::istream> open_file(const std::string &path) {
    // Check for BOM or heuristics using a temporary generic stream
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Can't open file: " + path);

    char header[4096];
    file.read(header, sizeof(header));
    std::streamsize read_bytes = file.gcount();
    file.close(); // Close to let file_source open it clean

    std::string encoding = "UTF-8";

    // BOM Check
    bool has_bom = false;
    if (read_bytes >= 2) {
        unsigned char b0 = static_cast<unsigned char>(header[0]);
        unsigned char b1 = static_cast<unsigned char>(header[1]);
        if (b0 == 0xFF && b1 == 0xFE) {
            encoding = "UTF-16LE";
            has_bom = true;
        } else if (b0 == 0xFE && b1 == 0xFF) {
            encoding = "UTF-16BE";
            has_bom = true;
        }
    }

    if (!has_bom && read_bytes > 0) {
        std::string_view chunk(header, read_bytes);
        if (!utf8::is_valid(chunk)) {
            // UTF-8, assume System Locale
            // Empty string for encoding is "System Locale" in boost::locale
            encoding = "";
        }
    }

    auto in = std::make_unique<io::filtering_istream>();

    if (encoding != "UTF-8") {
        in->push(EncodingFilter(encoding));
    }

    in->push(io::file_source(path));
    return in;
}

