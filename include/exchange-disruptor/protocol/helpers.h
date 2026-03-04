#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>
#include <vector>
#include <cstddef>

inline void write_u8(std::vector<std::uint8_t>& out, std::uint8_t v) {
    out.push_back(v);
}

inline void write_u16_be(std::vector<std::uint8_t>& out, std::uint16_t v) {
    out.push_back(static_cast<std::uint8_t>(v >> 8));
    out.push_back(static_cast<std::uint8_t>(v));
}

inline void write_u32_be(std::vector<std::uint8_t>& out, std::uint32_t v) {
    out.push_back(static_cast<std::uint8_t>(v >> 24));
    out.push_back(static_cast<std::uint8_t>(v >> 16));
    out.push_back(static_cast<std::uint8_t>(v >> 8));
    out.push_back(static_cast<std::uint8_t>(v));
}

inline void write_u64_be(std::vector<std::uint8_t>& out, std::uint64_t v) {
    out.push_back(static_cast<std::uint8_t>(v >> 56));
    out.push_back(static_cast<std::uint8_t>(v >> 48));
    out.push_back(static_cast<std::uint8_t>(v >> 40));
    out.push_back(static_cast<std::uint8_t>(v >> 32));
    out.push_back(static_cast<std::uint8_t>(v >> 24));
    out.push_back(static_cast<std::uint8_t>(v >> 16));
    out.push_back(static_cast<std::uint8_t>(v >> 8));
    out.push_back(static_cast<std::uint8_t>(v));
}

inline void write_i64_be(std::vector<std::uint8_t>& out, std::int64_t v) {
    write_u64_be(out, static_cast<std::uint64_t>(v));
}


inline bool read_u8(const std::vector<std::uint8_t>& in, std::size_t& off, std::uint8_t& v) {
    if (off + 1 > in.size()) return false;
    v = in[off++];
    return true;
}

inline bool read_u16_be(const std::vector<std::uint8_t>& in, std::size_t& off, std::uint16_t& v) {
    if (off + 2 > in.size()) return false;
    v = (static_cast<std::uint16_t>(in[off]) << 8) |
        (static_cast<std::uint16_t>(in[off + 1]));
    off += 2;
    return true;
}

inline bool read_u32_be(const std::vector<std::uint8_t>& in, std::size_t& off, std::uint32_t& v) {
    if (off + 4 > in.size()) return false;
    v = (static_cast<std::uint32_t>(in[off]) << 24) |
        (static_cast<std::uint32_t>(in[off + 1]) << 16) |
        (static_cast<std::uint32_t>(in[off + 2]) << 8) |
        (static_cast<std::uint32_t>(in[off + 3]));
    off += 4;
    return true;
}

inline bool read_u64_be(const std::vector<std::uint8_t>& in, std::size_t& off, std::uint64_t& v) {
    if (off + 8 > in.size()) return false;
    v = (static_cast<std::uint64_t>(in[off]) << 56) |
        (static_cast<std::uint64_t>(in[off + 1]) << 48) |
        (static_cast<std::uint64_t>(in[off + 2]) << 40) |
        (static_cast<std::uint64_t>(in[off + 3]) << 32) |
        (static_cast<std::uint64_t>(in[off + 4]) << 24) |
        (static_cast<std::uint64_t>(in[off + 5]) << 16) |
        (static_cast<std::uint64_t>(in[off + 6]) << 8) |
        (static_cast<std::uint64_t>(in[off + 7]));
    off += 8;
    return true;
}

inline bool read_i64_be(const std::vector<std::uint8_t>& in, std::size_t& off, std::int64_t& v) {
    std::uint64_t tmp = 0;
    if (!read_u64_be(in, off, tmp)) return false;
    v = static_cast<std::int64_t>(tmp);
    return true;
}

#endif // HELPERS_H