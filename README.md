# NominatimAPI
api for nominatim.org site-tool to search OSM data by name and address and to generate synthetic addresses of OSM points (reverse geocoding)
# main
```cpp
#include "NominatimAPI.h"
#include <iostream>

int main() {
   NominatimAPI api;
    auto status = api.get_status().then([](json::value result) {
        std::cout << result<< std::endl;
    });
    status.wait();
    
    return 0;
}
```

# Launch (your script)
```
g++ -std=c++11 -o main main.cpp -lcpprest -lssl -lcrypto -lpthread -lboost_system -lboost_chrono -lboost_thread
./main
```
