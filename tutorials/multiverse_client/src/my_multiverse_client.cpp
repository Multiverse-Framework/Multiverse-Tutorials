#include "multiverse_client_json.h"
#include <set>
#include <thread>
#include <csignal>
#include <cstring>

class MyMultiverseClient final : public MultiverseClientJson
{
public:
    MyMultiverseClient(const MyMultiverseClient &) = delete;

    void operator=(MyMultiverseClient const &) = delete;

    static MyMultiverseClient &get_instance()
    {
        static MyMultiverseClient mj_multiverse_client;
        return mj_multiverse_client;
    }

public:
    void init(const Json::Value &multiverse_params_json)
    {
        std::map<std::string, std::string> multiverse_params;

        const Json::Value &multiverse_server_params_json = multiverse_params_json["multiverse_server"];
        multiverse_params["server_host"] = multiverse_server_params_json.isMember("host") ? multiverse_server_params_json["host"].asString() : "tcp://127.0.0.1";
        multiverse_params["server_port"] = multiverse_server_params_json.isMember("port") ? multiverse_server_params_json["port"].asString() : "7000";

        const Json::Value &multiverse_client_params_json = multiverse_params_json["multiverse_client"];
        multiverse_params["client_port"] = multiverse_client_params_json["port"].asString();

        const Json::Value &multiverse_client_meta_data_params_json = multiverse_client_params_json["meta_data"];
        multiverse_params["world_name"] = multiverse_client_meta_data_params_json["world_name"].asString();
        multiverse_params["simulation_name"] = multiverse_client_meta_data_params_json["simulation_name"].asString();

        send_objects_json = multiverse_params_json["multiverse_client"]["send"];
        receive_objects_json = multiverse_params_json["multiverse_client"]["receive"];
        world_name = multiverse_params["world_name"];
        simulation_name = multiverse_params["simulation_name"];

        server_socket_addr = multiverse_params["server_host"] + ":" + multiverse_params["server_port"];

        host = multiverse_params["server_host"];
        port = multiverse_params["client_port"];

        time_now = 0.0;
        *world_time = time_now;

        connect();
    }

public:
    bool communicate(const bool resend_meta_data = false) override
    {
        return MultiverseClient::communicate(resend_meta_data);
    }

private:
    std::string world_name;

    std::string simulation_name;

    Json::Value send_objects_json;

    Json::Value receive_objects_json;

    std::map<std::string, std::set<std::string>> send_objects;

    std::map<std::string, std::set<std::string>> receive_objects;

    std::thread connect_to_server_thread;

    std::thread meta_data_thread;

    std::vector<double *> send_data_vec;

    std::vector<double *> receive_data_vec;

    std::map<std::string, std::map<std::string, std::vector<double>>> object_data; // TODO: Use your own data

    double time_now; // TODO: Use your own time

private:
    void start_connect_to_server_thread() override
    {
        connect_to_server_thread = std::thread(&MyMultiverseClient::connect_to_server, this);
    }

    void wait_for_connect_to_server_thread_finish() override
    {
        if (connect_to_server_thread.joinable())
        {
            connect_to_server_thread.join();
        }
    }

    void start_meta_data_thread() override
    {
        meta_data_thread = std::thread(&MyMultiverseClient::send_and_receive_meta_data, this);
    }

    void wait_for_meta_data_thread_finish() override
    {
        if (meta_data_thread.joinable())
        {
            meta_data_thread.join();
        }
    }

