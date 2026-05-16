// std::fstream 系列：构造、多种 openmode、binary/text、临时文件读写、noreplace（若可用）。

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace fs = std::filesystem;

namespace {

void printSep() {
    std::cout << "---\n";
}

#if defined(__cpp_lib_ios_noreplace) || (__cplusplus >= 202302L)

void demoNoreplaceFailsWhenFileAlreadyExists(fs::path const& unique_path) {
    {
        std::ofstream seeded(unique_path, std::ios::out | std::ios::trunc);
        seeded << "seed\n";
    }
    std::ofstream blocker(unique_path, std::ios::out | std::ios::noreplace);
    std::cout << "noreplace 打开已存在路径: is_open -> " << std::boolalpha << blocker.is_open()
              << " (应为 false)\n";
}

#else

void demoNoreplaceFailsWhenFileAlreadyExists(fs::path const& /*unused*/) {
    std::cout << "当前工具链未检测到 C++23 noreplace（跳过）。\n";
}

#endif

struct PodPayload {
    std::uint32_t magic{};
    double coefficient{};
};
static_assert(std::is_trivially_copyable_v<PodPayload>);

void scrubPath(fs::path const& tmp) noexcept {
    std::error_code ec{};
    fs::remove(tmp, ec);
}

void demoTemporaryTextRoundTrip(fs::path const& line_file) {
    scrubPath(line_file);
    {
        std::ofstream writer(line_file);
        writer << "line-one\n";
        writer << std::setw(10) << 16 << '\n';
    }
    {
        std::ifstream reader(line_file);
        std::string header{};
        std::getline(reader, header);
        int boxed = {};
        reader >> boxed;
        std::cout << "getline: \"" << header << "\" 随后整数: " << boxed << '\n';
    }
}

void demoReuseStreamWithOpenClose(fs::path const& scratch) {
    scrubPath(scratch);
    std::fstream bridge{};
    bridge.open(scratch, std::ios::out | std::ios::trunc);
    std::cout << "首次打开写: good=" << bridge.good() << " is_open=" << bridge.is_open() << '\n';
    bridge << "payload";
    bridge.close();

    bridge.open(scratch, std::ios::in);
    std::cout << "重用流对象只读 reopen: good=" << bridge.good() << " is_open=" << bridge.is_open()
              << '\n';
    std::string body{};
    bridge >> body;
    std::cout << "读回: \"" << body << "\"\n";
    bridge.close();
}

void demoBinaryWriteRead(fs::path const& bin_file) {
    scrubPath(bin_file);
    constexpr std::uint32_t kMagic = 0xABCD1234UL;
    constexpr double kCoefficient = 2.625;
    PodPayload outbound{.magic = kMagic, .coefficient = kCoefficient};
    {
        std::ofstream writer(bin_file, std::ios::binary | std::ios::trunc);
        writer.write(reinterpret_cast<char const*>(
                         &outbound),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                     static_cast<std::streamsize>(sizeof(outbound)));
    }
    PodPayload inbound{};
    {
        std::ifstream reader(bin_file, std::ios::binary);
        reader.read(reinterpret_cast<char*>(
                        &inbound),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                    static_cast<std::streamsize>(sizeof(inbound)));
    }
    std::cout << "二进制 round-trip: magic=0x" << std::hex << inbound.magic << std::dec
              << " coeff=" << inbound.coefficient << '\n';
}

void demoTruncVersusApp(fs::path const& truncate_path, fs::path const& append_path) {
    scrubPath(truncate_path);
    scrubPath(append_path);
    std::string const snippet{"zig"};
    {
        std::ofstream first_trunc(truncate_path, std::ios::out | std::ios::trunc);
        first_trunc << snippet << snippet;
        first_trunc.close();
        std::ofstream second_trunc(truncate_path, std::ios::out | std::ios::trunc);
        second_trunc << snippet;
        second_trunc.close();
    }
    auto trunc_len =
        fs::exists(truncate_path) ? fs::file_size(truncate_path) : static_cast<std::uintmax_t>(0);
    std::cout << "两次 trunc（第二次覆盖）：文件长度应为 " << snippet.size() << " → " << trunc_len
              << " 字节\n";

    {
        std::ofstream seed(append_path, std::ios::out | std::ios::trunc);
        seed << snippet;
        seed.close();
        std::ofstream tail(append_path, std::ios::out | std::ios::app);
        tail << snippet;
        tail.close();
    }
}

void demoTextVersusBinarySize(fs::path const& text_clone, fs::path const& binary_clone) {
    scrubPath(text_clone);
    scrubPath(binary_clone);
    constexpr char kToken = 'z';
    {
        std::ofstream text_sink(text_clone, std::ios::out);
        text_sink.put(kToken);
        text_sink.put('\n');
    }
    {
        std::ofstream binary_sink(binary_clone, std::ios::binary | std::ios::out);
        binary_sink.put(kToken);
        binary_sink.put('\n');
    }
    auto text_size =
        fs::exists(text_clone) ? fs::file_size(text_clone) : static_cast<std::uintmax_t>(0);
    auto binary_size =
        fs::exists(binary_clone) ? fs::file_size(binary_clone) : static_cast<std::uintmax_t>(0);
    std::cout << "同一 \"z\\n\" 文本模式字节数=" << text_size << " 二进制字节数=" << binary_size;
    std::cout << "（仅在 Windows CRT 文本转换下常见差异）\n";
}

void demoAteStartsAtEnd(fs::path const& tail_path) {
    scrubPath(tail_path);
    constexpr auto kAbcdLength = static_cast<std::streamoff>(4);
    std::string_view const tail_piece{"EF"};
    {
        std::ofstream seed(tail_path, std::ios::binary | std::ios::out | std::ios::trunc);
        seed << "abcd";
    }
    std::fstream extender(tail_path,
                          std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    std::cout << "ate 打开后 tellp=" << static_cast<std::intmax_t>(extender.tellp())
              << "（应等于 abcd 长度 " << static_cast<std::intmax_t>(kAbcdLength) << "）\n";
    extender.write(tail_piece.data(), static_cast<std::streamsize>(tail_piece.size()));
    extender.flush();
    std::ostringstream dump{};
    {
        std::ifstream verifier(tail_path, std::ios::binary | std::ios::in);
        dump << verifier.rdbuf();
    }
    std::cout << "binary 读出整段=\"" << dump.str() << "\"\n";
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    auto const base_dir = fs::temp_directory_path();
    auto const lines = base_dir / "mcpp_fstream_demo_lines.tmp";
    auto const reusable = base_dir / "mcpp_fstream_demo_reopen.tmp";
    auto const blob = base_dir / "mcpp_fstream_demo_blob.bin";
    auto const truncate_sample = base_dir / "mcpp_fstream_trunc.tmp";
    auto const append_sample = base_dir / "mcpp_fstream_app.tmp";
    auto const platform_text = base_dir / "mcpp_fstream_text.newline.tmp";
    auto const platform_binary = base_dir / "mcpp_fstream_binary.newline.tmp";
    auto const tail_sample = base_dir / "mcpp_fstream_ate.tmp";
    auto const noreplace_probe = base_dir / "mcpp_fstream_noreplace.tmp";

    demoTemporaryTextRoundTrip(lines);
    printSep();
    demoReuseStreamWithOpenClose(reusable);
    printSep();
    demoBinaryWriteRead(blob);
    printSep();
    demoTruncVersusApp(truncate_sample, append_sample);
    printSep();
    demoTextVersusBinarySize(platform_text, platform_binary);
    printSep();
    demoAteStartsAtEnd(tail_sample);
    printSep();
    demoNoreplaceFailsWhenFileAlreadyExists(noreplace_probe);

    scrubPath(lines);
    scrubPath(reusable);
    scrubPath(blob);
    scrubPath(truncate_sample);
    scrubPath(append_sample);
    scrubPath(platform_text);
    scrubPath(platform_binary);
    scrubPath(tail_sample);
    scrubPath(noreplace_probe);
#else
    std::cout << "未启用 std::filesystem，跳过 fstream_demo。\n";
#endif
    return 0;
}
