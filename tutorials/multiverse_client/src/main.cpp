#include "my_multiverse_client.cpp"
#include <fstream>

static MyMultiverseClient &my_multiverse_client = MyMultiverseClient::get_instance();
bool should_shut_down = false;

int main(int argc, char **argv)
{
    // register signal SIGINT and signal handler
    signal(SIGINT, [](int)
           {
        printf("Caught SIGINT (Ctrl+C), wait for 1s then shutdown.\n");
        should_shut_down = true; 
        std::this_thread::sleep_for(std::chrono::seconds(1)); });

    printf("Start MyMultiverseClient\n");

    if (argc < 2)
    {
        printf("USAGE: main multiverse_params.json\n");
        return 1;
    }

    const std::string params_file_path = argv[1];
    std::ifstream params_file(params_file_path);
    // confirm file opening
    if (!params_file.is_open()) {
        // print error message and return
        printf("Error opening file %s\n", params_file_path.c_str());
        return 1;
    }

    // Read the file line by line into a string
    std::string params_str = "";
    std::string line;
    while (std::getline(params_file, line)) {
        params_str.append(line);
    }

    // Close the file
    params_file.close();

    Json::Value params_json;
    Json::Reader reader;
    printf("Reading parameters from %s...\n", params_file_path.c_str());
    if (!reader.parse(params_str, params_json) || params_json.empty())
    {
        printf("Failed to parse parameters from %s\n", params_file_path.c_str());
        return 1;
    }

    printf("Received parameters from %s\n", params_file_path.c_str());
    printf("%s\n", params_json.toStyledString().c_str());

    my_multiverse_client.init(params_json);
    my_multiverse_client.start();

    while (!should_shut_down)
    {
        my_multiverse_client.communicate();
    }

    my_multiverse_client.disconnect();

    return 0;
}