    bool init_objects(bool from_request_meta_data) override
    {
        if (from_request_meta_data)
        {
            if (request_meta_data_json["receive"].empty())
            {
                receive_objects_json.clear();
            }
            if (request_meta_data_json["send"].empty())
            {
                send_objects_json.clear();
            }
            for (const std::string &object_name : request_meta_data_json["receive"].getMemberNames())
            {
                receive_objects_json[object_name] = request_meta_data_json["receive"][object_name];
            }
            for (const std::string &object_name : request_meta_data_json["send"].getMemberNames())
            {
                send_objects_json[object_name] = request_meta_data_json["send"][object_name];
            }
        }

        receive_objects.clear();
        for (const std::string &object_name : receive_objects_json.getMemberNames())
        {
            for (const Json::Value &attribute_json : receive_objects_json[object_name])
            {
                const std::string attribute_name = attribute_json.asString();
                receive_objects[object_name].insert(attribute_name);
            }
        }

        send_objects.clear();
        for (const std::string &object_name : send_objects_json.getMemberNames())
        {
            for (const Json::Value &attribute_json : send_objects_json[object_name])
            {
                const std::string attribute_name = attribute_json.asString();
                send_objects[object_name].insert(attribute_name);
            }
        }

        return send_objects.size() > 0 || receive_objects.size() > 0;
    }

    void bind_request_meta_data() override
    {
        request_meta_data_json.clear();
        request_meta_data_json["meta_data"]["world_name"] = world_name;
        request_meta_data_json["meta_data"]["simulation_name"] = simulation_name;
        request_meta_data_json["meta_data"]["length_unit"] = "m";
        request_meta_data_json["meta_data"]["angle_unit"] = "rad";
        request_meta_data_json["meta_data"]["mass_unit"] = "kg";
        request_meta_data_json["meta_data"]["time_unit"] = "s";
        request_meta_data_json["meta_data"]["handedness"] = "rhs";

        request_meta_data_json["send"] = Json::Value(Json::objectValue);
        for (const std::string &object_name : send_objects_json.getMemberNames())
        {
            for (const Json::Value &attribute_json : send_objects_json[object_name])
            {
                const std::string attribute_name = attribute_json.asString();
                request_meta_data_json["send"][object_name].append(attribute_name);
            }
        }

        request_meta_data_json["receive"] = Json::Value(Json::objectValue);
        for (const std::string &object_name : receive_objects_json.getMemberNames())
        {
            for (const Json::Value &attribute_json : receive_objects_json[object_name])
            {
                const std::string attribute_name = attribute_json.asString();
                request_meta_data_json["receive"][object_name].append(attribute_name);
            }
        }

        request_meta_data_str = request_meta_data_json.toStyledString();
    }

    void bind_response_meta_data() override
    {
        return;
    }

    void bind_api_callbacks() override
    {
        return;
    }

    void bind_api_callbacks_response() override
    {
        return;
    }

