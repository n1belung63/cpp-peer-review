#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>
#include <set>

using namespace std;

class Domain {
private:
    string name_;
    string inv_name_;
public:
    explicit Domain(const string& name) : name_(name), inv_name_(PrepareInverseName(name)) { }

    bool operator==(const Domain& rhs) const {
        return lexicographical_compare(
            inv_name_.begin(),
            inv_name_.end(),
            rhs.inv_name_.begin(),
            rhs.inv_name_.end()
        );
    }

    [[nodiscard]] bool IsSubdomain(const Domain& domain) const {
        if (inv_name_.size() < domain.inv_name_.size()) {
            return false;
        }
        
        return equal(domain.inv_name_.begin(), domain.inv_name_.end(), inv_name_.begin());
    }

    [[nodiscard]] const string& GetName() const {
        return name_;
    }

    [[nodiscard]] const string& GetInverseName() const {
        return inv_name_;
    }
    
private:
    string PrepareInverseName(const string& name) {
        return string(name.rbegin(),name.rend()) + "."s;
    }
    
};

class DomainChecker {
public:
    template <typename InputIt>
    DomainChecker(InputIt beg, InputIt end) : domains_(beg, end) {
        sort(domains_.begin(), domains_.end(), 
			[](const Domain& left, const Domain& right) {
                return left == right;
            });

        domains_.erase(
            unique (
                domains_.begin(),
                domains_.end(),
                [](const Domain& left, const Domain& right) {
                    return right.IsSubdomain(left);
                } ),
                domains_.end()
            );
    }

    bool IsForbidden(Domain domain) {
        if (domains_.empty()) {
            return false;
        }

        auto upper = upper_bound(domains_.begin(), domains_.end(), domain, 
            [](const Domain& left, const Domain& right) {
                return left == right;
            });

        if (upper != domains_.end() && domain.IsSubdomain(*upper)) {
            return true;
        }
        
        if (upper != domains_.begin()) {
            return domain.IsSubdomain(*prev(upper));
        }
        else {
            return false;
        }
    }

    vector<Domain> GetDomains() const {
        return domains_;
    }

private:
    vector<Domain> domains_;
};

template <typename Number>
Number ReadNumberOnLine(istream& input) {
    string line;
    getline(input, line);

    Number num;
    std::istringstream(line) >> num;

    return num;
}

