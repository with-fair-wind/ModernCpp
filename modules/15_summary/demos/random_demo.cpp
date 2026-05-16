// `<random>`：质量可控的随机数生成，区别于 C 接口 `rand`/`srand`。

#include <iostream>
#include <random>
#include <vector>

namespace {

void printInts(char const* label, std::vector<int> const& v) {
    std::cout << label << ":";
    for (int x : v) {
        std::cout << ' ' << x;
    }
    std::cout << '\n';
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    std::cout << "--- std::random_device（操作系统熵源，可能较慢）---\n";
    std::random_device rd;
    std::cout << "示例值: " << rd() << ", " << rd() << ", " << rd() << '\n';

    std::cout << "\n--- 引擎固定种子时可复现序列 ---\n";
    constexpr unsigned kSeed42 = 42U;
    std::mt19937 rng_a{
        kSeed42};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::mt19937 rng_b{
        kSeed42};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::cout << "mt19937 同种子前 5 个值是否一致: ";
    bool same = true;
    for (int i = 0; i < 5; ++i) {
        same = same && (rng_a() == rng_b());
    }
    std::cout << (same ? "是" : "否") << '\n';

    std::cout << "\n--- std::seed_seq 混匀多路种子 ---\n";
    std::seed_seq seed_seq{1, 2, 3, 4, 5};
    std::mt19937 rng_mixed{seed_seq};
    std::cout << "seed_seq 后 mt19937 首个值: " << rng_mixed() << '\n';

    std::cout << "\n--- minstd_rand（轻量线性同余）---\n";
    std::minstd_rand lcg{
        kSeed42};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::cout << "minstd_rand 前 3 个值: " << lcg() << ' ' << lcg() << ' ' << lcg() << '\n';

    std::cout << "\n--- default_random_engine（实现定义）---\n";
    std::default_random_engine def{
        kSeed42};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::cout << "default_random_engine 首个值: " << def() << '\n';

    std::cout << "\n--- 分布：均匀整数 [a,b] ---\n";
    std::mt19937 gen{
        kSeed42};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::uniform_int_distribution<int> uni_i{1, 6};
    constexpr int kDiceRolls = 10;
    std::vector<int> dice;
    dice.reserve(kDiceRolls);
    for (int i = 0; i < kDiceRolls; ++i) {
        dice.push_back(uni_i(gen));
    }
    printInts("骰子", dice);

    std::cout << "\n--- 均匀实数 [a,b) ---\n";
    std::uniform_real_distribution<double> uni_r{-1.0, 1.0};
    std::cout << "[-1,1) 样本: " << uni_r(gen) << ", " << uni_r(gen) << '\n';

    std::cout << "\n--- 正态分布 ---\n";
    std::normal_distribution<double> normal{100.0, 15.0};
    std::cout << "N(100,15) 样本: " << normal(gen) << ", " << normal(gen) << '\n';

    std::cout << "\n--- bernoulli_distribution（bool/Bernoulli）---\n";
    std::bernoulli_distribution coin{0.3};
    int heads = 0;
    constexpr int kTrials = 20;
    for (int i = 0; i < kTrials; ++i) {
        if (coin(gen)) {
            ++heads;
        }
    }
    std::cout << kTrials << " 次抛掷中 true 次数: " << heads << '\n';

    std::cout << "\n--- 不推荐 rand()/srand() ---\n";
    std::cout << "`rand()` 通常质量差、上限 `RAND_MAX` 可能过小且线程不安全；改用 `<random>` "
                 "可明确引擎/分布与统计语义。\n";

    return 0;
}
