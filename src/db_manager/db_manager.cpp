//
// Created by phlf on 11/04/16.
//

#include "cafer_core/db_manager.hpp"

using namespace cafer_core;

DatabaseManager::~DatabaseManager()
{
    client_disconnect_from_ros();

    _send_data_thread.reset();

    _status_publisher.reset();
    _request_subscriber.reset();
}

void DatabaseManager::init()
{
    //Publisher
    _status_publisher.reset(new Publisher(ros_nh->advertise<cafer_core::db_manager_status>("status", 10)));
    //Subscriber
    _request_subscriber.reset(new Subscriber(
            ros_nh->subscribe<cafer_core::db_manager_request>("request", 10,
                                                              boost::bind(&DatabaseManager::_request_cb, this,
                                                                          _1))));

    _send_data_thread.reset(new std::thread(&DatabaseManager::_send_data, this));

    _is_init = true;
}

void DatabaseManager::client_connect_to_ros()
{
    for (auto& wave:_connected_waves) {
        wave.second.connect();
    }
}

void DatabaseManager::client_disconnect_from_ros()
{
    for (auto& wave:_connected_waves) {
        wave.second.disconnect();
    }
}

void DatabaseManager::_send_data()
{
    std::unique_lock<std::mutex> lock(_signal_send_data_mutex);

    //Processing loop
    while (ros::ok()) {
        //Release _signal_process_mutex and blocks the thread.
        _signal_send_data_thread.wait(lock);
        //Thread notified: acquires _signal_process_mutex and resume.

        //TODO: Define task here.
    }
}

void DatabaseManager::_request_cb(const cafer_core::db_manager_requestConstPtr& request_msg)
{
    switch (static_cast<Request>(request_msg->request)) {
        case Request::RECORD_DATA:
            _record_data(request_msg->id);
            break;
        case Request::STOP_RECORDING:
            _stop_recording(request_msg->id);
            break;
        case Request::REQUEST_DATA:
        default:
            ROS_WARN_STREAM("Received unknown request.");
    }
}

void DatabaseManager::_record_data(const uint32_t& id)
{
    //This commented part may be useful in a dynamic scenario (adding waves during experiment)

//    if(_connected_waves.find(id)!=_connected_waves.end()) {
//        _connected_waves[id]=Wave(map_watchdog.find());
//    }

    auto wave = _connected_waves.find(id);
    if (wave == _connected_waves.end()) {
        ROS_WARN_STREAM("Unable to find wave " << id);
    }
    else {
        wave->second.connect();
    }
}

void DatabaseManager::_stop_recording(const uint32_t& id)
{
    auto wave = _connected_waves.find(id);
    if (wave == _connected_waves.end()) {
        ROS_WARN_STREAM("Unable to find wave " << id);
    }
    else {
        wave->second.disconnect();
    }
}

bool DatabaseManager::add_wave(std::string&& name)
{
    bool succeed = false;
    ClientDescriptor descriptor;

    if (find_by_name(name, descriptor)) {
        auto wave_it = _connected_waves.find(descriptor.id);

        if (wave_it == _connected_waves.end()) {
            _connected_waves.emplace(descriptor.id, std::move(Wave(name, _status_publisher.get())));
        }
        else {
            ROS_WARN_STREAM("The wave " << name << " is already linked to the DB_Manager!");
        }
    }
    else {
        ROS_WARN_STREAM("The wave " << name << " doesn't exist in the experiment!");
    }

    return succeed;
}


