#include "nanobench_html_graph_doctest_main.hpp"
#include "rng.hpp"
#include <vector>

#if __has_include(<boost/align/aligned_allocator.hpp>)
#  include <boost/align/aligned_allocator.hpp>

#  if defined(LEVEL1_DCACHE_LINESIZE)
constexpr std::size_t hardware_constructive_interference_size = LEVEL1_DCACHE_LINESIZE;
#  elif defined(__cpp_lib_hardware_interference_size)
constexpr std::size_t hardware_constructive_interference_size =
        std::hardware_constructive_interference_size;
#  else
constexpr std::size_t hardware_constructive_interference_size = 64;
#  endif

template <typename T, std::size_t alignment = hardware_constructive_interference_size>
using test_vector = std::vector<T, boost::alignment::aligned_allocator<T, alignment>>;

#else
template <typename T>
using test_vector = std::vector<T>;
#endif

constexpr auto avail_L1 = LEVEL1_DCACHE_SIZE / 8;
constexpr auto avail_L2 = LEVEL2_DCACHE_SIZE / 8;

template <typename T, typename  Func>
void compute(test_vector<T> const& a, test_vector<T> const& b, test_vector<T>& out, Func op)
{
    for (std::size_t i = 0; i < a.size(); ++i) {
        out[i] = op(a[i], b[i]);
    }
}

template <typename T>
test_vector<T> random_vector(int count)
{
    test_vector<T> v;
    v.reserve(count);
  for (auto i : RNG<T>(1, 1'000'000, count)) {
      v.push_back(i);
  }
  return v;
}

template <typename T, typename Func>
void bench_arite2(ankerl::nanobench::Bench& bench, char const* name, int const bytes, Func op)
{
  int const count = bytes / sizeof(T);
  auto const x = random_vector<T>(count);
  auto const y = random_vector<T>(count);
  test_vector<T> z(count);

  bench.run(name, [&]() {
          op(x, y, z);
          ankerl::nanobench::doNotOptimizeAway(z);
          });
}

TEST_CASE("mult/div float L1")
{
    ankerl::nanobench::Bench b;
    b.title("mult/div float L1")
        .unit("int")
        .warmup(100)
        .minEpochIterations(100'000)
        .relative(true);
    b.performanceCounters(true);

    bench_arite2<float>(
            b, "/ L1", avail_L1,
            [](test_vector<float> const& a, test_vector<float> const& b, test_vector<float>& out) {
                compute(a, b, out, [](auto l, auto r) { return l / r; });
            }
    );

    bench_arite2<float>(
            b, "/ L2", avail_L2,
            [](test_vector<float> const& a, test_vector<float> const& b, test_vector<float>& out) {
                compute(a, b, out, [](auto l, auto r) { return l / r; });
            }
    );

    bench_arite2<float>(
            b, "* L1", avail_L1,
            [](test_vector<float> const& a, test_vector<float> const& b, test_vector<float>& out) {
                compute(a, b, out, [](auto l, auto r) { return l * r; });
            }
    );

    bench_arite2<float>(
            b, "* L2", avail_L2,
            [](test_vector<float> const& a, test_vector<float> const& b, test_vector<float>& out) {
                compute(a, b, out, [](auto l, auto r) { return l * r; });
            }
    );

    render_graph(b, "mult/div float L1");
}
