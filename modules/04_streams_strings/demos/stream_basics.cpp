// std::istream / std::ostream / fstream / stringstream 的常见用法。
//
// 关键点：
//   格式化输入用 operator>>（默认按空白切段），二进制或行协议常配合 getline；
//   rdstate、eof/fail/good/clear 需要显式检查，别把「读到 EOF」与「读到合法值」混为一谈。

#include <fstream>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace {

void printSep() { std::cout << "---\n"; }

void demoStringStreams() {
    std::ostringstream out;
    out << "chunk=" << 3 << ' ' << std::quoted("escaped");
    std::cout << "ostringstream: " << out.str() << '\n';

    std::istringstream in{"12  hello   world"};
    int n = 0;
    std::string w0;
    std::string w1;
    in >> n >> w0 >> w1;
    std::cout << "istringstream 拆分: n=" << n << " w0=\"" << w0 << "\" w1=\"" << w1 << "\"\n";
    std::cout << "eof? " << in.eof() << " good? " << in.good() << '\n';
}

void demoTempFileRoundTrip() try {
#if __cpp_lib_filesystem >= 201703L
    auto path = fs::temp_directory_path() / "mcpp_streams_demo.tmp";
    {
        std::ofstream fout(path);
        fout << "line1 alpha\n";
        fout << 42 << ' ' << 3.14 << '\n';
    }
    {
        std::ifstream fin(path);
        std::string line{};
        std::getline(fin, line);
        int iv = 0;
        double dv = 0.0;
        fin >> iv >> dv;
        std::cout << "自文件读出: \"" << line << "\" 然后 " << iv << ' ' << dv << '\n';
    }
    std::error_code ec;
    fs::remove(path, ec);
#else
    (void)sizeof(0);  // 占位，避免警告
    std::cout << "<filesystem> 不可用，跳过临时文件演示。\n";
#endif
} catch (...) {
    std::cout << "临时文件演示因异常跳过。\n";
}

void demoCinPeek() {
    std::cout << "(演示) 可把 std::stringstream 视作 cin 替代品做单元测试。\n";
}

}  // namespace

int main() {
    demoStringStreams();
    printSep();
    demoTempFileRoundTrip();
    printSep();
    demoCinPeek();
    return 0;
}
