#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>

std::optional<int> findStudent(const std::map<std::string, int> &students, const std::string &name)
{
    auto it = students.find(name);
    return (it != students.end()) ? std::optional<int>(it->second) : std::nullopt;
}

std::optional<std::string> readFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return std::nullopt; // 如果文件打开失败，返回空值
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

void processOptional()
{
    // 创建一个包含学生信息的映射
    std::map<std::string, int> students = {{"Alice", 85}, {"Bob", 90}, {"Charlie", 78}};

    // 使用 findStudent 查找 Alice 的成绩
    std::optional<int> opt_score = findStudent(students, "Alice");

    // 检查是否找到了 Alice 的成绩
    if (opt_score)
        std::cout << "Alice's score is: " << *opt_score << std::endl; // 使用解引用访问值
    else
        std::cout << "Alice's score not found." << std::endl;

    // 使用 value_or 方法提供默认值
    int score = opt_score.value_or(0);
    std::cout << "Alice's score using value_or: " << score << std::endl;

    // 使用 emplace 修改 std::optional 中的值
    opt_score.emplace(95); // 修改成绩为 95
    std::cout << "Alice's new score: " << *opt_score << std::endl;

    // 使用 nullopt 重置 std::optional
    opt_score = std::nullopt;
    if (!opt_score)
        std::cout << "Alice's score has been reset." << std::endl;

    // 读取文件并返回内容作为 std::optional
    std::optional<std::string> file_content = readFile(R"(D:\Project_All\Cpp_Project\Mordern_CPP\test\example.txt)");
    if (file_content)
        std::cout << "File content: " << *file_content << std::endl;
    else
        std::cout << "Failed to read file." << std::endl;
}

int main()
{
    processOptional();
    return 0;
}
