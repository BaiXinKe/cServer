#include <memory>
#include <set>
#include <vector>

int main(int argc, char** argv)
{
    using intPtr = std::unique_ptr<int>;
    using intAndPtr = std::pair<int, intPtr>;

    std::vector<intPtr> intArray;
    intAndPtr ptr = std::make_pair(1, std::make_unique<int>(1));

    std::set<intAndPtr> ptrs;
    ptrs.insert(std::move(ptr));
}