// `<random>`：引擎可复现性与分布契约（含简单统计）。

#include <cmath>
#include <random>
#include <type_traits>

#include <gtest/gtest.h>

TEST(Random, Mt19937SameSeedSameSequence) {
    constexpr unsigned kSeed = 2026U;
    std::mt19937 a{kSeed};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::mt19937 b{kSeed};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    for (int i = 0; i < 32; ++i) {
        EXPECT_EQ(a(), b());
    }
}

TEST(Random, UniformIntClosedInterval) {
    std::mt19937 gen{11U};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::uniform_int_distribution<int> dist{-5, 5};
    for (int i = 0; i < 1000; ++i) {
        int const x = dist(gen);
        EXPECT_GE(x, -5);
        EXPECT_LE(x, 5);
    }
}

TEST(Random, UniformRealHalfOpenInterval) {
    std::mt19937 gen{7U};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::uniform_real_distribution<double> dist{2.0, 3.0};
    for (int i = 0; i < 5000; ++i) {
        double const x = dist(gen);
        EXPECT_GE(x, 2.0);
        EXPECT_LT(x, 3.0);
    }
}

TEST(Random, NormalDistributionMeanApproximate) {
    constexpr double kMean = 10.0;
    constexpr double kStdDev = 2.0;
    constexpr int kSamples = 80'000;
    std::mt19937 gen{99U};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::normal_distribution<double> dist{kMean, kStdDev};
    double sum = 0.0;
    for (int i = 0; i < kSamples; ++i) {
        sum += dist(gen);
    }
    double const sample_mean = sum / static_cast<double>(kSamples);
    double const expected_error = 4.0 * kStdDev / std::sqrt(static_cast<double>(kSamples));
    EXPECT_NEAR(sample_mean, kMean, expected_error);
}

TEST(Random, BernoulliReturnsBoolResultType) {
    static_assert(std::is_same_v<std::bernoulli_distribution::result_type, bool>);
    std::mt19937 gen{3U};  // NOLINT(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
    std::bernoulli_distribution dist{0.4};
    bool ok = dist(gen);
    EXPECT_TRUE(ok || !ok);
}

TEST(Random, RandomDeviceProducesValues) {
    std::random_device rd;
    EXPECT_NO_THROW(static_cast<void>(rd()));
}