    void init_send_and_receive_data() override
    {
        for (const std::pair<const std::string, std::set<std::string>> &send_object : send_objects)
        {
            const std::string object_name = send_object.first;
            for (const std::string &attribute_name : send_object.second)
            {
                if (strcmp(attribute_name.c_str(), "position") == 0)
                {
                    object_data[object_name][attribute_name] = {1.0, 2.0, 3.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // x
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][1]); // y
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][2]); // z
                }
                else if (strcmp(attribute_name.c_str(), "quaternion") == 0)
                {
                    object_data[object_name][attribute_name] = {1.0, 0.0, 0.0, 0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // w
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][1]); // x
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][2]); // y
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][3]); // z
                }
                else if (strcmp(attribute_name.c_str(), "relative_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // lin_x
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][1]); // lin_y
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][2]); // lin_z
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][3]); // ang_x
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][4]); // ang_y
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][5]); // ang_z
                }
                else if (strcmp(attribute_name.c_str(), "joint_rvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_rvalue
                }
                else if (strcmp(attribute_name.c_str(), "joint_tvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_tvalue
                }
                else if (strcmp(attribute_name.c_str(), "joint_linear_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_linear_velocity
                }
                else if (strcmp(attribute_name.c_str(), "joint_angular_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_angular_velocity
                }
                else if (strcmp(attribute_name.c_str(), "joint_force") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_force
                }
                else if (strcmp(attribute_name.c_str(), "joint_torque") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_torque
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_rvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_rvalue
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_tvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_tvalue
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_linear_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_linear_velocity
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_angular_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_angular_velocity
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_force") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_force
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_torque") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    send_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_torque
                }
                else
                {
                    printf("[Client %s] The attribute [%s] of [%s] is not supported.\n", port.c_str(), attribute_name.c_str(), object_name.c_str());
                    continue;
                }
            }
        }

        for (const std::pair<const std::string, std::set<std::string>> &receive_object : receive_objects)
        {
            const std::string object_name = receive_object.first;
            for (const std::string &attribute_name : receive_object.second)
            {
                if (strcmp(attribute_name.c_str(), "position") == 0)
                {
                    object_data[object_name][attribute_name] = {1.0, 2.0, 3.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // x
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][1]); // y
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][2]); // z
                }
                else if (strcmp(attribute_name.c_str(), "quaternion") == 0)
                {
                    object_data[receive_object.first][attribute_name] = {1.0, 0.0, 0.0, 0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // w
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][1]); // x
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][2]); // y
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][3]); // z
                }
                else if (strcmp(attribute_name.c_str(), "relative_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // lin_x
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][1]); // lin_y
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][2]); // lin_z
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][3]); // ang_x
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][4]); // ang_y
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][5]); // ang_z
                }
                else if (strcmp(attribute_name.c_str(), "joint_rvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_rvalue
                }
                else if (strcmp(attribute_name.c_str(), "joint_tvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_tvalue
                }
                else if (strcmp(attribute_name.c_str(), "joint_linear_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_linear_velocity
                }
                else if (strcmp(attribute_name.c_str(), "joint_angular_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_angular_velocity
                }
                else if (strcmp(attribute_name.c_str(), "joint_force") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_force
                }
                else if (strcmp(attribute_name.c_str(), "joint_torque") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // joint_torque
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_rvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_rvalue
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_tvalue") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_tvalue
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_linear_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_linear_velocity
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_angular_velocity") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_angular_velocity
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_force") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_force
                }
                else if (strcmp(attribute_name.c_str(), "cmd_joint_torque") == 0)
                {
                    object_data[object_name][attribute_name] = {0.0}; // TODO: Use your own data
                    receive_data_vec.emplace_back(&object_data[object_name][attribute_name][0]); // cmd_joint_torque
                }
                else
                {
                    printf("[Client %s] The attribute [%s] of [%s] is not supported.\n", port.c_str(), attribute_name.c_str(), object_name.c_str());
                    continue;
                }
            }
        }
    }

    void bind_send_data() override
    {
        if (send_data_vec.size() != send_buffer.buffer_double.size)
        {
            printf("The size of send_data_vec (%zd) does not match with send_buffer_size (%zd).\n", send_data_vec.size(), send_buffer.buffer_double.size);
            return;
        }

        time_now += 0.001;            // TODO: Use your own time
        *world_time = time_now;
        for (size_t i = 0; i < send_buffer.buffer_double.size; i++)
        {
            send_buffer.buffer_double.data[i] = *send_data_vec[i];
        }
    }

    void bind_receive_data() override
    {
        if (receive_data_vec.size() != receive_buffer.buffer_double.size)
        {
            printf("[Client %s] The size of receive_data_vec (%zd) does not match with receive_buffer_size (%zd)\n", port.c_str(), receive_data_vec.size(), receive_buffer.buffer_double.size);
            return;
        }

        *world_time = time_now + 0.001; // TODO: Use your own time
        for (size_t i = 0; i < receive_buffer.buffer_double.size; i++)
        {
            *receive_data_vec[i] = receive_buffer.buffer_double.data[i];
        }
    }

    void clean_up() override
    {
        send_data_vec.clear();

	    receive_data_vec.clear();
    }

    void reset() override
    {
        // TODO: Implement your own reset
        return;
    }

private:
    MyMultiverseClient()
    {
    }

    ~MyMultiverseClient()
    {
    }
};