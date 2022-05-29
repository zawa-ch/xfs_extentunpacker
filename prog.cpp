#include <cstdint>
#include <string>
#include <stdexcept>
#include <iostream>
#include <array>

enum class XfsExtentFlag {
    Normal,
    Unwritten,
};

std::ostream& operator<<(std::ostream& stream, const XfsExtentFlag& obj) {
    switch(obj) {
        case XfsExtentFlag::Normal: return stream << "normal";
        case XfsExtentFlag::Unwritten: return stream << "unwritten";
        default: return stream << "<invalid>";
    }
}

struct XfsExtentRecord {
public:
    typedef std::array<uint8_t, 16> RawBinary;
private:
    RawBinary _data;
public:
    XfsExtentRecord() = default;
    explicit XfsExtentRecord(const RawBinary& binary) : _data(binary) {}

    [[nodiscard]] XfsExtentFlag get_flag() const noexcept { return ((_data[0] >> 7) != 0)?(XfsExtentFlag::Unwritten):(XfsExtentFlag::Normal); }
    [[nodiscard]] uint64_t get_file_boffset() const noexcept {
        auto result = uint64_t(_data[0] & 0x7F);
        for(size_t i = 1; i < 6; ++i) {
            result <<= 8;
            result |= _data[i];
        }
        result <<= 7;
        result |= (_data[6] >> 1);
        return result;
    }
    [[nodiscard]] uint64_t get_fsb_index() const noexcept {
        auto result = uint64_t(_data[6] & 0x01);
        for(size_t i = 7; i < 13; ++i) {
            result <<= 8;
            result |= _data[i];
        }
        result <<= 3;
        result |= (_data[13] >> 5);
        return result;
    }
    [[nodiscard]] uint32_t get_fsb_count() const noexcept {
        auto result = uint64_t(_data[13] & 0x1F);
        for(size_t i = 14; i < 16; ++i) {
            result <<= 8;
            result |= _data[i];
        }
        return result;
    }

    explicit operator RawBinary() const noexcept { return _data; }
};

class Hexdecoder {
    std::string text;
public:
    Hexdecoder() : text() {}
    explicit Hexdecoder(const std::string& text) : text(text) {}

    template<size_t N>
    [[nodiscard]] std::array<uint8_t, N> decode() const {
        auto result = std::array<uint8_t, N>();
        size_t l = 0;
        for(auto i: text) {
            if (!issp(i))
            {
                result[l / 2] <<= 4;
                result[l / 2] |= c2b(i);
                ++l;
            }
        }
        if ((l / 2) != N) { throw std::exception(); }
        return result;
    }
private:
    [[nodiscard]] static uint8_t c2b(const char& c) {
        switch (c)
        {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'a': return 10;
            case 'b': return 11;
            case 'c': return 12;
            case 'd': return 13;
            case 'e': return 14;
            case 'f': return 15;
            case 'A': return 10;
            case 'B': return 11;
            case 'C': return 12;
            case 'D': return 13;
            case 'E': return 14;
            case 'F': return 15;
            default: throw std::exception();
        }
    }
    [[nodiscard]] static bool issp(const char& c) {
        return c == ' ' || c == '\n' || c == '\t';
    }
};

int main(int argc, char const *argv[])
{
    std::string input;
    std::getline(std::cin, input);
    auto dec = Hexdecoder(input);
    auto extent = XfsExtentRecord(dec.decode<16>());
    std::cout << "flag: " << extent.get_flag() << std::endl;
    std::cout << "fileoffset: " << extent.get_file_boffset() << std::endl;
    std::cout << "fsbindex: " << extent.get_fsb_index() << std::endl;
    std::cout << "block #: " << extent.get_fsb_count() << std::endl;
    return 0;
}
