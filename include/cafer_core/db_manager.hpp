//
// Created by phlf on 07/07/16.
//

#ifndef CAFER_CORE_DB_MANAGER_H
#define CAFER_CORE_DB_MANAGER_H

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include <condition_variable>
#include <thread>
#include <atomic>
#include <fstream>

#include <boost/filesystem.hpp>

#include "cafer_core/cafer_core.hpp"

#include <cafer_core/DBManager.h>

namespace cafer_core {

    class DatabaseManager : public cafer_core::Component {
    public:
        enum class Request : uint8_t {
            RECORD_DATA, STOP_RECORDING, REQUEST_DATA
        };

        enum class Response : uint8_t {
            STATUS_READY, STATUS_ACTIVE, ERROR, DATA
        };

        using Component::Component;

        ~DatabaseManager();

        void init() override;

        void client_connect_to_ros() override;

        void client_disconnect_from_ros() override;

        void update() override
        {};

        bool add_wave(std::string&& name);

    private:

        class _Wave;

        /**
        * Class manipulating the filesystem.
        */
        class _DBFileSystem {
        public:
            _DBFileSystem(_Wave*);

            ~_DBFileSystem();

            void new_records();

            void close_records();

            void save_data(std::unique_ptr<cafer_core::Data>);

        private:
            cafer_core::shared_ptr<_Wave> _wave;

            uint32_t _counter = 0;
            std::map<std::string, std::ofstream> _records;
            boost::filesystem::path _ros_home;


        };

        class _WriteWorker {
        public:
            _WriteWorker();

            _WriteWorker(_Wave*);

            ~_WriteWorker();

            void awake_worker();

            void pause_worker();

            void link_to_wave(_Wave*);

        private:
            cafer_core::shared_ptr<_Wave> _wave;

            std::atomic<bool> _is_active{false};
            std::unique_ptr<std::thread> _processing_thread;
            std::condition_variable _signal_processing_thread;
            std::mutex _signal_process_mutex;

            void _processing();
        };

        class _Wave {
        public:
            const uint32_t id;
            const std::string name;
            const std::string type;
            bool sequential;
            std::map<std::string, std::string> data_topics;
            std::map<std::string, std::string> data_structure;

            _DBFileSystem fs_manager;
            std::vector<std::unique_ptr<cafer_core::IManager>> managers;
            cafer_core::shared_ptr<cafer_core::Publisher> status_publisher;

            _Wave(_Wave&&);

            _Wave(uint32_t id_, std::string&, Publisher*);

            void add_manager(cafer_core::IManager&);

            bool no_data_left();

            void connect();

            void disconnect();

        private:
            _WriteWorker _write_worker;

        };

        uint32_t requester_id;
        std::string _data_request;

        cafer_core::shared_ptr<cafer_core::Publisher> _status_publisher;
        std::unique_ptr<cafer_core::Subscriber> _request_subscriber;

        std::unique_ptr<std::thread> _send_data_thread;
        std::condition_variable _signal_send_data_thread;
        std::mutex _signal_send_data_mutex;

        std::map<uint32_t, _Wave> _connected_waves;

        void _request_cb(const cafer_core::DBManagerConstPtr& request_msg);

        void _send_data();

        void _record_data(const uint32_t& id);

        void _stop_recording(const uint32_t& id);

        bool _find_wave_by_type(std::string& type, std::vector<std::string>& waves_uris);
    };
}

#endif //CAFER_CORE_DB_MANAGER_H
