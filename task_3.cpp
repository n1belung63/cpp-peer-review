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

    const DBQuery query = std::invoke([&person_pars, &db] {
        ostringstream query_str;
        query_str << "from Persons "s
                << "select Name, Age "s
                << "where Age between "s << person_pars.min_age << " and "s << person_pars.max_age << " "s
                << "and Name like '%"s << db.Quote(person_pars.name_filter) << "%'"s;
        return DBQuery(query_str.str());
    });

    vector<Person> persons;
    for (auto [name, age] : db.LoadRows<string, int>(query)) {
        persons.push_back({move(name), age});
    }
    return persons;
}

/*********************************
** 2. CheckDateTimeValidity.cpp **
*********************************/

struct DateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};


void CheckDateTimeFieldValidity(const std::string& dt_name, int dt, int dt_min, int dt_max) {
    if (dt < dt_min) {
        throw domain_error(dt_name + " is too small"s);
    }

    if (dt > dt_max) {
        throw domain_error(dt_name + " is too big"s);
    }
}

void CheckDateTimeValidity(const DateTime& dt) {
    static const int MAX_YEAR = 9999;
    static const int MIN_YEAR = 1;

    static const int MAX_MONTH = 12;
    static const int MIN_MONTH = 1;

    auto MAX_DAY {
        [](int year, int month) {
            const bool is_leap_year = (year % 4 == 0) && !(year % 100 == 0 && year % 400 != 0);
            const std::array month_lengths = {31, 28 + is_leap_year, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            return month_lengths[month - 1];
        }
    };
    static const int MIN_DAY = 1;

    static const int MAX_HOUR = 23;
    static const int MIN_HOUR = 0;

    static const int MAX_MINUTE = 59;
    static const int MIN_MINUTE = 0;

    static const int MAX_SECOND = 59;
    static const int MIN_SECOND = 0;

    CheckDateTimeFieldValidity("year"s, dt.year, MIN_YEAR, MAX_YEAR);
    CheckDateTimeFieldValidity("month"s, dt.month, MIN_MONTH, MAX_MONTH);
    CheckDateTimeFieldValidity("day"s, dt.day, MIN_DAY, MAX_DAY(dt.year, dt.month));
    CheckDateTimeFieldValidity("hour"s, dt.hour, MIN_HOUR, MAX_HOUR);
    CheckDateTimeFieldValidity("minute"s, dt.minute, MIN_MINUTE, MAX_MINUTE);
    CheckDateTimeFieldValidity("second"s, dt.second, MIN_SECOND, MAX_SECOND);
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
