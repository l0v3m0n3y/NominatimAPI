#ifndef NOMINATIM_API_H
#define NOMINATIM_API_H

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <pplx/pplx.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace web;
using namespace web::http;
using namespace web::http::client;

class NominatimAPI {
private:
    std::string api_base = "https://nominatim.openstreetmap.org";
    http_client_config client_config;
    
    http_request create_request(const std::string& endpoint, const std::string& method) {
        http_request request;
        
        if (method == "GET") {
            request.set_method(methods::GET);
        } else if (method == "POST") {
            request.set_method(methods::POST);
        } else if (method == "PUT") {
            request.set_method(methods::PUT);
        } else if (method == "DEL") {
            request.set_method(methods::DEL);
        }
        
        request.set_request_uri(utility::conversions::to_string_t(endpoint));
        
        // Set headers
        request.headers().add(U("Host"), U("nominatim.openstreetmap.org"));
        request.headers().add(U("Content-Type"), U("application/json"));
        request.headers().add(U("User-Agent"), U("Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0"));
        
        return request;
    }
    
    std::string build_query_params(const std::map<std::string, std::string>& params) {
        if (params.empty()) return "";
        
        std::string query = "?";
        bool first = true;
        for (const auto& param : params) {
            if (!param.second.empty()) {
                if (!first) query += "&";
                auto encoded_value = web::uri::encode_data_string(utility::conversions::to_string_t(param.second));
                query += param.first + "=" + utility::conversions::to_utf8string(encoded_value);
                first = false;
            }
        }
        return query;
    }
    
    pplx::task<json::value> make_api_call(const std::string& endpoint, const std::string& method) {
        http_client client(utility::conversions::to_string_t(api_base), client_config);
        auto request = create_request(endpoint, method);

        return client.request(request)
            .then([](http_response response) {
                if (response.status_code() == status_codes::OK) {
                    return response.extract_json();
                } else {
                    json::value error_obj;
                    error_obj[U("error")] = json::value::string(
                        U("HTTP Error: ") + utility::conversions::to_string_t(std::to_string(response.status_code())));
                    error_obj[U("success")] = json::value::boolean(false);
                    return pplx::task_from_result(error_obj);
                }
            })
            .then([](pplx::task<json::value> previousTask) {
                try {
                    return previousTask.get();
                } catch (const std::exception& e) {
                    json::value error_obj;
                    error_obj[U("error")] = json::value::string(
                        U("Exception: ") + utility::conversions::to_string_t(e.what()));
                    error_obj[U("success")] = json::value::boolean(false);
                    return error_obj;
                }
            });
    }

public:
    NominatimAPI() {
        client_config.set_validate_certificates(false);
    }

    pplx::task<json::value> search(
        const std::string& query,
        const std::string& addressdetails = "",
        const std::string& limit = "",
        const std::string& countrycodes = "",
        const std::string& viewbox = "",
        const std::string& bounded = "",
        const std::string& exclude_place_ids = "",
        const std::string& polygon = "",
        const std::string& email = "") {
        
        std::map<std::string, std::string> params;
        params["q"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(query)));
        params["format"] = "json";
        
        if (!addressdetails.empty()) params["addressdetails"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(addressdetails)));
        if (!limit.empty()) params["limit"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(limit)));
        if (!countrycodes.empty()) params["countrycodes"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(countrycodes)));
        if (!viewbox.empty()) params["viewbox"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(viewbox)));
        if (!bounded.empty()) params["bounded"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(bounded)));
        if (!exclude_place_ids.empty()) params["exclude_place_ids"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(exclude_place_ids)));
        if (!polygon.empty()) params["polygon"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(polygon)));
        if (!email.empty()) params["email"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(email)));
        
        return make_api_call("/search" + build_query_params(params), "GET");
    }

    pplx::task<json::value> reverse(
        double lat,
        double lon,
        const std::string& zoom = "",
        const std::string& addressdetails = "",
        const std::string& polygon = "",
        const std::string& email = "") {
        
        std::map<std::string, std::string> params;
        params["lat"] = std::to_string(lat);
        params["lon"] = std::to_string(lon);
        params["format"] = "json";
        
        if (!zoom.empty()) params["zoom"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(zoom)));
        if (!addressdetails.empty()) params["addressdetails"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(addressdetails)));
        if (!polygon.empty()) params["polygon"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(polygon)));
        if (!email.empty()) params["email"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(email)));
        
        return make_api_call("/reverse" + build_query_params(params), "GET");
    }

    pplx::task<json::value> lookup(
        const std::string& osm_ids,
        const std::string& addressdetails = "",
        const std::string& polygon = "",
        const std::string& email = "") {
        
        std::map<std::string, std::string> params;
        params["osm_ids"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(osm_ids)));
        params["format"] = "json";
        
        if (!addressdetails.empty()) params["addressdetails"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(addressdetails)));
        if (!polygon.empty()) params["polygon"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(polygon)));
        if (!email.empty()) params["email"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(email)));
        
        return make_api_call("/lookup" + build_query_params(params), "GET");
    }

    pplx::task<json::value> details(
        const std::string& place_id,
        const std::string& addressdetails = "",
        const std::string& polygon = "",
        const std::string& email = "") {
        
        std::map<std::string, std::string> params;
        params["place_id"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(place_id)));
        params["format"] = "json";
        
        if (!addressdetails.empty()) params["addressdetails"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(addressdetails)));
        if (!polygon.empty()) params["polygon"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(polygon)));
        if (!email.empty()) params["email"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(email)));
        
        return make_api_call("/details" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_status() {
        std::map<std::string, std::string> params;
        params["format"] = "json";
        return make_api_call("/status" + build_query_params(params), "GET");
    }
};

#endif
