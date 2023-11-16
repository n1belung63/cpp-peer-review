#include <iostream>
#include <array>
#include <string>
#include <iomanip>
#include <variant>
#include <sstream>
#include <optional>

using namespace std::string_literals;

class EbookManager {
public:
    struct ReadRequest {
        int user;
        int page;
    };

    struct CheerRequest {
        int user;
    };

    int ReadRequestCount(std::istream& in_stream) {
        int req_count;
        in_stream >> req_count;
        return req_count;
    }

    using Request = std::variant<std::monostate, ReadRequest, CheerRequest>;
    Request ParseRequest(std::istream& in_stream) {
        std::string temp_str;
        int user, page;
        in_stream >> temp_str;
        if (temp_str == "READ"s) {
            in_stream >> user >> page;
            return ReadRequest{user, page};
        }
        else if (temp_str == "CHEER"s) {
            in_stream >> user;
            return CheerRequest{user};
        }
        else {
            return std::monostate{};
        }
    }

    std::optional<std::string> PerformRequest(Request request) {
        std::stringstream ss;
        if (std::holds_alternative<ReadRequest>(request)) {
            UpdateReadInfo(std::get<ReadRequest>(request));
            return std::nullopt;
        }
        else if (std::holds_alternative<CheerRequest>(request)) {
            ss << CalculateFractionWhoReadLess(std::get<CheerRequest>(request));
            return ss.str();
        }
        else {
            return std::nullopt;
        }     
    }

    void PrintResponse(const std::optional<std::string>& out_str) {
        if (out_str.has_value()) {
            std::cout << std::setprecision(6) << out_str.value() << std::endl;
        }
    }

private:
    std::array<int,100001> user_to_page_ = {0};
    std::array<int,1001> page_to_user_count_ = {0};
    int user_count_ = 0;
 
    void UpdateReadInfo(const ReadRequest& user_info) {
        if (user_to_page_[user_info.user] == 0) {
            user_count_++;
        }

        for (int p=user_to_page_[user_info.user]+1; p<=user_info.page; ++p) {
            page_to_user_count_[p]++;
        }

        user_to_page_[user_info.user] = user_info.page;
    }

    double CalculateFractionWhoReadLess(const CheerRequest& user_info) {
        if (user_count_ == 0 || user_to_page_.at(user_info.user) == 0) {
            return 0;
        }

        if (user_count_ == 1) {
            return 1;
        }

        const int user_count_with_the_same_page_read = page_to_user_count_.at(user_to_page_.at(user_info.user));
        return (user_count_ - user_count_with_the_same_page_read) / static_cast<double>(user_count_ - 1);
    }
};

int main() {
    EbookManager em;

    const int request_count = em.ReadRequestCount(std::cin);
    for (int i=0; i<request_count; ++i) {
        em.PrintResponse(em.PerformRequest(em.ParseRequest(std::cin)));
    }
}