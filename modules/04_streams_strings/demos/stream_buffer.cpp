// std::streambuf 的职责与自定义缓冲：与 seekg/seekp 协同定位读写窗口。
//
// 关键点：
//   典型实现需要维护 eback/gptr/egptr 与 pbase/pptr/epptr；
//   stringstream 的底层 stringbuf 已支持 seeks，适合先读懂再自创最小版本。

#include <cstring>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

namespace {

// 极小只写缓冲：将字符沉淀到外部 vector<char>。
class VectorWriteBuf final : public std::streambuf {
public:
    explicit VectorWriteBuf(std::vector<char>& sink) noexcept : sink_(sink) {
        constexpr std::size_t kChunk = 32;
        buffer_.resize(kChunk);
        setp(buffer_.data(), buffer_.data() + static_cast<std::ptrdiff_t>(buffer_.size()));
    }

protected:
    int_type overflow(int_type ch) override {
        if (!traits_type::eq_int_type(ch, traits_type::eof())) {
            flushBuf();
            (void)sputc(traits_type::to_char_type(ch));  // 成功路径按惯例返回写入字符（int_type）
            return ch;
        }
        return traits_type::eof();
    }

    std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
                           std::ios_base::openmode which) override {
        if ((which & std::ios_base::out) == 0) {
            return static_cast<std::streamoff>(-1);
        }
        flushBuf();
        switch (way) {
        case std::ios_base::beg:
            if (off < 0 || static_cast<std::size_t>(off) > sink_.size()) {
                return static_cast<std::streamoff>(-1);
            }
            write_pos_ = static_cast<std::size_t>(off);
            break;
        case std::ios_base::cur:
            // 近似：视作“写指针”永远在尾部时可简化为 SEEK_END-only；此处支持 cur/end 的最小语义。
            if (off != 0) {
                return static_cast<std::streamoff>(-1);
            }
            break;
        case std::ios_base::end:
            write_pos_ = sink_.size();
            break;
        default:
            return static_cast<std::streamoff>(-1);
        }
        (void)off;
        return static_cast<std::streampos>(static_cast<std::streamoff>(write_pos_));
    }

    int sync() override {
        flushBuf();
        return 0;
    }

private:
    void flushBuf() {
        auto len = static_cast<std::size_t>(pptr() - pbase());
        std::cout << "(debug) flush " << len << " bytes into sink\n";  // 教学可视化
        if (write_pos_ + len > sink_.size()) {
            sink_.resize(write_pos_ + len);
        }
        std::memcpy(sink_.data() + write_pos_, pbase(), len);
        write_pos_ += len;
        setp(buffer_.data(), buffer_.data() + static_cast<std::ptrdiff_t>(buffer_.size()));
    }

    std::vector<char> buffer_;
    std::vector<char>& sink_;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    std::size_t write_pos_{0};
};

void printSep() { std::cout << "---\n"; }

void demoStringbufSeek() {
    std::stringstream ss{"0123456789"};
    ss.seekg(5);
    char ch = '\0';
    ss.get(ch);
    std::cout << "seekg 到 5 后读取: '" << ch << "'\n";
    ss.clear();
    ss.seekp(0, std::ios_base::beg);
    ss.seekp(2);
    ss << "**";
    std::cout << "在位置 2 覆写两字后整串 -> " << ss.str() << '\n';
}

void demoCustomWriteBuf() {
    std::vector<char> backing{};
    VectorWriteBuf buf(backing);
    std::ostream out(&buf);
    out << "hello";
    out << "-buffer";
    out.flush();  // 触发 sync
    std::cout << "自定义 streambuf 实际沉淀长度: " << backing.size() << '\n';
}

}  // namespace

int main() {
    demoStringbufSeek();
    printSep();
    demoCustomWriteBuf();
    std::cout << "生产代码应直接使用 std::stringbuf / filebuf 或使用 Boost.Nowide 之类成熟方案。\n";
    return 0;
}
