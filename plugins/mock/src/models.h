#pragma once

#include <nlohmann/json.hpp>

// example enum type declaration
//enum TaskState {
//    TS_STOPPED,
//    TS_RUNNING,
//    TS_COMPLETED,
//    TS_INVALID = -1,
//};
//
//// map TaskState values to JSON as strings
//NLOHMANN_JSON_SERIALIZE_ENUM(TaskState, {
//    {TS_INVALID, nullptr},
//    {TS_STOPPED, "stopped"},
//    {TS_RUNNING, "running"},
//    {TS_COMPLETED, "completed"},
//    })

namespace bridge {
    struct tick {
        std::string symbol;
        // add other fields as needed
    };

    inline void to_json(nlohmann::json& j, const bridge::tick& t) {
        j = nlohmann::json{
            {"symbol", t.symbol}
            // add other fields as needed
        };
    }
}