vector<Domain> ReadDomains(istream& input, size_t domain_count) {
    string line;
    vector<Domain> domains;
    domains.reserve(domain_count);

    for (size_t i = 0; i < domain_count; ++i) {
        getline(input, line);
        domains.push_back(Domain(line));
    }
    return domains;
}

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line, const std::string& hint) {
    if (!value) {
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename Func>
void RunTestImpl(const Func& func, const std::string& func_name) {
    func();
    std::cerr << func_name << " OK"s << std::endl;
}

#define RUN_TEST(func)  RunTestImpl((func), #func)

void TestIsSubdomain() {
    Domain domain("gdz.ru"s);

    vector<Domain> domains;
    domains.push_back(Domain("math.gdz.ru"s));
    domains.push_back(Domain("history.gdz.ru"s));
    domains.push_back(Domain("biology.gdz.ru"s));
    domains.push_back(Domain("freegdz.ru"s));

    for (size_t i=0; i<3; ++i) {
        ASSERT_HINT(domains[i].IsSubdomain(domain), "Should be true"s);
    }
    ASSERT_HINT(!domains[3].IsSubdomain(domain), "Should be false"s);
}

void TestNoForbiddenDomains() {
    vector<Domain> forbidden_domains;

    vector<Domain> test_domains;
    test_domains.push_back(Domain("gdz.ru"s));
    test_domains.push_back(Domain("gdz.com"s));
    test_domains.push_back(Domain("m.maps.me"s));
    test_domains.push_back(Domain("alg.m.gdz.ru"s));
    test_domains.push_back(Domain("maps.com"s));
    test_domains.push_back(Domain("maps.ru"s));
    test_domains.push_back(Domain("gdz.ua"s));

    vector<int> true_results = { 0, 0, 0, 0, 0, 0, 0 };

    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

    for (size_t i=0; i<test_domains.size(); ++i) {
        string msg = "Error is in "s + test_domains.at(i).GetName() + " domain"s;
        ASSERT_HINT(true_results.at(i) == static_cast<int>(checker.IsForbidden(test_domains.at(i))), msg);
    }
}

void TestDomainCheckerInit() {
    vector<Domain> forbidden_domains;
    forbidden_domains.push_back(Domain("gdz.ru"s));
    forbidden_domains.push_back(Domain("maps.me"s));
    forbidden_domains.push_back(Domain("m.gdz.ru"s));
    forbidden_domains.push_back(Domain("com"s));

    vector<string> forbidden_domains_after_init_names = { "maps.me"s, "com"s, "gdz.ru"s };

    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());
    vector<Domain> forbidden_domains_after_init = checker.GetDomains();

    ASSERT_HINT(forbidden_domains_after_init.size() == forbidden_domains_after_init_names.size(), "Should be equal"s);  
    for (size_t i = 0; i < forbidden_domains_after_init.size(); ++i) {
        ASSERT_HINT(forbidden_domains_after_init.at(i).GetName() == forbidden_domains_after_init_names.at(i), "Should be equal"s);  
    }
}

void TestIsForbidden() {
    {
        vector<Domain> forbidden_domains;
        forbidden_domains.push_back(Domain("gdz.ru"s));
        forbidden_domains.push_back(Domain("maps.me"s));
        forbidden_domains.push_back(Domain("m.gdz.ru"s));
        forbidden_domains.push_back(Domain("com"s));

        vector<Domain> test_domains;
        test_domains.push_back(Domain("gdz.ru"s));
        test_domains.push_back(Domain("gdz.com"s));
        test_domains.push_back(Domain("m.maps.me"s));
        test_domains.push_back(Domain("alg.m.gdz.ru"s));
        test_domains.push_back(Domain("maps.com"s));
        test_domains.push_back(Domain("maps.ru"s));
        test_domains.push_back(Domain("gdz.ua"s));

        vector<int> true_results = { 1, 1, 1, 1, 1, 0, 0 };

        DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

        for (size_t i=0; i<test_domains.size(); ++i) {
            string msg = "Error is in "s + test_domains.at(i).GetName() + " domain"s;
            ASSERT_HINT(true_results.at(i) == static_cast<int>(checker.IsForbidden(test_domains.at(i))), msg);
        }
    }

    {
        vector<Domain> forbidden_domains;
        forbidden_domains.push_back(Domain("aa.aaaa.a.a.a.aaaa.a"s));
        forbidden_domains.push_back(Domain("aa"s));
        forbidden_domains.push_back(Domain("a.aa.aaaa.a.a.a.aaaa.a"s));
        forbidden_domains.push_back(Domain("a.aa"s));
        forbidden_domains.push_back(Domain("a.aa.aaaa.aa.a.aa"s));
        forbidden_domains.push_back(Domain("aaaa.a.a.aaaa.a"s));
        forbidden_domains.push_back(Domain("aa"s));
        forbidden_domains.push_back(Domain("aaaa.a.a.a.aaaa.a"s));
        forbidden_domains.push_back(Domain("aa.aaaa.a.a.a.aaaa.a"s));
        forbidden_domains.push_back(Domain("aaaa.aa.aaa.aaaa.a"s));

        vector<Domain> test_domains;
        test_domains.push_back(Domain("aaa.aaaa"s));
        test_domains.push_back(Domain("aaa"s));

        vector<int> true_results = { false, false };

        DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

        for (size_t i=0; i<test_domains.size(); ++i) {
            string msg = "Error is in "s + test_domains.at(i).GetName() + " domain"s;
            ASSERT_HINT(true_results.at(i) == checker.IsForbidden(test_domains.at(i)), msg);
        }

    }
}

void TestDomainChecker() {
    RUN_TEST(TestIsSubdomain);
    RUN_TEST(TestNoForbiddenDomains);
    RUN_TEST(TestDomainCheckerInit);
    RUN_TEST(TestIsForbidden);
}

int main() {
    TestDomainChecker();

    const std::vector<Domain> forbidden_domains = ReadDomains(cin, ReadNumberOnLine<size_t>(cin));
    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

    const std::vector<Domain> test_domains = ReadDomains(cin, ReadNumberOnLine<size_t>(cin));
    for (const Domain& domain : test_domains) {
         cout << (checker.IsForbidden(domain) ? "Bad"sv : "Good"sv) << endl;
    }
}
 