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
constexpr size_t default_capacity { 3u };

class RoundRobin {
public:
    RoundRobin(size_t capacity = default_capacity)
        : _set_indexer { capacity }
    {
        _array.reserve(capacity);
    }

    void Set(t_resource resource, const t_indexer& indexer) {
        if (_array.size() < _array.capacity()) {
            _array.emplace_back(resource);
            return;
        }

        _array[ restrict_index(indexer) ] = resource;
    }

    void Set(t_resource resource) {
        Set(resource, _set_indexer);
    }

    t_resource Get(const t_indexer& indexer) const {
        if (!_array.size()) {
            throw std::runtime_error { "array size can not be empty to getting resource" };
        }

        return _array[ restrict_index(indexer) ];
    }

protected:
    inline size_t restrict(size_t index) const {
        return index % _array.size();
    }

    inline size_t restrict_index(const t_indexer& indexer) const {
        const size_t index = indexer.index();
        return restrict(index);
    }

protected:
    std::vector<t_resource> _array;

    forward_indexer _set_indexer;
};


class TestRoundRobin : public RoundRobin {
public:
    const size_t capacity() const {
        return _array.capacity();
    }
};


bool test_empty(const RoundRobin& rr,
                const t_indexer& to_get);

bool test_set_get(TestRoundRobin& rr,
                  const size_t capacity,
                  const t_indexer& to_get,
                  const std::vector<int>& resources,
                  const std::vector<int>& must_be);

bool test_indexer(const t_indexer& indexer,
                  const std::vector<size_t>& must_be);

template <typename t_testable, typename... t_arguments>
void test(const std::string& message, t_testable testable, t_arguments... arguments) {
    std::cout << message << (testable(arguments ...) ? " is OK" : " is FAILED") << std::endl;
}

template <typename t_value, typename t_step, typename t_steps = t_value>
std::vector<t_value> make_range(t_value since, t_step step, t_steps steps) {
    std::vector<t_value> range;
    const t_value till = (since + steps);
    for (t_value value = since; value < till; value += step) {
        range.emplace_back(value);
    }
    return range;
}


int main() {
    test("forward  indexer", test_indexer,
         forward_indexer {}, make_range(std::numeric_limits<size_t>::min(), +1, 25u));

    test("backward indexer", test_indexer,
         backward_indexer {}, make_range(std::numeric_limits<size_t>::max(), -1, 25u));

    test("getting from empty balancer", test_empty,
        RoundRobin {}, forward_indexer {} );

    test("adding to empty balancer then getting from balancer", test_set_get,
        TestRoundRobin {}, size_t { default_capacity }, forward_indexer {},
        std::vector<int> { 0, 1, 2, 3 },
        std::vector<int> { 3, 1, 2, 3, 1, 2, 3 }
        );

    return 0;
}

bool test_empty(const RoundRobin& rr, const t_indexer& to_get) {
    try {
        rr.Get(to_get);
    }
    catch (...) {
        return true;
    }

    return false;
}

bool test_set_get(TestRoundRobin& rr, const size_t capacity, const t_indexer& to_get,
                  const std::vector<int>& resources, const std::vector<int>& must_be) {
    if (capacity != rr.capacity()) {
        return false;
    }

    for (const int resource : resources) {
        rr.Set(resource);
    }

    bool result { true };
    for (const size_t resource : must_be) {
        result = result & (resource == rr.Get(to_get));
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
