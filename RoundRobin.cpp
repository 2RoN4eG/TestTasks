#include <vector>
#include <iostream>
#include <functional>
#include <limits>


class t_indexer {
public:
    virtual ~t_indexer() = 0;

    virtual size_t index() const = 0;
};

t_indexer::~t_indexer() {
}


class forward_indexer : public t_indexer {
public:
    forward_indexer(size_t since = std::numeric_limits<size_t>::min())
        : _index { since }
    { }

    ~forward_indexer() override { }

    size_t index() const override {
        return _index ++;
    }

protected:
    mutable size_t _index {};
};


class backward_indexer : public t_indexer {
public:
    backward_indexer(size_t since = std::numeric_limits<size_t>::max())
        : _index { since }
    { }

    ~backward_indexer() override { }

    size_t index() const override {
        return _index --;
    }

protected:
    mutable size_t _index {};
};


using t_resource = int;

class RoundRobin {
public:
    RoundRobin(size_t capacity = 3u)
        : _indexer { capacity }
    {
        _array.reserve(capacity);
    }

    void AddResource(t_resource resource, const t_indexer& indexer) {
        if (_array.size() < _array.capacity()) {
            _array.emplace_back(resource);
            return;
        }

        const size_t index = indexer.index();
        const size_t restricted = restrict(index);

        _array[restricted] = resource;
    }

    void AddResource(t_resource resource) {
        AddResource(resource, _indexer);
    }

    t_resource GetResource(const t_indexer& indexer) const {
        if (!_array.size()) {
            throw std::runtime_error { "array size can not be empty to getting resource" };
        }

        const size_t index = indexer.index();
        const size_t restricted = restrict(index);

        return _array[restricted];
    }

protected:
    inline size_t restrict(size_t index) const {
        return index % _array.size();
    }

protected:
    std::vector<t_resource> _array;

    forward_indexer _indexer;
};


class TestRoundRobin : public RoundRobin {
public:
    const size_t capacity() const {
        return _array.capacity();
    }
};


bool test_empty_balancer(const RoundRobin& rr, const t_indexer& indexer);
bool test_adding_getting(TestRoundRobin& rr, const size_t capacity, const t_indexer& indexer, const std::vector<int>& resources, const std::vector<int>& must_be);
bool test_indexer(const t_indexer& indexer, const std::vector<size_t>& must_be);


template <typename t_testable, typename... t_arguments>
void test(const std::string& message, t_testable testable, t_arguments... arguments) {
    std::cout << message << (testable(arguments ...) ? " is OK" : " is FAILED") << std::endl;
}


int main() {
    test("forward  indexer", test_indexer, forward_indexer {}, std::vector<size_t> {
        std::numeric_limits<size_t>::min() + 0,
        std::numeric_limits<size_t>::min() + 1,
        std::numeric_limits<size_t>::min() + 2,
        std::numeric_limits<size_t>::min() + 3,
        std::numeric_limits<size_t>::min() + 4,
        std::numeric_limits<size_t>::min() + 5,
        std::numeric_limits<size_t>::min() + 6,
        std::numeric_limits<size_t>::min() + 7,
        std::numeric_limits<size_t>::min() + 8,
        std::numeric_limits<size_t>::min() + 9
    });

    test("backward indexer", test_indexer, backward_indexer {}, std::vector<size_t> {
        std::numeric_limits<size_t>::max() - 0,
        std::numeric_limits<size_t>::max() - 1,
        std::numeric_limits<size_t>::max() - 2,
        std::numeric_limits<size_t>::max() - 3,
        std::numeric_limits<size_t>::max() - 4,
        std::numeric_limits<size_t>::max() - 5,
        std::numeric_limits<size_t>::max() - 6,
        std::numeric_limits<size_t>::max() - 7,
        std::numeric_limits<size_t>::max() - 8,
        std::numeric_limits<size_t>::max() - 9
    });

    test("getting from empty balancer",
        test_empty_balancer,
        RoundRobin {},
        forward_indexer {}
        );

    test("adding to empty balancer then getting from balancer",
        test_adding_getting,
        TestRoundRobin {},
        size_t { 3u },  // mandatory for test data and results
        forward_indexer {},
        std::vector<int> { 0, 1, 2, 3 },
        std::vector<int> { 3, 1, 2, 3, 1, 2, 3 }
        );

    return 0;
}

bool test_empty_balancer(const RoundRobin& rr, const t_indexer& indexer) {
    try {
        rr.GetResource(indexer);
    }
    catch (...) {
        return true;
    }

    return false;
}

bool test_adding_getting(TestRoundRobin& rr, const size_t capacity, const t_indexer& indexer, const std::vector<int>& resources, const std::vector<int>& must_be) {
    if (capacity != rr.capacity()) {
        return false;
    }

    for (const int resource : resources) {
        rr.AddResource(resource);
    }

    bool result { true };
    for (const size_t resource : must_be) {
        result = result & (resource == rr.GetResource(indexer));
    }

    return result;
}

bool test_indexer(const t_indexer& indexer, const std::vector<size_t>& must_be) {
    bool result { true };
    for (const size_t index : must_be) {
        result = result & (index == indexer.index());
    }

    return result;
}
