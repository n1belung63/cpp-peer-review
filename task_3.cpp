/**********************
** 1. LoadPerson.cpp **
**********************/

struct DBParameters {
    string_view name;
    int connection_timeout;
    bool allow_exceptions;
    DBLogLevel log_level;
};

struct PersonFilterParameters {
    int min_age;
    int max_age;
    string_view name_filter;
};

vector<Person> LoadPersons(const DBParameters& db_pars, const PersonFilterParameters& person_pars) {
    DBConnector connector(db_pars.allow_exceptions, db_pars.log_level);

    DBHandler db = (db_pars.name.starts_with("tmp."s)) ? 
        connector.ConnectTmp(db_pars.name, db_pars.connection_timeout) : 
        connector.Connect(db_pars.name, db_pars.connection_timeout);

    if (!db_pars.allow_exceptions && !db.IsOK()) {
        return {};
    }

    const DBQuery query = [&person_pars, &db] {
        ostringstream query_str;
        query_str << "from Persons "s
                << "select Name, Age "s
                << "where Age between "s << person_pars.min_age << " and "s << person_pars.max_age << " "s
                << "and Name like '%"s << db.Quote(person_pars.name_filter) << "%'"s;
        return DBQuery(query_str.str());
    }(); 

    vector<Person> persons;
    for (auto [name, age] : db.LoadRows<string, int>(query)) {
        persons.push_back({move(name), age});
    }
    return persons;
}

/*********************************
** 2. CheckDateTimeValidity.cpp **
*********************************/

enum class DateTimeErrorCode {
    Ok,
    YearOutOfRange,
    MonthOutOfRange,
    DayOutOfRange,
    HourOutOfRange,
    MinuteOutOfRange,
    SecondOutOfRange
};

struct DateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

DateTimeErrorCode CheckDateTimeValidity(const DateTime& dt) {
    const int MAX_YEAR = 9999;
    const int MIN_YEAR = 1;

    const int MAX_MONTH = 12;
    const int MIN_MONTH = 1;

    const int MAX_MONTH = 12;
    const int MIN_MONTH = 1;

    auto MAX_DAY {
        [](int year, int month) {
            const bool is_leap_year = (year % 4 == 0) && !(year % 100 == 0 && year % 400 != 0);
            const array month_lengths = {31, 28 + is_leap_year, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            return month_lengths[month - 1];
        }
    };
    const int MIN_DAY = 1;

    const int MAX_HOUR = 23;
    const int MIN_HOUR = 0;

    const int MAX_MINUTE = 59;
    const int MIN_MINUTE = 0;

    const int MAX_SECOND = 59;
    const int MIN_SECOND = 0;

    if (dt.year < MIN_YEAR || dt.year > MAX_YEAR) {
        return DateTimeErrorCode::YearOutOfRange;
    }

    if (dt.month < MIN_MONTH || dt.month > MAX_MONTH) {
        return DateTimeErrorCode::MonthOutOfRange;
    }

    if (dt.day < MIN_DAY || dt.day > MAX_DAY(dt.year, dt.month)) {
        return DateTimeErrorCode::DayOutOfRange;
    }

    if (dt.hour < MIN_HOUR || dt.hour > MAX_HOUR) {
        return DateTimeErrorCode::HourOutOfRange;
    }

    if (dt.minute < MIN_MINUTE || dt.minute > MAX_MINUTE) {
        return DateTimeErrorCode::MinuteOutOfRange;
    }

    if (dt.second < MIN_SECOND || dt.second > MAX_SECOND) {
        return DateTimeErrorCode::SecondOutOfRange;
    }

    return DateTimeErrorCode::Ok;
}

/****************************
** 3. ParseCitySubjson.cpp **
****************************/

void ParseCitySubjson(const Json& json, const Country& country, vector<City>& cities) {
    for (const auto& city_json : json.AsList()) {
        const auto& city_obj = city_json.AsObject();
        cities.push_back({
            city_obj["name"s].AsString(), 
            city_obj["iso_code"s].AsString(),
            country.phone_code + city_obj["phone_code"s].AsString(),
            country.name,
            country.iso_code,
            country.time_zone,
            country.languages
        });
    }
}

// ParseCitySubjson вызывается только из функции ParseCountryJson следующим образом:
void ParseCountryJson(vector<Country>& countries, vector<City>& cities, const Json& json) {
    for (const auto& country_json : json.AsList()) {
        const auto& country_obj = country_json.AsObject();
        countries.push_back({
            country_obj["name"s].AsString(),
            country_obj["iso_code"s].AsString(),
            country_obj["phone_code"s].AsString(),
            country_obj["time_zone"s].AsString(),
        });
        Country& country = countries.back();
        for (const auto& lang_obj : country_obj["languages"s].AsList()) {
            country.languages.push_back(FromString<Language>(lang_obj.AsString()));
        }
        ParseCitySubjson(country_obj["cities"s], country, cities);
    }
}
